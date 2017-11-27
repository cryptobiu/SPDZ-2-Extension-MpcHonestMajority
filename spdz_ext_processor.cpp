
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
	bool start_open_on;
	std::vector<unsigned long> shares, opens;
	sem_t open_done;
	void exec_open();
	//---------------------------------------------------

public:
	spdz_ext_processor_cc_imp();
	~spdz_ext_processor_cc_imp();

	int start(const int pid, const char * field, const int offline_size);
	int stop(const time_t timeout_sec = 2);

	int start_open(const size_t share_count, const unsigned long * share_values);
	int stop_open(size_t * open_count, unsigned long ** open_values, const time_t timeout_sec = 2);

	friend void * cc_proc(void * arg);

	static const int op_code_open;
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
int spdz_ext_processor_ifc::start_open(const size_t share_count, const unsigned long * share_values)
{
	return impl->start_open(share_count, share_values);
}

//***********************************************************************************************//
int spdz_ext_processor_ifc::stop_open(size_t * open_count, unsigned long ** open_values, const time_t timeout_sec)
{
	return impl->stop_open(open_count, open_values, timeout_sec);
}

//***********************************************************************************************//
void * cc_proc(void * arg)
{
	spdz_ext_processor_cc_imp * processor = (spdz_ext_processor_cc_imp *)arg;
	processor->run();
}

//***********************************************************************************************//
const int spdz_ext_processor_cc_imp::op_code_open = 100;

//***********************************************************************************************//
spdz_ext_processor_cc_imp::spdz_ext_processor_cc_imp()
 : runner(0), run_flag(false), the_party(NULL), party_id(-1), offline_size(-1)
 , start_open_on(false)
{
	pthread_mutex_init(&q_lock, NULL);
	sem_init(&task, 0, 0);
	sem_init(&open_done, 0, 0);
}

//***********************************************************************************************//
spdz_ext_processor_cc_imp::~spdz_ext_processor_cc_imp()
{
	pthread_mutex_destroy(&q_lock);
	sem_destroy(&task);
	sem_destroy(&open_done);
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
int spdz_ext_processor_cc_imp::start_open(const size_t share_count, const unsigned long * share_values)
{
	if(start_open_on)
	{
		std::cerr << "spdz_ext_processor_cc_imp::start_open: open is already started (a stop_open() call is required)." << std::endl;
		return -1;
	}
	start_open_on = true;

	shares.assign(share_values, share_values + share_count);
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

	if(the_party->verify())
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
