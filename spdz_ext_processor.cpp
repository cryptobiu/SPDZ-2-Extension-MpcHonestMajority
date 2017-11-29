
#include <iostream>
#include <string.h>
#include <errno.h>
#include <deque>

#include "spdz_ext_processor.h"
#include "ProtocolParty.h"

//***********************************************************************************************//
class spdz_ext_processor_cc_imp
{
	pthread_t runner;
	bool run_flag;

	ProtocolParty<ZpMersenneLongElement> * the_party;
	int party_id, offline_size;

	sem_t task;
	std::deque<int> task_q;
	pthread_mutex_t q_lock;

	void run();
	int push_task(const int op_code);
	int pop_task();

	//--start_open---------------------------------------
	bool start_open_on, do_verify;
	std::vector<unsigned long> shares, opens;
	sem_t open_done;
	void exec_open();
	//---------------------------------------------------

	//--triple-------------------------------------------
	unsigned long * pa, * pb, * pc;
	sem_t triple_done;
	void exec_triple();
	//---------------------------------------------------

	//--offline------------------------------------------
	int size_of_offline;
	sem_t offline_done;
	void exec_offline();
	//---------------------------------------------------

	//--input--------------------------------------------
	int intput_party_id;
	unsigned long * p_intput_value;
	sem_t input_done;
	void exec_input();
	//---------------------------------------------------

	//--verify-------------------------------------------
	bool verification_on;
	int * verification_error;
	sem_t verify_done;
	void exec_verify();
	//---------------------------------------------------

public:
	spdz_ext_processor_cc_imp();
	~spdz_ext_processor_cc_imp();

	int start(const int pid, const char * field, const int offline_size = 0);
	int stop(const time_t timeout_sec = 2);

	int offline(const int offline_size, const time_t timeout_sec = 2);

	int start_open(const size_t share_count, const unsigned long * share_values, int verify);
	int stop_open(size_t * open_count, unsigned long ** open_values, const time_t timeout_sec = 2);

	int triple(unsigned long * a, unsigned long * b, unsigned long * c, const time_t timeout_sec = 2);

	int input(const int input_of_pid, unsigned long * input_value);

	int start_verify(int * error);
	int stop_verify(const time_t timeout_sec);

	friend void * cc_proc(void * arg);

	static const int op_code_open;
	static const int op_code_triple;
	static const int op_code_offline;
	static const int op_code_input;
	static const int op_code_verify;
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
int spdz_ext_processor_ifc::start(const int pid, const char * field, const int offline_size)
{
	return impl->start(pid, field, offline_size);
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
int spdz_ext_processor_ifc::start_open(const size_t share_count, const unsigned long * share_values, int verify)
{
	return impl->start_open(share_count, share_values, verify);
}

//***********************************************************************************************//
int spdz_ext_processor_ifc::stop_open(size_t * open_count, unsigned long ** open_values, const time_t timeout_sec)
{
	return impl->stop_open(open_count, open_values, timeout_sec);
}

//***********************************************************************************************//
int spdz_ext_processor_ifc::triple(unsigned long * a, unsigned long * b, unsigned long * c, const time_t timeout_sec)
{
	return impl->triple(a, b, c, timeout_sec);
}

//***********************************************************************************************//
int spdz_ext_processor_ifc::input(const int input_of_pid, unsigned long * input_value)
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

//***********************************************************************************************//
spdz_ext_processor_cc_imp::spdz_ext_processor_cc_imp()
 : runner(0), run_flag(false), the_party(NULL), party_id(-1), offline_size(-1)
 , start_open_on(false), pa(NULL), pb(NULL), pc(NULL), size_of_offline(-1)
 , intput_party_id(-1), p_intput_value(NULL), do_verify(false), verification_error(NULL)
 , verification_on(false)
{
	pthread_mutex_init(&q_lock, NULL);
	sem_init(&task, 0, 0);
	sem_init(&open_done, 0, 0);
	sem_init(&triple_done, 0, 0);
	sem_init(&offline_done, 0, 0);
	sem_init(&input_done, 0, 0);
	sem_init(&verify_done, 0, 0);
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
}

//***********************************************************************************************//
int spdz_ext_processor_cc_imp::start(const int pid, const char * field, const int offline)
{
	if(run_flag)
	{
		std::cerr << "spdz_ext_processor_cc_imp::start: this processor is already started" << std::endl;
		return -1;
	}

	party_id = pid;
	offline_size = offline;
	the_party = new ProtocolParty<ZpMersenneLongElement>(party_id, offline_size);
	the_party->init();

	run_flag = true;
	int result = pthread_create(&runner, NULL, cc_proc, this);
	if(0 != result)
	{
		char errmsg[512];
		std::cerr << "spdz_ext_processor_cc_imp::start: pthread_create() failed with error " << result << " : " << strerror_r(result, errmsg, 512) << std::endl;
		run_flag = false;
		return -1;
	}

	std::cout << "spdz_ext_processor_cc_imp::start: pid " << party_id << std::endl;
	return 0;
}

//***********************************************************************************************//
int spdz_ext_processor_cc_imp::stop(const time_t timeout_sec)
{
	if(!run_flag)
	{
		std::cerr << "spdz_ext_processor_cc_imp::stop this processor is not running." << std::endl;
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
		std::cerr << "spdz_ext_processor_cc_imp::stop: pthread_timedjoin_np() failed with error " << result << " : " << strerror_r(result, errmsg, 512) << std::endl;
		return -1;
	}

	std::cout << "spdz_ext_processor_cc_imp::stop: pid " << party_id << std::endl;
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
			std::cout << "spdz_ext_processor_cc_imp::run: op_code " << op_code << std::endl;
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
			default:
				std::cerr << "spdz_ext_processor_cc_imp::run: unsupported op code " << op_code << std::endl;
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
		std::cerr << "spdz_ext_processor_cc_imp::push_task: pthread_mutex_lock() failed with error " << result << " : " << strerror_r(result, errmsg, 512) << std::endl;
		return -1;
	}
	task_q.push_back(op_code);
	sem_post(&task);
	result = pthread_mutex_unlock(&q_lock);
	if(0 != result)
	{
		char errmsg[512];
		std::cerr << "spdz_ext_processor_cc_imp::push_task: pthread_mutex_unlock() failed with error " << result << " : " << strerror_r(result, errmsg, 512) << std::endl;
	}
	std::cout << "spdz_ext_processor_cc_imp::push_task: op_code " << op_code << std::endl;
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
			std::cerr << "spdz_ext_processor_cc_imp::pop_task: pthread_mutex_unlock() failed with error " << result << " : " << strerror_r(result, errmsg, 512) << std::endl;
		}
	}
	else
	{
		char errmsg[512];
		std::cerr << "spdz_ext_processor_cc_imp::pop_task: pthread_mutex_lock() failed with error " << result << " : " << strerror_r(result, errmsg, 512) << std::endl;
	}

	std::cout << "spdz_ext_processor_cc_imp::pop_task: op_code " << op_code << std::endl;
	return op_code;
}

//***********************************************************************************************//
int spdz_ext_processor_cc_imp::offline(const int offline_size, const time_t timeout_sec)
{
	size_of_offline = offline_size;
	if(0 != push_task(spdz_ext_processor_cc_imp::op_code_offline))
	{
		std::cerr << "spdz_ext_processor_cc_imp::start_open: failed pushing an open task to queue." << std::endl;
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
		std::cerr << "spdz_ext_processor_cc_imp::triple: sem_timedwait() failed with error " << result << " : " << strerror_r(result, errmsg, 512) << std::endl;
		return -1;
	}

	return 0;
}

//***********************************************************************************************//
void spdz_ext_processor_cc_imp::exec_offline()
{
	//the_party->offline(size_of_offline);
	sem_post(&offline_done);
}

//***********************************************************************************************//
int spdz_ext_processor_cc_imp::start_open(const size_t share_count, const unsigned long * share_values, int verify)
{
	if(start_open_on)
	{
		std::cerr << "spdz_ext_processor_cc_imp::start_open: open is already started (a stop_open() call is required)." << std::endl;
		return -1;
	}
	start_open_on = true;

	shares.assign(share_values, share_values + share_count);
	do_verify = (verify != 0)? true: false;
	if(0 != push_task(spdz_ext_processor_cc_imp::op_code_open))
	{
		std::cerr << "spdz_ext_processor_cc_imp::start_open: failed pushing an open task to queue." << std::endl;
		start_open_on = false;
		return -1;
	}

	std::cout << "spdz_ext_processor_cc_imp::start_open: exit" << std::endl;
	return 0;
}

//***********************************************************************************************//
int spdz_ext_processor_cc_imp::stop_open(size_t * open_count, unsigned long ** open_values, const time_t timeout_sec)
{
	if(!start_open_on)
	{
		std::cerr << "spdz_ext_processor_cc_imp::stop_open: open is not started." << std::endl;
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
		std::cerr << "spdz_ext_processor_cc_imp::stop_open: sem_timedwait() failed with error " << result << " : " << strerror_r(result, errmsg, 512) << std::endl;
		return -1;
	}

	if(!opens.empty())
	{
		*open_values = new unsigned long[*open_count = opens.size()];
		memcpy(*open_values, &opens[0], (*open_count)*sizeof(unsigned long));
		opens.clear();
	}

	std::cout << "spdz_ext_processor_cc_imp::stop_open: exit" << std::endl;
	return 0;
}

//***********************************************************************************************//
void spdz_ext_processor_cc_imp::exec_open()
{
	std::vector<ZpMersenneLongElement> ext_shares, ext_opens;
	for(std::vector<unsigned long>::const_iterator i = shares.begin(); i != shares.end(); ++i)
	{
		ext_shares.push_back(ZpMersenneLongElement(*i));
	}
	ext_opens.resize(ext_shares.size());
	shares.clear();

	std::cout << "spdz_ext_processor_cc_imp::exec_open: calling open for " << ext_shares.size() << std::endl;

	the_party->openShare((int)ext_shares.size(), ext_shares, ext_opens);

	opens.clear();

	if(!do_verify || the_party->verify())
	{
		std::cout << "spdz_ext_processor_cc_imp::exec_open: verify open for " << ext_opens.size() << std::endl;
		for(std::vector<ZpMersenneLongElement>::const_iterator i = ext_opens.begin(); i != ext_opens.end(); ++i)
		{
			opens.push_back(i->elem);
		}
	}
	else
	{
		std::cerr << "spdz_ext_processor_cc_imp::exec_open: verify failed - no open values returned." << std::endl;
	}

	std::cout << "spdz_ext_processor_cc_imp::exec_open: posting open done." << std::endl;
	sem_post(&open_done);
}

//***********************************************************************************************//
int spdz_ext_processor_cc_imp::triple(unsigned long * a, unsigned long * b, unsigned long * c, const time_t timeout_sec)
{
	pa = a;
	pb = b;
	pc = c;

	if(0 != push_task(spdz_ext_processor_cc_imp::op_code_triple))
	{
		std::cerr << "spdz_ext_processor_cc_imp::triple: failed pushing a triple task to queue." << std::endl;
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
		std::cerr << "spdz_ext_processor_cc_imp::triple: sem_timedwait() failed with error " << result << " : " << strerror_r(result, errmsg, 512) << std::endl;
		return -1;
	}

	pa = pb = pc = NULL;
	return 0;
}

//***********************************************************************************************//
void spdz_ext_processor_cc_imp::exec_triple()
{
	//ZpMersenneLongElement A, B, C;
	//the_party->triple(A, B, C);		//<--- not yet implemented
	*pa = 0;//A.elem;
	*pb = 0;//B.elem;
	*pc = 0;//C.elem;
	sem_post(&triple_done);
}

//***********************************************************************************************//
int spdz_ext_processor_cc_imp::input(const int input_of_pid, unsigned long * input_value)
{
	p_intput_value = input_value;
	intput_party_id = input_of_pid;

	if(0 != push_task(spdz_ext_processor_cc_imp::op_code_input))
	{
		std::cerr << "spdz_ext_processor_cc_imp::input: failed pushing an input task to queue." << std::endl;
		return -1;
	}

	//No timeout waiting for input - the user might take long to enter data

	int result = sem_wait(&input_done);
	if(0 != result)
	{
		result = errno;
		char errmsg[512];
		std::cerr << "spdz_ext_processor_cc_imp::triple: sem_wait() failed with error " << result << " : " << strerror_r(result, errmsg, 512) << std::endl;
		return -1;
	}

	p_intput_value = NULL;
	intput_party_id = -1;
	return 0;
}

//***********************************************************************************************//
void spdz_ext_processor_cc_imp::exec_input()
{
	//ZpMersenneLongElement input_value;
	//the_party->input(input_party_id, input_value);		//<--- not yet implemented
	*p_intput_value = 0;//input_value.elem;
	sem_post(&input_done);
}

//***********************************************************************************************//
int spdz_ext_processor_cc_imp::start_verify(int * error)
{
	if(verification_on)
	{
		std::cerr << "spdz_ext_processor_cc_imp::start_verify: verify is already started (a stop_verify() call is required)." << std::endl;
		return -1;
	}
	verification_on = true;

	verification_error = error;
	if(0 != push_task(spdz_ext_processor_cc_imp::op_code_verify))
	{
		std::cerr << "spdz_ext_processor_cc_imp::start_verify: failed pushing a verify start task to queue." << std::endl;
		return -1;
	}
	return 0;
}

//***********************************************************************************************//
int spdz_ext_processor_cc_imp::stop_verify(const time_t timeout_sec)
{
	if(!verification_on)
	{
		std::cerr << "spdz_ext_processor_cc_imp::stop_verify: verify is not started." << std::endl;
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
		std::cerr << "spdz_ext_processor_cc_imp::stop_verify: sem_timedwait() failed with error " << result << " : " << strerror_r(result, errmsg, 512) << std::endl;
		return -1;
	}

	return 0;
}

//***********************************************************************************************//
void spdz_ext_processor_cc_imp::exec_verify()
{
	*verification_error = (the_party->verify())? 0: -1;
	sem_post(&verify_done);
}

//***********************************************************************************************//
