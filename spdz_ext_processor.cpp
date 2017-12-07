
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <errno.h>
#include <deque>
#include <syslog.h>

#include "spdz_ext_processor.h"
#include "Protocol.h"
#include "ZpMersenneLongElement.h"

//***********************************************************************************************//
class spdz_ext_processor_cc_imp
{
	pthread_t runner;
	bool run_flag;

	TemplateField<ZpMersenneLongElement> * the_field;
	Protocol<ZpMersenneLongElement> * the_party;
	int party_id, offline_size;

	sem_t task;
	std::deque<int> task_q;
	pthread_mutex_t q_lock;

	void run();
	int push_task(const int op_code);
	int pop_task();

	//--offline------------------------------------------done
	sem_t offline_done;
	void exec_offline();
	bool offline_success;
	//---------------------------------------------------

	//--start_open---------------------------------------done
	bool start_open_on, do_verify;
	std::vector<u_int64_t> shares, opens;
	sem_t open_done;
	void exec_open();
	bool open_success;
	//---------------------------------------------------

	//--triple-------------------------------------------done
	u_int64_t * pa, * pb, * pc;
	sem_t triple_done;
	void exec_triple();
	bool triple_success;
	//---------------------------------------------------

	//--input--------------------------------------------done
	int input_party_id;
	u_int64_t * p_input_value;
	sem_t input_done;
	void exec_input();
	bool input_success;
	//---------------------------------------------------

	//--verify-------------------------------------------done
	bool verification_on;
	int * verification_error;
	sem_t verify_done;
	void exec_verify();
	bool verify_success;
	//---------------------------------------------------

	//--input_asynch-------------------------------------
	bool input_asynch_on;
	int intput_asynch_party_id;
	size_t num_of_inputs;
	std::vector<u_int64_t> input_values;
	sem_t input_asynch_done;
	void exec_input_asynch();
	bool input_asynch_success;
	//---------------------------------------------------

	//--mult---------------------------------------------
	bool mult_on;
	std::vector<u_int64_t> mult_values, products;
	sem_t mult_done;
	void exec_mult();
	bool mult_success;
	//---------------------------------------------------

public:
	spdz_ext_processor_cc_imp();
	~spdz_ext_processor_cc_imp();

	int start(const int pid, const int num_of_parties, const char * field, const int offline_size = 0);
	int stop(const time_t timeout_sec = 2);

	int offline(const int offline_size, const time_t timeout_sec = 2);

	int start_open(const size_t share_count, const u_int64_t * share_values, int verify);
	int stop_open(size_t * open_count, u_int64_t ** open_values, const time_t timeout_sec = 2);

	int triple(u_int64_t * a, u_int64_t * b, u_int64_t * c, const time_t timeout_sec = 2);

	int input(const int input_of_pid, u_int64_t * input_value);

	int start_verify(int * error);
	int stop_verify(const time_t timeout_sec);

    int start_input(const int input_of_pid, const size_t num_of_inputs);
    int stop_input(size_t * input_count, u_int64_t ** inputs);

    int start_mult(const size_t share_count, const u_int64_t * shares, int verify);
    int stop_mult(size_t * product_count, u_int64_t ** products);

    friend void * cc_proc(void * arg);

	static const int op_code_open;
	static const int op_code_triple;
	static const int op_code_offline;
	static const int op_code_input;
	static const int op_code_verify;
	static const int op_code_input_asynch;
	static const int op_code_mult;
};

//***********************************************************************************************//
spdz_ext_processor_ifc::spdz_ext_processor_ifc()
: impl(new spdz_ext_processor_cc_imp)
{
}

//***********************************************************************************************//
spdz_ext_processor_ifc::~spdz_ext_processor_ifc()
{
	delete impl;
}

//***********************************************************************************************//
int spdz_ext_processor_ifc::start(const int pid, const int num_of_parties, const char * field, const int offline_size)
{
	return impl->start(pid, num_of_parties, field, offline_size);
}

//***********************************************************************************************//
int spdz_ext_processor_ifc::stop(const time_t timeout_sec)
{
	return impl->stop(timeout_sec);
}

//***********************************************************************************************//
int spdz_ext_processor_ifc::offline(const int offline_size, const time_t timeout_sec)
{
	return impl->offline(offline_size, timeout_sec);
}

//***********************************************************************************************//
int spdz_ext_processor_ifc::start_open(const size_t share_count, const u_int64_t * share_values, int verify)
{
	return impl->start_open(share_count, share_values, verify);
}

//***********************************************************************************************//
int spdz_ext_processor_ifc::stop_open(size_t * open_count, u_int64_t ** open_values, const time_t timeout_sec)
{
	return impl->stop_open(open_count, open_values, timeout_sec);
}

//***********************************************************************************************//
int spdz_ext_processor_ifc::triple(u_int64_t * a, u_int64_t * b, u_int64_t * c, const time_t timeout_sec)
{
	return impl->triple(a, b, c, timeout_sec);
}

//***********************************************************************************************//
int spdz_ext_processor_ifc::input(const int input_of_pid, u_int64_t * input_value)
{
	return impl->input(input_of_pid, input_value);
}

//***********************************************************************************************//
int spdz_ext_processor_ifc::start_verify(int * error)
{
	return impl->start_verify(error);
}

//***********************************************************************************************//
int spdz_ext_processor_ifc::stop_verify(const time_t timeout_sec)
{
	return impl->stop_verify(timeout_sec);
}

//***********************************************************************************************//
int spdz_ext_processor_ifc::start_input(const int input_of_pid, const size_t num_of_inputs)
{
	return impl->start_input(input_of_pid, num_of_inputs);
}

//***********************************************************************************************//
int spdz_ext_processor_ifc::stop_input(size_t * input_count, u_int64_t ** inputs)
{
	return impl->stop_input(input_count, inputs);
}

//***********************************************************************************************//
int spdz_ext_processor_ifc::start_mult(const size_t share_count, const u_int64_t * shares, int verify)
{
	return impl->start_mult(share_count, shares, verify);
}

//***********************************************************************************************//
int spdz_ext_processor_ifc::stop_mult(size_t * product_count, u_int64_t ** products)
{
	return impl->stop_mult(product_count, products);
}

//***********************************************************************************************//
void * cc_proc(void * arg)
{
	spdz_ext_processor_cc_imp * processor = (spdz_ext_processor_cc_imp *)arg;
	processor->run();
}

//***********************************************************************************************//
const int spdz_ext_processor_cc_imp::op_code_open = 100;
const int spdz_ext_processor_cc_imp::op_code_triple = 101;
const int spdz_ext_processor_cc_imp::op_code_offline = 102;
const int spdz_ext_processor_cc_imp::op_code_input = 103;
const int spdz_ext_processor_cc_imp::op_code_verify = 104;
const int spdz_ext_processor_cc_imp::op_code_input_asynch = 105;
const int spdz_ext_processor_cc_imp::op_code_mult = 106;

//***********************************************************************************************//
spdz_ext_processor_cc_imp::spdz_ext_processor_cc_imp()
 : runner(0), run_flag(false), the_field(NULL), the_party(NULL), party_id(-1), offline_size(-1)
 , offline_success(false)
 , start_open_on(false), do_verify(false), open_success(false)
 , pa(NULL), pb(NULL), pc(NULL), triple_success(false)
 , input_party_id(-1), input_success(false), p_input_value(NULL)
 , verification_error(NULL), verification_on(false), verify_success(false)
 , input_asynch_on(false), intput_asynch_party_id(-1), num_of_inputs(0), input_asynch_success(false)
 , mult_on(false), mult_success(false)
{
	pthread_mutex_init(&q_lock, NULL);
	sem_init(&task, 0, 0);
	sem_init(&open_done, 0, 0);
	sem_init(&triple_done, 0, 0);
	sem_init(&offline_done, 0, 0);
	sem_init(&input_done, 0, 0);
	sem_init(&verify_done, 0, 0);
	sem_init(&input_asynch_done, 0, 0);
	sem_init(&mult_done, 0, 0);
	openlog("spdz_ext_biu", LOG_NDELAY|LOG_PID, LOG_USER);
	setlogmask(LOG_UPTO(LOG_DEBUG));
}

//***********************************************************************************************//
spdz_ext_processor_cc_imp::~spdz_ext_processor_cc_imp()
{
	pthread_mutex_destroy(&q_lock);
	sem_destroy(&task);
	sem_destroy(&open_done);
	sem_destroy(&triple_done);
	sem_destroy(&offline_done);
	sem_destroy(&input_done);
	sem_destroy(&verify_done);
	sem_destroy(&input_asynch_done);
	sem_destroy(&mult_done);
	closelog();
}

//***********************************************************************************************//
int spdz_ext_processor_cc_imp::start(const int pid, const int num_of_parties, const char * field, const int offline)
{
	if(run_flag)
	{
		syslog(LOG_ERR, "spdz_ext_processor_cc_imp::start: this processor is already started");
		return -1;
	}

	std::string input_file;
	{
		char sz[64];
		snprintf(sz, 64, "party_%d_input.txt", pid);
		input_file = sz;
	}
	party_id = pid;
	offline_size = offline;
	the_field = new TemplateField<ZpMersenneLongElement>(0);
	the_party = new Protocol<ZpMersenneLongElement>(num_of_parties, pid, offline, offline, the_field, input_file);
	if(!the_party->offline())
	{
		syslog(LOG_ERR, "spdz_ext_processor_cc_imp::start: protocol library initialization failure.");
		return -1;
	}

	run_flag = true;
	int result = pthread_create(&runner, NULL, cc_proc, this);
	if(0 != result)
	{
		char errmsg[512];
		syslog(LOG_ERR, "spdz_ext_processor_cc_imp::start: pthread_create() failed with error %d : %s", result, strerror_r(result, errmsg, 512));
		run_flag = false;
		return -1;
	}

	syslog(LOG_NOTICE, "spdz_ext_processor_cc_imp::start: pid %d", party_id);
	return 0;
}

//***********************************************************************************************//
int spdz_ext_processor_cc_imp::stop(const time_t timeout_sec)
{
	if(!run_flag)
	{
		syslog(LOG_ERR, "spdz_ext_processor_cc_imp::stop this processor is not running.");
		return -1;
	}

	run_flag = false;
	void * return_code = NULL;

	struct timespec timeout;
	clock_gettime(CLOCK_REALTIME, &timeout);
	timeout.tv_sec += timeout_sec;

	int result = pthread_timedjoin_np(runner, &return_code, &timeout);
	if(0 != result)
	{
		char errmsg[512];
		syslog(LOG_ERR, "spdz_ext_processor_cc_imp::stop: pthread_timedjoin_np() failed with error %d : %s", result, strerror_r(result, errmsg, 512));
		return -1;
	}

	delete the_party;
	the_party = NULL;

	delete the_field;
	the_field = NULL;

	syslog(LOG_NOTICE, "spdz_ext_processor_cc_imp::stop: pid %d", party_id);
	return 0;
}

//***********************************************************************************************//
void spdz_ext_processor_cc_imp::run()
{
	long timeout_ns = 200 /*ms*/ * 1000 /*us*/ * 1000 /*ns*/;
	while(run_flag)
	{
		struct timespec timeout;
		clock_gettime(CLOCK_REALTIME, &timeout);
		timeout.tv_nsec += timeout_ns;
		timeout.tv_sec += timeout.tv_nsec / 1000000000;
		timeout.tv_nsec = timeout.tv_nsec % 1000000000;

		int result = sem_timedwait(&task, &timeout);
		if(0 == result)
		{
			int op_code = pop_task();
			syslog(LOG_NOTICE, "spdz_ext_processor_cc_imp::run: op_code %d", op_code);
			switch(op_code)
			{
			case op_code_open:
				exec_open();
				break;
			case op_code_triple:
				exec_triple();
				break;
			case op_code_offline:
				exec_offline();
				break;
			case op_code_input:
				exec_input();
				break;
			case op_code_verify:
				exec_verify();
				break;
			case op_code_input_asynch:
				exec_input_asynch();
				break;
			case op_code_mult:
				exec_mult();
				break;
			default:
				syslog(LOG_WARNING, "spdz_ext_processor_cc_imp::run: unsupported op_code %d", op_code);
				break;
			}
		}
	}
}

//***********************************************************************************************//
int spdz_ext_processor_cc_imp::push_task(const int op_code)
{
	int result = pthread_mutex_lock(&q_lock);
	if(0 != result)
	{
		char errmsg[512];
		syslog(LOG_ERR, "spdz_ext_processor_cc_imp::push_task: pthread_mutex_lock() failed with error %d : %s", result, strerror_r(result, errmsg, 512));
		return -1;
	}
	task_q.push_back(op_code);
	sem_post(&task);
	result = pthread_mutex_unlock(&q_lock);
	if(0 != result)
	{
		char errmsg[512];
		syslog(LOG_ERR, "spdz_ext_processor_cc_imp::push_task: pthread_mutex_unlock() failed with error %d : %s", result, strerror_r(result, errmsg, 512));
	}
	syslog(LOG_NOTICE, "spdz_ext_processor_cc_imp::push_task: op_code %d", op_code);
	return 0;
}

//***********************************************************************************************//
int spdz_ext_processor_cc_imp::pop_task()
{
	int op_code = -1;
	int result = pthread_mutex_lock(&q_lock);
	if(0 == result)
	{
		if(!task_q.empty())
		{
			op_code = *task_q.begin();
			task_q.pop_front();
		}

		result = pthread_mutex_unlock(&q_lock);
		if(0 != result)
		{
			char errmsg[512];
			syslog(LOG_ERR, "spdz_ext_processor_cc_imp::pop_task: pthread_mutex_unlock() failed with error %d : %s", result, strerror_r(result, errmsg, 512));
		}
	}
	else
	{
		char errmsg[512];
		syslog(LOG_ERR, "spdz_ext_processor_cc_imp::pop_task: pthread_mutex_lock() failed with error %d : %s", result, strerror_r(result, errmsg, 512));
	}

	syslog(LOG_NOTICE, "spdz_ext_processor_cc_imp::pop_task: op_code %d", op_code);
	return op_code;
}

//***********************************************************************************************//
int spdz_ext_processor_cc_imp::offline(const int /*offline_size*/, const time_t timeout_sec)
{
	/*
	 * In BIU implementation the offline size is set at the protocol construction
	 * the call to offline will reallocate the same size of offline
	 */
	offline_success = false;
	if(0 != push_task(spdz_ext_processor_cc_imp::op_code_offline))
	{
		syslog(LOG_ERR, "spdz_ext_processor_cc_imp::offline: failed pushing an offline task to queue.");
		return -1;
	}

	struct timespec timeout;
	clock_gettime(CLOCK_REALTIME, &timeout);
	timeout.tv_sec += timeout_sec;

	int result = sem_timedwait(&offline_done, &timeout);
	if(0 != result)
	{
		result = errno;
		char errmsg[512];
		syslog(LOG_ERR, "spdz_ext_processor_cc_imp::offline: sem_timedwait() failed with error %d : %s", result, strerror_r(result, errmsg, 512));
		return -1;
	}

	return (offline_success)? 0: -1;
}

//***********************************************************************************************//
void spdz_ext_processor_cc_imp::exec_offline()
{
	offline_success = the_party->offline();
	sem_post(&offline_done);
}

//***********************************************************************************************//
int spdz_ext_processor_cc_imp::start_open(const size_t share_count, const u_int64_t * share_values, int verify)
{
	if(start_open_on)
	{
		syslog(LOG_ERR, "spdz_ext_processor_cc_imp::start_open: open is already started (a stop_open() call is required).");
		return -1;
	}
	start_open_on = true;
	open_success = false;

	shares.assign(share_values, share_values + share_count);
	do_verify = (verify != 0)? true: false;
	if(0 != push_task(spdz_ext_processor_cc_imp::op_code_open))
	{
		syslog(LOG_ERR, "spdz_ext_processor_cc_imp::start_open: failed pushing an open task to queue.");
		start_open_on = false;
		return -1;
	}

	return 0;
}

//***********************************************************************************************//
int spdz_ext_processor_cc_imp::stop_open(size_t * open_count, u_int64_t ** open_values, const time_t timeout_sec)
{
	if(!start_open_on)
	{
		syslog(LOG_ERR, "spdz_ext_processor_cc_imp::stop_open: open is not started.");
		return -1;
	}
	start_open_on = false;

	*open_count = 0;
	*open_values = NULL;

	struct timespec timeout;
	clock_gettime(CLOCK_REALTIME, &timeout);
	timeout.tv_sec += timeout_sec;

	int result = sem_timedwait(&open_done, &timeout);
	if(0 != result)
	{
		result = errno;
		char errmsg[512];
		syslog(LOG_ERR, "spdz_ext_processor_cc_imp::stop_open: sem_timedwait() failed with error %d : %s", result, strerror_r(result, errmsg, 512));
		return -1;
	}

	if(open_success)
	{
		if(!opens.empty())
		{
			*open_values = new u_int64_t[*open_count = opens.size()];
			memcpy(*open_values, &opens[0], (*open_count)*sizeof(u_int64_t));
			opens.clear();
		}
		return 0;
	}
	else
	{
		syslog(LOG_ERR, "spdz_ext_processor_cc_imp::stop_open: open failed.");
		return -1;
	}
}

//***********************************************************************************************//
void spdz_ext_processor_cc_imp::exec_open()
{
	std::vector<ZpMersenneLongElement> ext_shares, ext_opens;
	syslog(LOG_INFO, "spdz_ext_processor_cc_imp::exec_open: calling open for %u shares", (u_int32_t)shares.size());
	for(std::vector<u_int64_t>::const_iterator i = shares.begin(); i != shares.end(); ++i)
	{
		ext_shares.push_back(ZpMersenneLongElement(*i));
		syslog(LOG_DEBUG, "spdz_ext_processor_cc_imp::exec_open() share value %lu", *i);
	}
	ext_opens.resize(ext_shares.size());
	shares.clear();
	opens.clear();

	if(open_success = the_party->openShare((int)ext_shares.size(), ext_shares, ext_opens))
	{
		do_verify = false;
		if(!do_verify || the_party->verify())
		{
			syslog(LOG_INFO, "spdz_ext_processor_cc_imp::exec_open: verify open for %u opens", (u_int32_t)ext_opens.size());
			for(std::vector<ZpMersenneLongElement>::const_iterator i = ext_opens.begin(); i != ext_opens.end(); ++i)
			{
				syslog(LOG_DEBUG, "spdz_ext_processor_cc_imp::exec_open() open value %lu", (u_int64_t)i->elem);
				opens.push_back(i->elem);
			}
		}
		else
		{
			syslog(LOG_ERR, "spdz_ext_processor_cc_imp::exec_open: verify failure.");
		}
	}
	else
	{
		syslog(LOG_ERR, "spdz_ext_processor_cc_imp::exec_open: openShare failure.");
		ext_shares.clear();
		ext_opens.clear();
	}
	sem_post(&open_done);
}

//***********************************************************************************************//
int spdz_ext_processor_cc_imp::triple(u_int64_t * a, u_int64_t * b, u_int64_t * c, const time_t timeout_sec)
{
	pa = a;
	pb = b;
	pc = c;
	triple_success = false;

	if(0 != push_task(spdz_ext_processor_cc_imp::op_code_triple))
	{
		syslog(LOG_ERR, "spdz_ext_processor_cc_imp::triple: failed pushing a triple task to queue.");
		return -1;
	}

	struct timespec timeout;
	clock_gettime(CLOCK_REALTIME, &timeout);
	timeout.tv_sec += timeout_sec;

	int result = sem_timedwait(&triple_done, &timeout);
	if(0 != result)
	{
		result = errno;
		char errmsg[512];
		syslog(LOG_ERR, "spdz_ext_processor_cc_imp::triple: sem_timedwait() failed with error %d : %s", result, strerror_r(result, errmsg, 512));
		return -1;
	}

	pa = pb = pc = NULL;
	return (triple_success)? 0: -1;
}

//***********************************************************************************************//
void spdz_ext_processor_cc_imp::exec_triple()
{
	vector<ZpMersenneLongElement> triple(3);
	if(triple_success = the_party->triples(1, triple))
	{
		*pa = triple[0].elem;
		*pb = triple[1].elem;
		*pc = triple[2].elem;
		syslog(LOG_DEBUG, "spdz_ext_processor_cc_imp::exec_triple: share a = %lu; share b = %lu; share c = %lu;", *pa, *pb, *pc);
	}

	{//test the triple with open
		std::vector<ZpMersenneLongElement> ext_shares(3), ext_opens(3);
		ext_shares[0].elem = *pa;
		ext_shares[1].elem = *pb;
		ext_shares[2].elem = *pc;

		if(open_success = the_party->openShare((int)ext_shares.size(), ext_shares, ext_opens))
		{
			syslog(LOG_INFO, "spdz_ext_processor_cc_imp::exec_triple: test open of triple success");
			syslog(LOG_INFO, "spdz_ext_processor_cc_imp::exec_triple: open a = %lu, open b = %lu, open c = %lu", ext_opens[0].elem, ext_opens[1].elem, ext_opens[2].elem);
		}
		else
		{
			syslog(LOG_ERR, "spdz_ext_processor_cc_imp::exec_triple: test open of triple failure");
		}
	}
	sem_post(&triple_done);
}

//***********************************************************************************************//
int spdz_ext_processor_cc_imp::input(const int input_of_pid, u_int64_t * input_value)
{
	p_input_value = input_value;
	input_party_id = input_of_pid;
	input_success = false;

	if(0 != push_task(spdz_ext_processor_cc_imp::op_code_input))
	{
		syslog(LOG_ERR, "spdz_ext_processor_cc_imp::input: failed pushing an input task to queue.");
		return -1;
	}

	//No timeout waiting for input - the user might take long to enter data

	int result = sem_wait(&input_done);
	if(0 != result)
	{
		result = errno;
		char errmsg[512];
		syslog(LOG_ERR, "spdz_ext_processor_cc_imp::input: sem_wait() failed with error %d : %s", result, strerror_r(result, errmsg, 512));
		return -1;
	}

	p_input_value = NULL;
	input_party_id = -1;

	return (input_success)? 0: -1;
}

//***********************************************************************************************//
void spdz_ext_processor_cc_imp::exec_input()
{
	std::vector<ZpMersenneLongElement> input_value(1);
	if(input_success = the_party->input(input_party_id, input_value))
	{
		*p_input_value = input_value[0].elem;
		syslog(LOG_INFO, "spdz_ext_processor_cc_imp::exec_input: input value %lu", *p_input_value);

		{//test the input with open
			std::vector<ZpMersenneLongElement> ext_shares(1), ext_opens(1);
			ext_shares[0].elem = *p_input_value;

			if(open_success = the_party->openShare((int)ext_shares.size(), ext_shares, ext_opens))
			{
				syslog(LOG_INFO, "spdz_ext_processor_cc_imp::exec_input: test open of triple success");
				syslog(LOG_INFO, "spdz_ext_processor_cc_imp::exec_input: open input = %lu", ext_opens[0].elem);
			}
			else
			{
				syslog(LOG_ERR, "spdz_ext_processor_cc_imp::exec_input: test open of input failure");
			}
		}
	}
	else
	{
		syslog(LOG_ERR, "spdz_ext_processor_cc_imp::exec_input: protocol input failure.");
	}
	sem_post(&input_done);
}

//***********************************************************************************************//
int spdz_ext_processor_cc_imp::start_verify(int * error)
{
	if(verification_on)
	{
		syslog(LOG_ERR, "spdz_ext_processor_cc_imp::start_verify: verify is already started (a stop_verify() call is required).");
		return -1;
	}
	verification_on = true;
	verify_success = false;
	verification_error = error;
	if(0 != push_task(spdz_ext_processor_cc_imp::op_code_verify))
	{
		syslog(LOG_ERR, "spdz_ext_processor_cc_imp::start_verify: failed pushing a verify start task to queue.");
		return -1;
	}
	return 0;
}

//***********************************************************************************************//
int spdz_ext_processor_cc_imp::stop_verify(const time_t timeout_sec)
{
	if(!verification_on)
	{
		syslog(LOG_ERR, "spdz_ext_processor_cc_imp::stop_verify: verify is not started.");
		return -1;
	}
	verification_on = false;

	struct timespec timeout;
	clock_gettime(CLOCK_REALTIME, &timeout);
	timeout.tv_sec += timeout_sec;

	int result = sem_timedwait(&verify_done, &timeout);
	if(0 != result)
	{
		result = errno;
		char errmsg[512];
		syslog(LOG_ERR, "spdz_ext_processor_cc_imp::stop_verify: sem_timedwait() failed with error %d : %s", result, strerror_r(result, errmsg, 512));
		return -1;
	}

	verification_error = NULL;
	return (verify_success)? 0: -1;
}

//***********************************************************************************************//
void spdz_ext_processor_cc_imp::exec_verify()
{
	//*verification_error = (verify_success = the_party->verify())? 0: -1;
	*verification_error = 0;
	verify_success = true;
	sem_post(&verify_done);
}

//***********************************************************************************************//
int spdz_ext_processor_cc_imp::start_input(const int input_of_pid, const size_t num_of_inputs_)
{
	if(input_asynch_on)
	{
		syslog(LOG_ERR, "spdz_ext_processor_cc_imp::start_input: asynch input is already started (a stop_input() call is required).");
		return -1;
	}
	input_asynch_on = true;
	input_asynch_success = false;
	intput_asynch_party_id = input_of_pid;
	num_of_inputs = num_of_inputs_;

	if(0 != push_task(spdz_ext_processor_cc_imp::op_code_input_asynch))
	{
		syslog(LOG_ERR, "spdz_ext_processor_cc_imp::start_input: failed pushing an input start task to queue.");
		return -1;
	}
	return 0;
}

//***********************************************************************************************//
int spdz_ext_processor_cc_imp::stop_input(size_t * input_count, u_int64_t ** inputs)
{
	if(!input_asynch_on)
	{
		syslog(LOG_ERR, "spdz_ext_processor_cc_imp::stop_input: asynch input is not started.");
		return -1;
	}
	input_asynch_on = false;

	int result = sem_wait(&input_asynch_done);
	if(0 != result)
	{
		result = errno;
		char errmsg[512];
		syslog(LOG_ERR, "spdz_ext_processor_cc_imp::stop_input: sem_wait() failed with error %d : %s", result, strerror_r(result, errmsg, 512));
		return -1;
	}

	if(input_asynch_success)
	{
		if(!input_values.empty())
		{
			*inputs = new u_int64_t[*input_count = input_values.size()];
			memcpy(*inputs, &input_values[0], *input_count * sizeof(u_int64_t));
		}
		return 0;
	}
	else
	{
		return -1;
	}
}

//***********************************************************************************************//
void spdz_ext_processor_cc_imp::exec_input_asynch()
{
	input_values.clear();
	input_values.resize(num_of_inputs, 0);

	std::vector<ZpMersenneLongElement> ext_inputs(num_of_inputs);
	if(input_asynch_success = the_party->input(intput_asynch_party_id, ext_inputs))
	{
		for(std::vector<ZpMersenneLongElement>::const_iterator i = ext_inputs.begin(); i != ext_inputs.end(); ++i)
		{
			input_values.push_back(i->elem);
		}
	}
	else
	{
		syslog(LOG_ERR, "spdz_ext_processor_cc_imp::exec_input_asynch: protocol input failure.");
	}
	sem_post(&input_asynch_done);
}

//***********************************************************************************************//
int spdz_ext_processor_cc_imp::start_mult(const size_t share_count, const u_int64_t * shares, int verify)
{
	if(mult_on)
	{
		syslog(LOG_ERR, "spdz_ext_processor_cc_imp::start_mult: mult is already started (a stop_mult() call is required).");
		return -1;
	}
	mult_on = true;
	mult_success = false;
	mult_values.assign(shares, shares + share_count);
	products.clear();

	if(0 != push_task(spdz_ext_processor_cc_imp::op_code_mult))
	{
		syslog(LOG_ERR, "spdz_ext_processor_cc_imp::start_mult: failed pushing a mult start task to queue.");
		return -1;
	}
	return 0;

}

//***********************************************************************************************//
int spdz_ext_processor_cc_imp::stop_mult(size_t * product_count, u_int64_t ** product_values)
{
	if(!mult_on)
	{
		syslog(LOG_ERR, "spdz_ext_processor_cc_imp::stop_mult: mult is not started.");
		return -1;
	}
	mult_on = false;

	int result = sem_wait(&mult_done);
	if(0 != result)
	{
		result = errno;
		char errmsg[512];
		syslog(LOG_ERR, "spdz_ext_processor_cc_imp::stop_mult: sem_wait() failed with error %d : %s", result, strerror_r(result, errmsg, 512));
		return -1;
	}

	if(mult_success)
	{
		if(!products.empty())
		{
			*product_values = new u_int64_t[*product_count = products.size()];
			memcpy(*product_values, &products[0], *product_count * sizeof(u_int64_t));
		}
		return 0;
	}
	else
	{
		return -1;
	}

}

//***********************************************************************************************//
void spdz_ext_processor_cc_imp::exec_mult()
{
	size_t xy_pair_count =  mult_values.size()/2;
	std::vector<ZpMersenneLongElement> x_shares(xy_pair_count), y_shares(xy_pair_count), xy_shares(xy_pair_count);

	for(size_t i = 0; i < xy_pair_count; ++i)
	{
		x_shares[i].elem = mult_values[2*i];
		y_shares[i].elem = mult_values[2*i+1];
		syslog(LOG_DEBUG, "spdz_ext_processor_cc_imp::exec_mult: X-Y pair %lu: X=%lu Y=%lu", i, x_shares[i].elem, y_shares[i].elem);
	}

	if(mult_success = the_party->multShares(xy_pair_count, x_shares, y_shares, xy_shares))
	{
		for(size_t i = 0; i < xy_pair_count; ++i)
		{
			products.push_back(xy_shares[i].elem);
			syslog(LOG_DEBUG, "spdz_ext_processor_cc_imp::exec_mult: X-Y product %lu: X*Y=%lu", i, products[i]);
		}
		/*
		for(std::vector<ZpMersenneLongElement>::const_iterator i = xy_shares.begin(); i != xy_shares.end(); ++i)
		{
			products.push_back(i->elem);
			syslog(LOG_DEBUG, "spdz_ext_processor_cc_imp::exec_mult: X-Y product %lu: X*Y=%lu", i, (u_int64_t)i->elem);
		}
		*/
	}
	else
	{
		syslog(LOG_ERR, "spdz_ext_processor_cc_imp::exec_mult: protocol mult failure.");
	}
	sem_post(&mult_done);
}

//***********************************************************************************************//
