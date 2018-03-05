
#include "spdz2_ext_processor_base.h"

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>

//***********************************************************************************************//
void * spdz2_ext_processor_proc(void * arg)
{
	spdz2_ext_processor_base * processor = (spdz2_ext_processor_base *)arg;
	processor->run();
	return NULL;
}

//***********************************************************************************************//
const int spdz2_ext_processor_base::sm_op_code_offline_synch = 100;
const int spdz2_ext_processor_base::sm_op_code_input_synch = 101;
const int spdz2_ext_processor_base::sm_op_code_triple_synch = 102;
const int spdz2_ext_processor_base::sm_op_code_share_immediate_synch = 103;
const int spdz2_ext_processor_base::sm_op_code_bit_synch = 104;
const int spdz2_ext_processor_base::sm_op_code_inverse_synch = 105;
const int spdz2_ext_processor_base::sm_op_code_open_asynch = 200;
const int spdz2_ext_processor_base::sm_op_code_verify_asynch = 201;
const int spdz2_ext_processor_base::sm_op_code_input_asynch = 202;
const int spdz2_ext_processor_base::sm_op_code_mult_asynch = 203;
const int spdz2_ext_processor_base::sm_op_code_share_immediates_asynch = 204;

//***********************************************************************************************//
spdz2_ext_processor_base::spdz2_ext_processor_base()
/*ops*/		: m_runner(0), m_run_flag(false), m_party_id(-1), m_num_of_parties(0), m_thread_id(-1)
/*offline*/	, m_offline_synch_success(false)
/*input_s*/	, m_input_synch_success(false), m_input_synch_output(NULL), m_input_synch_pid(-1)
/*triple*/	, m_triple_synch_success(false), m_A(NULL), m_B(NULL), m_C(NULL)
/*shr_im_s*/, m_share_immediate_synch_success(false), m_share_immediate_synch_value(NULL), m_share_immediate_synch_share(NULL)
/*bit*/		, m_bit_synch_success(false), m_bit_synch_share(NULL)
/*inverse*/	, m_inverse_synch_success(false), m_inverse_synch_value_share(NULL), m_inverse_synch_inverse_share(NULL)
/*open*/	, m_open_asynch_on(false), m_open_asynch_success(false), m_open_asynch_count(0), m_open_asynch_input(NULL), m_open_asynch_output(NULL), m_open_asynch_verify(false)
/*verify*/	, m_verify_asynch_on(false), m_verify_asynch_success(false), m_verify_asynch_error(NULL)
/*input_a*/	, m_input_asynch_on(false), m_input_asynch_success(false), m_input_asynch_pid(-1), m_input_asynch_count(0), m_input_asynch_output(NULL)
/*mult*/	, m_mult_asynch_on(false), m_mult_asynch_success(false), m_mult_asynch_count(0), m_mult_asynch_input(NULL), m_mult_asynch_output(NULL), m_mult_asynch_verify(false)
/*shr_im_a*/, m_share_immediates_asynch_on(false), m_share_immediates_asynch_success(false), m_share_immediates_asynch_count(0), m_share_immediates_asynch_input(NULL), m_share_immediates_asynch_output(NULL)
{
	pthread_mutex_init(&m_q_lock, NULL);
	sem_init(&m_task, 0, 0);
	sem_init(&m_offline_synch_done, 0, 0);
	sem_init(&m_input_synch_done, 0, 0);
	sem_init(&m_triple_synch_done, 0, 0);
	sem_init(&m_share_immediate_synch_done, 0, 0);
	sem_init(&m_bit_synch_done, 0, 0);
	sem_init(&m_inverse_synch_done, 0, 0);
	sem_init(&m_open_asynch_done, 0, 0);
	sem_init(&m_verify_asynch_done, 0, 0);
	sem_init(&m_input_asynch_done, 0, 0);
	sem_init(&m_mult_asynch_done, 0, 0);
	sem_init(&m_share_immediates_asynch_done, 0, 0);

	mpz_init(m_inverse_synch_value);
	mpz_init(m_inverse_synch_inverse);
}

//***********************************************************************************************//
spdz2_ext_processor_base::~spdz2_ext_processor_base()
{
	pthread_mutex_destroy(&m_q_lock);
	sem_destroy(&m_task);
	sem_destroy(&m_offline_synch_done);
	sem_destroy(&m_input_synch_done);
	sem_destroy(&m_triple_synch_done);
	sem_destroy(&m_share_immediate_synch_done);
	sem_destroy(&m_bit_synch_done);
	sem_destroy(&m_inverse_synch_done);
	sem_destroy(&m_open_asynch_done);
	sem_destroy(&m_verify_asynch_done);
	sem_destroy(&m_input_asynch_done);
	sem_destroy(&m_mult_asynch_done);
	sem_destroy(&m_share_immediates_asynch_done);

	mpz_clear(m_inverse_synch_value);
	mpz_clear(m_inverse_synch_inverse);
}

//***********************************************************************************************//
int spdz2_ext_processor_base::start(const int pid, const int num_of_parties, const int thread_id, const char * field,
									const int open_count, const int mult_count, const int bits_count)
{
	m_party_id = pid;
	m_num_of_parties = num_of_parties;
	m_thread_id = thread_id;

	m_syslog_name = get_syslog_name();
	openlog(m_syslog_name.c_str(), LOG_NDELAY|LOG_PID, LOG_USER);
	setlogmask(LOG_UPTO(LOG_NOTICE));

	if(m_run_flag)
	{
		syslog(LOG_ERR, "spdz2_ext_processor_base::start: this processor is already started");
		return -1;
	}
	syslog(LOG_NOTICE, "spdz2_ext_processor_base::start: pid %d", m_party_id);

	if(0 != init_protocol(open_count, mult_count, bits_count))
	{
		syslog(LOG_ERR, "spdz2_ext_processor_base::start: protocol initialization failure.");
		return -1;
	}

	syslog(LOG_NOTICE, "spdz2_ext_processor_base::init_protocol: starting input share [%s]", spdz2_ext_processor_base::get_time_stamp().c_str());
	if(0 != load_inputs())
	{
		syslog(LOG_ERR, "spdz2_ext_processor_base::start: input load failure.");
		return -1;
	}

	syslog(LOG_NOTICE, "spdz2_ext_processor_base::init_protocol: starting online [%s]", spdz2_ext_processor_base::get_time_stamp().c_str());

	m_run_flag = true;
	int result = pthread_create(&m_runner, NULL, spdz2_ext_processor_proc, this);
	if(0 != result)
	{
		char errmsg[512];
		syslog(LOG_ERR, "spdz2_ext_processor_base::start: pthread_create() failed with error %d : %s", result, strerror_r(result, errmsg, 512));
		m_run_flag = false;
		return -1;
	}

	return 0;
}

//***********************************************************************************************//
int spdz2_ext_processor_base::stop(const time_t timeout_sec)
{
	if(!m_run_flag)
	{
		syslog(LOG_ERR, "spdz2_ext_processor_base::stop this processor is not running.");
		return -1;
	}
	syslog(LOG_NOTICE, "spdz2_ext_processor_base::stop: online done [%s]", spdz2_ext_processor_base::get_time_stamp().c_str());

	m_run_flag = false;
	void * return_code = NULL;

	struct timespec timeout;
	clock_gettime(CLOCK_REALTIME, &timeout);
	timeout.tv_sec += timeout_sec;

	int result = pthread_timedjoin_np(m_runner, &return_code, &timeout);
	if(0 != result)
	{
		char errmsg[512];
		syslog(LOG_ERR, "spdz2_ext_processor_base::stop: pthread_timedjoin_np() failed with error %d : %s", result, strerror_r(result, errmsg, 512));
		return -1;
	}

	syslog(LOG_NOTICE, "spdz2_ext_processor_base::stop: pid %d", m_party_id);
	delete_protocol();
	delete_inputs();
	closelog();
	return 0;
}

//***********************************************************************************************//
void spdz2_ext_processor_base::run()
{
	long timeout_ns = 200 /*ms*/ * 1000 /*us*/ * 1000 /*ns*/;
	while(m_run_flag)
	{
		struct timespec timeout;
		clock_gettime(CLOCK_REALTIME, &timeout);
		timeout.tv_nsec += timeout_ns;
		timeout.tv_sec += timeout.tv_nsec / 1000000000;
		timeout.tv_nsec = timeout.tv_nsec % 1000000000;

		int result = sem_timedwait(&m_task, &timeout);
		if(0 == result)
		{
			int op_code = pop_task();
			syslog(LOG_DEBUG, "spdz2_ext_processor_base::run: op_code %d", op_code);
			switch(op_code)
			{
			case sm_op_code_offline_synch:
				exec_offline_synch();
				break;
			case sm_op_code_input_synch:
				exec_input_synch();
				break;
			case sm_op_code_triple_synch:
				exec_triple_synch();
				break;
			case sm_op_code_share_immediate_synch:
				exec_share_immediate_synch();
				break;
			case sm_op_code_bit_synch:
				exec_bit_synch();
				break;
			case sm_op_code_inverse_synch:
				exec_inverse_synch();
				break;
			case sm_op_code_open_asynch:
				exec_open_asynch();
				break;
			case sm_op_code_verify_asynch:
				exec_verify_asynch();
				break;
			case sm_op_code_input_asynch:
				exec_input_asynch();
				break;
			case sm_op_code_mult_asynch:
				exec_mult_asynch();
				break;
			case sm_op_code_share_immediates_asynch:
				exec_share_immediates_asynch();
				break;
			default:
				syslog(LOG_WARNING, "spdz2_ext_processor_base::run: unsupported op_code %d", op_code);
				break;
			}
		}
	}
}

//***********************************************************************************************//
int spdz2_ext_processor_base::push_task(const int op_code)
{
	int result = pthread_mutex_lock(&m_q_lock);
	if(0 != result)
	{
		char errmsg[512];
		syslog(LOG_ERR, "spdz2_ext_processor_base::push_task: pthread_mutex_lock() failed with error %d : %s", result, strerror_r(result, errmsg, 512));
		return -1;
	}
	m_task_q.push_back(op_code);
	sem_post(&m_task);
	result = pthread_mutex_unlock(&m_q_lock);
	if(0 != result)
	{
		char errmsg[512];
		syslog(LOG_ERR, "spdz2_ext_processor_base::push_task: pthread_mutex_unlock() failed with error %d : %s", result, strerror_r(result, errmsg, 512));
	}
	syslog(LOG_DEBUG, "spdz2_ext_processor_base::push_task: op_code %d", op_code);
	return 0;
}

//***********************************************************************************************//
int spdz2_ext_processor_base::pop_task()
{
	int op_code = -1;
	int result = pthread_mutex_lock(&m_q_lock);
	if(0 == result)
	{
		if(!m_task_q.empty())
		{
			op_code = *m_task_q.begin();
			m_task_q.pop_front();
		}

		result = pthread_mutex_unlock(&m_q_lock);
		if(0 != result)
		{
			char errmsg[512];
			syslog(LOG_ERR, "spdz2_ext_processor_base::pop_task: pthread_mutex_unlock() failed with error %d : %s", result, strerror_r(result, errmsg, 512));
		}
	}
	else
	{
		char errmsg[512];
		syslog(LOG_ERR, "spdz2_ext_processor_base::pop_task: pthread_mutex_lock() failed with error %d : %s", result, strerror_r(result, errmsg, 512));
	}

	syslog(LOG_DEBUG, "spdz2_ext_processor_base::pop_task: op_code %d", op_code);
	return op_code;
}

//***********************************************************************************************//
int spdz2_ext_processor_base::load_inputs()
{
	std::list<std::string> party_input_specs;
	if(0 != load_party_input_specs(party_input_specs))
	{
		syslog(LOG_ERR, "spdz2_ext_processor_base::load_inputs: failed loading the party input specs");
		return -1;
	}

	for(std::list<std::string>::const_iterator i = party_input_specs.begin(); i != party_input_specs.end(); ++i)
	{
		if(0 != load_party_inputs(*i))
		{
			syslog(LOG_ERR, "spdz2_ext_processor_base::load_inputs: failed loading party input by spec [%s]", i->c_str());
			return -1;
		}
	}

	return 0;
}

//***********************************************************************************************//
int spdz2_ext_processor_base::delete_inputs()
{
	for(std::map< int , shared_input_t >::iterator i = m_shared_inputs.begin(); i != m_shared_inputs.end(); ++i)
	{
		if(NULL != i->second.shared_values && 0 < i->second.share_count)
		{
			for(size_t j = 0; j < i->second.share_count; ++j)
				mpz_clear(i->second.shared_values[j]);
			delete i->second.shared_values;
			i->second.shared_values = NULL;
			i->second.share_count = 0;
			i->second.share_index = 0;
		}
	}
	m_shared_inputs.clear();
}

//***********************************************************************************************//
int spdz2_ext_processor_base::load_party_input_specs(std::list<std::string> & party_input_specs)
{
	static const char common_inputs_spec[] = "Parties_Inputs.txt";

	party_input_specs.clear();
	FILE * pf = fopen(common_inputs_spec, "r");
	if(NULL != pf)
	{
		char sz[128];
		while(NULL != fgets(sz, 128, pf))
		{
			if (NULL == strstr(sz, "#"))
				party_input_specs.push_back(sz);
		}
		fclose(pf);
	}
	else
	{
		syslog(LOG_ERR, "spdz2_ext_processor_base::load_party_input_specs: failed to open [%s].", common_inputs_spec);
		return -1;
	}
	return (party_input_specs.empty())? -1: 0;
}

//***********************************************************************************************//
int spdz2_ext_processor_base::load_party_inputs(const std::string & party_input_spec)
{
	std::string::size_type pos = party_input_spec.find(',');
	if(std::string::npos != pos)
	{
		int pid = (int)strtol(party_input_spec.substr(0, pos).c_str(), NULL, 10);
		size_t count = (size_t)strtol(party_input_spec.substr(pos+1).c_str(), NULL, 10);
		if(0 < count)
			return load_party_inputs(pid, count);
		else
			return 0;
	}
	else
	{
		syslog(LOG_ERR, "spdz2_ext_processor_base::load_party_inputs: invalid party input spec format [%s]", party_input_spec.c_str());
		return -1;
	}
}

//***********************************************************************************************//
int spdz2_ext_processor_base::load_party_inputs(const int pid, const size_t count)
{
	return (pid == this->m_party_id)? load_self_party_inputs(count): load_peer_party_inputs(pid, count);
}

//***********************************************************************************************//
int spdz2_ext_processor_base::load_self_party_inputs(const size_t count)
{
	int result = -1;
	mpz_t * clr_values = NULL;
	if(0 == load_clr_party_inputs(&clr_values, count) && NULL != clr_values)
	{
		result = load_peer_party_inputs(m_party_id, count, clr_values);

		for(size_t i = 0; i < count; ++i)
			mpz_clear(clr_values[i]);
		delete clr_values;
	}
	else
		syslog(LOG_ERR, "spdz2_ext_processor_base::load_self_party_inputs: failed loading clear inputs.");
	return result;
}

int spdz2_ext_processor_base::load_peer_party_inputs(const int pid, const size_t count, const mpz_t * clr_values)
{
	int result = -1;
	shared_input_t party_inputs;
	party_inputs.share_count = count;
	party_inputs.share_index = 0;
	party_inputs.shared_values = new mpz_t[party_inputs.share_count];
	for(size_t i = 0; i < party_inputs.share_count; ++i)
		mpz_init(party_inputs.shared_values[i]);

	if(protocol_share(pid, count, (NULL != clr_values)? clr_values: party_inputs.shared_values, party_inputs.shared_values))
	{
		if(m_shared_inputs.insert(std::pair<int, shared_input_t>(pid, party_inputs)).second)
			result = 0;
		else
		{
			for(size_t i = 0; i < count; ++i)
				mpz_clear(party_inputs.shared_values[i]);
			delete party_inputs.shared_values;
			syslog(LOG_ERR, "spdz2_ext_processor_base::load_peer_party_inputs: failed to map-insert shared inputs for pid=%d.", pid);
		}
	}
	else
	{
		for(size_t i = 0; i < count; ++i)
			mpz_clear(party_inputs.shared_values[i]);
		delete party_inputs.shared_values;
		syslog(LOG_ERR, "spdz2_ext_processor_base::load_peer_party_inputs: protocol_share() failed for pid=%d.", pid);
	}
	return result;
}

//***********************************************************************************************//
int spdz2_ext_processor_base::load_clr_party_inputs(mpz_t ** clr_values, const size_t count)
{
	char sz[128];
	snprintf(sz, 128, "party_%d_input.txt", m_party_id);
	std::string input_file = sz;

	FILE * pf = fopen(input_file.c_str(), "r");
	if(NULL != pf)
	{
		*clr_values = new mpz_t[count];
		size_t clr_values_idx = 0;

		while(NULL != fgets(sz, 128, pf))
		{
			mpz_init((*clr_values)[clr_values_idx]);
			mpz_set_str((*clr_values)[clr_values_idx++], sz, 10);
			if(clr_values_idx >= count)
				break;
		}
		fclose(pf);

		if(count > clr_values_idx)
		{
			for(size_t i = 0; i < clr_values_idx; i++)
				mpz_clear((*clr_values)[i]);
			delete (*clr_values);
			*clr_values = NULL;
			syslog(LOG_ERR, "spdz2_ext_processor_base::load_clr_party_inputs: not enough inputs in file [%s]; required %lu; file has %lu;",
					input_file.c_str(), count, clr_values_idx);
		}
	}
	return (NULL != (*clr_values))? 0: -1;
}

//***********************************************************************************************//
int spdz2_ext_processor_base::offline(const int /*offline_size*/, const time_t timeout_sec)
{
	/*
	 * In BIU implementation the offline size is set at the protocol construction
	 * the call to offline will reallocate the same size of offline
	 */
	m_offline_synch_success = false;
	if(0 != push_task(spdz2_ext_processor_base::sm_op_code_offline_synch))
	{
		syslog(LOG_ERR, "spdz2_ext_processor_base::offline: failed pushing a task to queue.");
		return -1;
	}

	struct timespec timeout;
	clock_gettime(CLOCK_REALTIME, &timeout);
	timeout.tv_sec += timeout_sec;

	int result = sem_timedwait(&m_offline_synch_done, &timeout);
	if(0 != result)
	{
		result = errno;
		char errmsg[512];
		syslog(LOG_ERR, "spdz2_ext_processor_base::offline: sem_timedwait() failed with error %d : %s", result, strerror_r(result, errmsg, 512));
		return -1;
	}

	return (m_offline_synch_success)? 0: -1;
}

//***********************************************************************************************//
void spdz2_ext_processor_base::exec_offline_synch()
{
	m_offline_synch_success = protocol_offline();
	sem_post(&m_offline_synch_done);
}

//***********************************************************************************************//
int spdz2_ext_processor_base::input(const int input_of_pid, mpz_t * input_value, const time_t timeout_sec)
{
	m_input_synch_success = false;
	m_input_synch_output = input_value;
	m_input_synch_pid = input_of_pid;

	if(0 != push_task(spdz2_ext_processor_base::sm_op_code_input_synch))
	{
		syslog(LOG_ERR, "spdz2_ext_processor_base::input: failed pushing a task to queue.");
		return -1;
	}

	struct timespec timeout;
	clock_gettime(CLOCK_REALTIME, &timeout);
	timeout.tv_sec += timeout_sec;

	int result = sem_timedwait(&m_input_synch_done, &timeout);
	if(0 != result)
	{
		result = errno;
		char errmsg[512];
		syslog(LOG_ERR, "spdz2_ext_processor_base::input: sem_timedwait() failed with error %d : %s", result, strerror_r(result, errmsg, 512));
		return -1;
	}

	return (m_input_synch_success)? 0: -1;
}

//***********************************************************************************************//
void spdz2_ext_processor_base::exec_input_synch()
{
	std::map< int , shared_input_t >::iterator i = m_shared_inputs.find(m_input_synch_pid);
	if((m_input_synch_success = (m_shared_inputs.end() != i && 0 < i->second.share_count && i->second.share_index < i->second.share_count)))
		mpz_set(*m_input_synch_output, i->second.shared_values[i->second.share_index++]);
	else
		syslog(LOG_ERR, "spdz2_ext_processor_base::exec_input_synch: failed to get input for pid %d.", m_input_synch_pid);
	sem_post(&m_input_synch_done);
}

//***********************************************************************************************//
int spdz2_ext_processor_base::triple(mpz_t * a, mpz_t * b, mpz_t * c, const time_t timeout_sec)
{
	m_triple_synch_success = false;
	m_A = a;
	m_B = b;
	m_C = c;

	if(0 != push_task(spdz2_ext_processor_base::sm_op_code_triple_synch))
	{
		syslog(LOG_ERR, "spdz2_ext_processor_base::triple: failed pushing a task to queue.");
		return -1;
	}

	struct timespec timeout;
	clock_gettime(CLOCK_REALTIME, &timeout);
	timeout.tv_sec += timeout_sec;

	int result = sem_timedwait(&m_triple_synch_done, &timeout);
	if(0 != result)
	{
		result = errno;
		char errmsg[512];
		syslog(LOG_ERR, "spdz2_ext_processor_base::triple: sem_timedwait() failed with error %d : %s", result, strerror_r(result, errmsg, 512));
		return -1;
	}

	return (m_triple_synch_success)? 0: -1;
}

//***********************************************************************************************//
void spdz2_ext_processor_base::exec_triple_synch()
{
	m_triple_synch_success = protocol_triple(m_A, m_B, m_C);
	sem_post(&m_triple_synch_done);
}

//***********************************************************************************************//
int spdz2_ext_processor_base::share_immediate(const mpz_t * value, mpz_t * share, const time_t timeout_sec)
{
	m_share_immediate_synch_success = false;
	m_share_immediate_synch_value = value;
	m_share_immediate_synch_share = share;

	if(0 != push_task(spdz2_ext_processor_base::sm_op_code_share_immediate_synch))
	{
		syslog(LOG_ERR, "spdz2_ext_processor_base::share_immediate: failed pushing a task to queue.");
		return -1;
	}

	struct timespec timeout;
	clock_gettime(CLOCK_REALTIME, &timeout);
	timeout.tv_sec += timeout_sec;

	int result = sem_timedwait(&m_share_immediate_synch_done, &timeout);
	if(0 != result)
	{
		result = errno;
		char errmsg[512];
		syslog(LOG_ERR, "spdz2_ext_processor_base::share_immediate: sem_timedwait() failed with error %d : %s", result, strerror_r(result, errmsg, 512));
		return -1;
	}

	return (m_share_immediate_synch_success)? 0: -1;
}

//***********************************************************************************************//
void spdz2_ext_processor_base::exec_share_immediate_synch()
{
	m_share_immediate_synch_success = protocol_share(0, 1, m_share_immediate_synch_value, m_share_immediate_synch_share);
	sem_post(&m_share_immediate_synch_done);
}

//***********************************************************************************************//
int spdz2_ext_processor_base::bit(mpz_t * share, const time_t timeout_sec)
{
	m_bit_synch_success = false;
	m_bit_synch_share = share;

	if(0 != push_task(spdz2_ext_processor_base::sm_op_code_bit_synch))
	{
		syslog(LOG_ERR, "spdz2_ext_processor_base::bit: failed pushing a task to queue.");
		return -1;
	}

	struct timespec timeout;
	clock_gettime(CLOCK_REALTIME, &timeout);
	timeout.tv_sec += timeout_sec;

	int result = sem_timedwait(&m_bit_synch_done, &timeout);
	if(0 != result)
	{
		result = errno;
		char errmsg[512];
		syslog(LOG_ERR, "spdz2_ext_processor_base::bit: sem_timedwait() failed with error %d : %s", result, strerror_r(result, errmsg, 512));
		return -1;
	}

	return (m_bit_synch_success)? 0: -1;
}

//***********************************************************************************************//
void spdz2_ext_processor_base::exec_bit_synch()
{
	m_bit_synch_success = protocol_bits(1, m_bit_synch_share);
	sem_post(&m_bit_synch_done);
}

//***********************************************************************************************//
int spdz2_ext_processor_base::inverse(mpz_t * share_value, mpz_t * share_inverse, const time_t timeout_sec)
{
	m_inverse_synch_success = false;
	m_inverse_synch_value_share = share_value;
	m_inverse_synch_inverse_share = share_inverse;

	if(0 != push_task(spdz2_ext_processor_base::sm_op_code_inverse_synch))
	{
		syslog(LOG_ERR, "spdz2_ext_processor_base::inverse: failed pushing a task to queue.");
		return -1;
	}

	struct timespec timeout;
	clock_gettime(CLOCK_REALTIME, &timeout);
	timeout.tv_sec += timeout_sec;

	int result = sem_timedwait(&m_inverse_synch_done, &timeout);
	if(0 != result)
	{
		result = errno;
		char errmsg[512];
		syslog(LOG_ERR, "spdz2_ext_processor_base::inverse: sem_timedwait() failed with error %d : %s", result, strerror_r(result, errmsg, 512));
		return -1;
	}

	return (m_inverse_synch_success)? 0: -1;
}

//***********************************************************************************************//
void spdz2_ext_processor_base::exec_inverse_synch()
{
	m_inverse_synch_success =
			protocol_random_value(&m_inverse_synch_value) &&
			protocol_value_inverse(&m_inverse_synch_value, &m_inverse_synch_inverse) &&
			protocol_share(0, 1, &m_inverse_synch_value, m_inverse_synch_value_share) &&
			protocol_share(0, 1, &m_inverse_synch_inverse, m_inverse_synch_inverse_share);
	sem_post(&m_inverse_synch_done);
}

//***********************************************************************************************//
int spdz2_ext_processor_base::start_open(const size_t share_count, const mpz_t * share_values, mpz_t * opens, int verify)
{
	if(m_open_asynch_on)
	{
		syslog(LOG_ERR, "spdz2_ext_processor_base::start_open: open is already started.");
		return -1;
	}
	m_open_asynch_on = true;
	m_open_asynch_success = false;
	m_open_asynch_count = share_count;
	m_open_asynch_input = share_values;
	m_open_asynch_output = opens;
	m_open_asynch_verify = (verify != 0)? true: false;

	if(0 != push_task(spdz2_ext_processor_base::sm_op_code_open_asynch))
	{
		syslog(LOG_ERR, "spdz2_ext_processor_base::start_open: failed pushing a task to queue.");
		return -1;
	}
	return 0;
}

//***********************************************************************************************//
int spdz2_ext_processor_base::stop_open(const time_t timeout_sec)
{
	if(!m_open_asynch_on)
	{
		syslog(LOG_ERR, "spdz2_ext_processor_base::stop_open: open is not started.");
		return -1;
	}
	m_open_asynch_on = false;

	struct timespec timeout;
	clock_gettime(CLOCK_REALTIME, &timeout);
	timeout.tv_sec += timeout_sec;

	int result = sem_timedwait(&m_open_asynch_done, &timeout);
	if(0 != result)
	{
		result = errno;
		char errmsg[512];
		syslog(LOG_ERR, "spdz2_ext_processor_base::stop_open: sem_timedwait() failed with error %d : %s", result, strerror_r(result, errmsg, 512));
		return -1;
	}

	return (m_open_asynch_success)? 0: -1;
}

//***********************************************************************************************//
void spdz2_ext_processor_base::exec_open_asynch()
{
	m_open_asynch_success = protocol_open(m_open_asynch_count, m_open_asynch_input, m_open_asynch_output, m_open_asynch_verify);
	sem_post(&m_open_asynch_done);
}

//***********************************************************************************************//
int spdz2_ext_processor_base::start_verify(int * error)
{
	if(m_verify_asynch_on)
	{
		syslog(LOG_ERR, "spdz2_ext_processor_base::start_verify: verify is already started.");
		return -1;
	}
	m_verify_asynch_on = true;
	m_verify_asynch_success = false;
	m_verify_asynch_error = error;

	if(0 != push_task(spdz2_ext_processor_base::sm_op_code_verify_asynch))
	{
		syslog(LOG_ERR, "spdz2_ext_processor_base::start_verify: failed pushing a task to queue.");
		return -1;
	}
	return 0;
}

//***********************************************************************************************//
int spdz2_ext_processor_base::stop_verify(const time_t timeout_sec)
{
	if(!m_verify_asynch_on)
	{
		syslog(LOG_ERR, "spdz2_ext_processor_base::stop_verify: verify is not started.");
		return -1;
	}
	m_verify_asynch_on = false;

	struct timespec timeout;
	clock_gettime(CLOCK_REALTIME, &timeout);
	timeout.tv_sec += timeout_sec;

	int result = sem_timedwait(&m_verify_asynch_done, &timeout);
	if(0 != result)
	{
		result = errno;
		char errmsg[512];
		syslog(LOG_ERR, "spdz2_ext_processor_base::stop_verify: sem_timedwait() failed with error %d : %s", result, strerror_r(result, errmsg, 512));
		return -1;
	}

	return (m_verify_asynch_success)? 0: -1;
}

//***********************************************************************************************//
void spdz2_ext_processor_base::exec_verify_asynch()
{
	m_verify_asynch_success = protocol_verify(m_verify_asynch_error);
	sem_post(&m_verify_asynch_done);
}

//***********************************************************************************************//
int spdz2_ext_processor_base::start_input(const int input_of_pid, const size_t num_of_inputs, mpz_t * inputs)
{
	if(m_input_asynch_on)
	{
		syslog(LOG_ERR, "spdz2_ext_processor_base::start_input: input is already started.");
		return -1;
	}
	m_input_asynch_on = true;

	m_input_asynch_success = false;
	m_input_asynch_pid = input_of_pid;
	m_input_asynch_count = num_of_inputs;
	m_input_asynch_output = inputs;

	if(0 != push_task(spdz2_ext_processor_base::sm_op_code_input_asynch))
	{
		syslog(LOG_ERR, "spdz2_ext_processor_base::start_input: failed pushing a task to queue.");
		return -1;
	}
	return 0;
}

//***********************************************************************************************//
int spdz2_ext_processor_base::stop_input(const time_t timeout_sec)
{
	if(!m_input_asynch_on)
	{
		syslog(LOG_ERR, "spdz2_ext_processor_base::stop_input: input is not started.");
		return -1;
	}
	m_input_asynch_on = false;

	struct timespec timeout;
	clock_gettime(CLOCK_REALTIME, &timeout);
	timeout.tv_sec += timeout_sec;

	int result = sem_timedwait(&m_input_asynch_done, &timeout);
	if(0 != result)
	{
		result = errno;
		char errmsg[512];
		syslog(LOG_ERR, "spdz2_ext_processor_base::stop_input: sem_timedwait() failed with error %d : %s", result, strerror_r(result, errmsg, 512));
		return -1;
	}

	return (m_input_asynch_success)? 0: -1;
}

//***********************************************************************************************//
void spdz2_ext_processor_base::exec_input_asynch()
{
	std::map< int , shared_input_t >::iterator i = m_shared_inputs.find(m_input_asynch_pid);
	if(m_shared_inputs.end() != i)
	{
		if((i->second.share_count - i->second.share_index) >= m_input_asynch_count)
		{
			for(size_t j = 0; j < m_input_asynch_count; ++j)
			{
				mpz_set(m_input_asynch_output[j], i->second.shared_values[i->second.share_index++]);
			}
			m_input_asynch_success = true;
		}
		else
		{
			syslog(LOG_ERR, "spdz2_ext_processor_base::exec_input_synch: not enough input for pid %d; required %lu; available %lu;",
					m_input_asynch_pid, m_input_asynch_count, (i->second.share_count - i->second.share_index));
		}
	}
	else
	{
		syslog(LOG_ERR, "spdz2_ext_processor_base::exec_input_synch: failed to get input for pid %d.", m_input_asynch_pid);
	}
	sem_post(&m_input_asynch_done);
}

//***********************************************************************************************//
int spdz2_ext_processor_base::start_mult(const size_t share_count, const mpz_t * shares, mpz_t * products, int verify)
{
	if(m_mult_asynch_on)
	{
		syslog(LOG_ERR, "spdz2_ext_processor_base::start_mult: mult is already started.");
		return -1;
	}
	m_mult_asynch_on = true;
	m_mult_asynch_success = false;
	m_mult_asynch_count = share_count;
	m_mult_asynch_input = shares;
	m_mult_asynch_output = products;
	m_mult_asynch_verify = (verify != 0)? true: false;

	if(0 != push_task(spdz2_ext_processor_base::sm_op_code_mult_asynch))
	{
		syslog(LOG_ERR, "spdz2_ext_processor_base::start_mult: failed pushing a task to queue.");
		return -1;
	}
	return 0;
}

//***********************************************************************************************//
int spdz2_ext_processor_base::stop_mult(const time_t timeout_sec)
{
	if(!m_mult_asynch_on)
	{
		syslog(LOG_ERR, "spdz2_ext_processor_base::stop_mult: mult is not started.");
		return -1;
	}
	m_mult_asynch_on = false;

	struct timespec timeout;
	clock_gettime(CLOCK_REALTIME, &timeout);
	timeout.tv_sec += timeout_sec;

	int result = sem_timedwait(&m_mult_asynch_done, &timeout);
	if(0 != result)
	{
		result = errno;
		char errmsg[512];
		syslog(LOG_ERR, "spdz2_ext_processor_base::stop_mult: sem_timedwait() failed with error %d : %s", result, strerror_r(result, errmsg, 512));
		return -1;
	}

	return (m_mult_asynch_success)? 0: -1;
}

//***********************************************************************************************//
void spdz2_ext_processor_base::exec_mult_asynch()
{
	m_mult_asynch_success = protocol_mult(m_mult_asynch_count, m_mult_asynch_input, m_mult_asynch_output, m_mult_asynch_verify);
	sem_post(&m_mult_asynch_done);
}

//***********************************************************************************************//
int spdz2_ext_processor_base::start_share_immediates(const size_t value_count, const mpz_t * values, mpz_t * shares)
{
	if(m_share_immediates_asynch_on)
	{
		syslog(LOG_ERR, "spdz2_ext_processor_base::start_share_immediates: share immediates is already started.");
		return -1;
	}
	m_share_immediates_asynch_on = true;
	m_share_immediates_asynch_success = false;
	m_share_immediates_asynch_count = value_count;
	m_share_immediates_asynch_input = values;
	m_share_immediates_asynch_output = shares;

	if(0 != push_task(spdz2_ext_processor_base::sm_op_code_share_immediates_asynch))
	{
		syslog(LOG_ERR, "spdz2_ext_processor_base::start_share_immediates: failed pushing a task to queue.");
		return -1;
	}
	return 0;
}

//***********************************************************************************************//
int spdz2_ext_processor_base::stop_share_immediates(const time_t timeout_sec)
{
	if(!m_share_immediates_asynch_on)
	{
		syslog(LOG_ERR, "spdz2_ext_processor_base::stop_share_immediates: share immediates is not started.");
		return -1;
	}
	m_share_immediates_asynch_on = false;

	struct timespec timeout;
	clock_gettime(CLOCK_REALTIME, &timeout);
	timeout.tv_sec += timeout_sec;

	int result = sem_timedwait(&m_share_immediates_asynch_done, &timeout);
	if(0 != result)
	{
		result = errno;
		char errmsg[512];
		syslog(LOG_ERR, "spdz2_ext_processor_base::stop_share_immediates: sem_timedwait() failed with error %d : %s", result, strerror_r(result, errmsg, 512));
		return -1;
	}

	return (m_share_immediates_asynch_success)? 0: -1;
}

//***********************************************************************************************//
void spdz2_ext_processor_base::exec_share_immediates_asynch()
{
	m_share_immediates_asynch_success = protocol_share(0, m_share_immediates_asynch_count, m_share_immediates_asynch_input, m_share_immediates_asynch_output);
	sem_post(&m_share_immediates_asynch_done);
}

//***********************************************************************************************//
std::string spdz2_ext_processor_base::get_time_stamp()
{
	struct timespec timestamp;
	char timestamp_buffer[256];
	clock_gettime(CLOCK_REALTIME, &timestamp);
	snprintf(timestamp_buffer, 256, "%lu.%03lu", (u_int64_t)timestamp.tv_sec, (u_int64_t)((timestamp.tv_nsec/1000000)%1000));
	return timestamp_buffer;
}

//***********************************************************************************************//
