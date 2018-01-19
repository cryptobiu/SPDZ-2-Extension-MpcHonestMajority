
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
/*ops*/		: m_runner(0), m_run_flag(false), m_party_id(-1), m_offline_size(-1), m_num_of_parties(0)
/*offline*/	, m_offline_synch_success(false)
/*input_s*/	, m_input_synch_success(false), m_input_synch_output(NULL), m_input_synch_pid(-1)
/*triple*/	, m_triple_synch_success(false), m_A(NULL), m_B(NULL), m_C(NULL)
/*shr_im_s*/, m_share_immediate_synch_success(false), m_share_immediate_synch_value(NULL), m_share_immediate_synch_share(NULL)
/*bit*/		, m_bit_synch_success(false), m_bit_synch_share(NULL)
/*inverse*/	, m_inverse_synch_success(false), m_inverse_synch_value_share(NULL), m_inverse_synch_inverse_share(NULL)
/*open*/	, m_open_asynch_on(false), m_open_asynch_success(false), m_open_asynch_count(0), m_open_asynch_input(NULL), m_open_asynch_output(NULL), m_open_asynch_verify(false)
/*verify*/	, m_verify_asynch_on(false), m_verify_asynch_success(false), m_verify_asynch_error(NULL)
/*input_a*/	, m_input_asynch_on(false), m_input_asynch_success(false), m_input_asynch_pid(-1), m_input_asynch_count(0), m_input_asynch_output(NULL), m_input_asynch_input(NULL)
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

	mpz_init(m_input_synch_input);
	mpz_init(m_bit_synch_value);
	mpz_init(m_inverse_synch_value);
	mpz_init(m_inverse_synch_inverse);

	gmp_randinit_mt (m_bit_synch_rand_state);
	gmp_randseed_ui (m_bit_synch_rand_state, 25);

	openlog("spdz_ext_biu", LOG_NDELAY|LOG_PID, LOG_USER);
	setlogmask(LOG_UPTO(LOG_WARNING));
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

	mpz_clear(m_input_synch_input);
	mpz_clear(m_bit_synch_value);
	mpz_clear(m_inverse_synch_value);
	mpz_clear(m_inverse_synch_inverse);

	gmp_randclear (m_bit_synch_rand_state);

	closelog();
}

//***********************************************************************************************//
int spdz2_ext_processor_base::start(const int pid, const int num_of_parties, const char * field, const int offline_size)
{
	if(m_run_flag)
	{
		syslog(LOG_ERR, "spdz2_ext_processor_base::start: this processor is already started");
		return -1;
	}

	m_party_id = pid;
	m_offline_size = offline_size;
	m_num_of_parties = num_of_parties;

	char sz[64];
	snprintf(sz, 64, "party_%d_input.txt", pid);
	input_file = sz;
	load_file_input();

	if(0 != init_protocol())
	{
		syslog(LOG_ERR, "spdz2_ext_processor_base::start: protocol initialization failure.");
		return -1;
	}

	m_run_flag = true;
	int result = pthread_create(&m_runner, NULL, spdz2_ext_processor_proc, this);
	if(0 != result)
	{
		char errmsg[512];
		syslog(LOG_ERR, "spdz2_ext_processor_base::start: pthread_create() failed with error %d : %s", result, strerror_r(result, errmsg, 512));
		m_run_flag = false;
		return -1;
	}

	syslog(LOG_NOTICE, "spdz2_ext_processor_base::start: pid %d", m_party_id);
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
	clear_file_input();
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
			syslog(LOG_NOTICE, "spdz2_ext_processor_base::run: op_code %d", op_code);
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
	syslog(LOG_NOTICE, "spdz2_ext_processor_base::push_task: op_code %d", op_code);
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

	syslog(LOG_NOTICE, "spdz2_ext_processor_base::pop_task: op_code %d", op_code);
	return op_code;
}

//***********************************************************************************************//
void spdz2_ext_processor_base::load_file_input()
{
	char sz[128];
	snprintf(sz, 128, "party_%d_input.txt", m_party_id);
	FILE * pf = fopen(sz, "r");
	if(NULL != pf)
	{
		while(NULL != fgets(sz, 128, pf))
		{
			m_file_input.push_back(sz);
		}
		fclose(pf);
	}
	m_next_file_input = m_file_input.begin();
}

//***********************************************************************************************//
void spdz2_ext_processor_base::clear_file_input()
{
	m_file_input.clear();
}

//***********************************************************************************************//
int spdz2_ext_processor_base::get_input_from_file(mpz_t * value)
{
	if(m_file_input.empty()) return -1;
	if(m_next_file_input == m_file_input.end()) m_next_file_input = m_file_input.begin();
	mpz_set_str(*value, (*(m_next_file_input++)).c_str(), 10);
	return 0;
}

//***********************************************************************************************//
int spdz2_ext_processor_base::get_input_from_user(mpz_t * value)
{
	char sz[128];
	if(NULL != fgets(sz, 128, stdin))
	{
		mpz_set_str(*value, sz, 10);
		return 0;
	}
	return -1;
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
	if(input_of_pid == m_party_id && 0 != get_input(&m_input_synch_input))
	{
		syslog(LOG_ERR, "spdz2_ext_processor_base::input: failed getting input value.");
		return -1;
	}

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
	m_input_synch_success = protocol_share(m_input_synch_pid, 1, &m_input_synch_input, m_input_synch_output);
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
	/*
party [0] value [1] share[0] = 1229106570198852139
party [1] value [1] share[0] = 152370131184010326
party [2] value [1] share[0] = 1381476701382862464

party [0] value [0] share[0] = 284139962301200075
party [1] value [0] share[0] = 568279924602400150
party [2] value [0] share[0] = 852419886903600225
	 */

	static const u_int64_t _0_shares[3] = { 284139962301200075, 568279924602400150, 852419886903600225 };
	static const u_int64_t _1_shares[3] = { 1229106570198852139, 152370131184010326, 1381476701382862464 };

	if(1 == gmp_urandomb_ui (m_bit_synch_rand_state, 1))
	{
		mpz_set_ui(*share, _1_shares[m_party_id%3]);
	}
	else
	{
		mpz_set_ui(*share, _0_shares[m_party_id%3]);
	}
	return 0;

	/*
	m_bit_synch_success = false;

	mpz_set_ui(m_bit_synch_value, 0);//rand()%2);
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
	*/
}

//***********************************************************************************************//
void spdz2_ext_processor_base::exec_bit_synch()
{
	m_bit_synch_success = protocol_share(0, 1, &m_bit_synch_value, m_bit_synch_share);
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

	m_input_asynch_input = new mpz_t[m_input_asynch_count];
	for(size_t i = 0; i < m_input_asynch_count; ++i)
	{
		mpz_init(m_input_asynch_input[i]);
		if(m_party_id == m_input_asynch_pid)
		{
			 get_input(m_input_asynch_input + i);
		}
	}

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

	for(size_t i = 0; i < m_input_asynch_count; ++i)
	{
		mpz_clear(m_input_asynch_input[i]);
	}
	delete []m_input_asynch_input;
	m_input_asynch_input = NULL;

	return (m_input_asynch_success)? 0: -1;
}

//***********************************************************************************************//
void spdz2_ext_processor_base::exec_input_asynch()
{
	m_input_asynch_success = protocol_share(m_input_asynch_pid, m_input_asynch_count, m_input_asynch_input, m_input_asynch_output);
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
