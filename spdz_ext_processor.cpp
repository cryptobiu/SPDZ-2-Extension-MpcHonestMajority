
#include <iostream>
#include <string.h>
#include <errno.h>

#include "spdz_ext_processor.h"
#include "ProtocolParty.h"

//***********************************************************************************************//
class spdz_ext_processor_imp
{
	pthread_t runner;
	bool run_flag, start_on;
	sem_t task, done;
	std::vector<unsigned long> shares, opens;

	int party_id, offline_size;

	void run();

public:
	spdz_ext_processor_imp();
	~spdz_ext_processor_imp();

	int start(const int pid, const char * field, const int offline_size);
	int stop(const time_t timeout_sec = 2);

	int start_open(const size_t share_count, const unsigned long * share_values);
	int stop_open(size_t * open_count, unsigned long ** open_values, const time_t timeout_sec = 2);

	friend void * proc(void * arg);
};

//***********************************************************************************************//
spdz_ext_processor_ifc::spdz_ext_processor_ifc()
: impl(new spdz_ext_processor_imp)
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
void * proc(void * arg)
{
	spdz_ext_processor_imp * processor = (spdz_ext_processor_imp *)arg;
	processor->run();
}

//***********************************************************************************************//
spdz_ext_processor_imp::spdz_ext_processor_imp()
 : runner(0), run_flag(false), start_on(false), party_id(-1), offline_size(-1)
{
	sem_init(&task, 0, 0);
	sem_init(&done, 0, 0);
}

//***********************************************************************************************//
spdz_ext_processor_imp::~spdz_ext_processor_imp()
{
	sem_destroy(&task);
	sem_destroy(&done);
}

//***********************************************************************************************//
int spdz_ext_processor_imp::start(const int pid, const char * field, const int offline)
{
	if(run_flag)
	{
		std::cerr << "spdz_ext_processor_imp::start this processor is already started" << std::endl;
		return -1;
	}

	party_id = pid;
	offline_size = offline;

	run_flag = true;
	int result = pthread_create(&runner, NULL, proc, this);
	if(0 != result)
	{
		char errmsg[512];
		std::cerr << "spdz_ext_processor_imp::start pthread_create() failed with error " << result << " : " << strerror_r(result, errmsg, 512) << std::endl;
		run_flag = false;
		return -1;
	}

	std::cout << "spdz_ext_processor_imp::start with pid " << party_id << std::endl;
	return 0;
}

//***********************************************************************************************//
int spdz_ext_processor_imp::stop(const time_t timeout_sec)
{
	if(!run_flag)
	{
		std::cerr << "spdz_ext_processor_imp::stop this processor is not running." << std::endl;
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
		std::cerr << "spdz_ext_processor_imp::stop pthread_timedjoin_np() failed with error " << result << " : " << strerror_r(result, errmsg, 512) << std::endl;
		return -1;
	}

	std::cout << "spdz_ext_processor_imp::stop with pid " << party_id << std::endl;
	return 0;
}

//***********************************************************************************************//
int spdz_ext_processor_imp::start_open(const size_t share_count, const unsigned long * share_values)
{
	if(!run_flag)
	{
		std::cerr << "spdz_ext_processor_imp::start_open this processor is not running." << std::endl;
		return -1;
	}

	if(start_on)
	{
		std::cerr << "spdz_ext_processor_imp::start_open open is already started (a stop_open() call is required)." << std::endl;
		return -1;
	}

	start_on = true;
	shares.assign(share_values, share_values + share_count);

	sem_post(&task);
	return 0;
}

//***********************************************************************************************//
int spdz_ext_processor_imp::stop_open(size_t * open_count, unsigned long ** open_values, const time_t timeout_sec)
{
	if(!run_flag)
	{
		std::cerr << "spdz_ext_processor_imp::stop_open this processor is not running." << std::endl;
		return -1;
	}

	if(!start_on)
	{
		std::cerr << "spdz_ext_processor_imp::stop_open open is not started." << std::endl;
		return -1;
	}

	*open_count = 0;
	*open_values = NULL;

	struct timespec timeout;
	clock_gettime(CLOCK_REALTIME, &timeout);
	timeout.tv_sec += timeout_sec;

	int result = sem_timedwait(&done, &timeout);
	if(0 != result)
	{
		result = errno;
		char errmsg[512];
		std::cerr << "spdz_ext_processor_imp::stop_open sem_timedwait() failed with error " << result << " : " << strerror_r(result, errmsg, 512) << std::endl;
		return -1;
	}

	if(!opens.empty())
	{
		*open_values = new unsigned long[*open_count = opens.size()];
		memcpy(*open_values, &opens[0], (*open_count)*sizeof(unsigned long));
		opens.clear();
	}

	start_on = false;
	return 0;
}

//***********************************************************************************************//
void spdz_ext_processor_imp::run()
{
	ProtocolParty<ZpMersenneLongElement> * the_party = new ProtocolParty<ZpMersenneLongElement>(party_id, offline_size);
	the_party->init();

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
			std::vector<ZpMersenneLongElement> ext_shares, ext_opens;
			for(std::vector<unsigned long>::const_iterator i = shares.begin(); i != shares.end(); ++i)
			{
				ext_shares.push_back(ZpMersenneLongElement(*i));
			}
			ext_opens.resize(ext_shares.size());
			shares.clear();

			the_party->openShare((int)ext_shares.size(), ext_shares, ext_opens);

			opens.clear();

			if(the_party->verify())
			{
				for(std::vector<ZpMersenneLongElement>::const_iterator i = ext_opens.begin(); i != ext_opens.end(); ++i)
				{
					opens.push_back(i->elem);
				}
			}
			else
			{
				std::cerr << "spdz_ext_processor_imp::run verify failed - no open values returned." << std::endl;
			}

			sem_post(&done);
		}
	}

	delete the_party;
}

//***********************************************************************************************//
