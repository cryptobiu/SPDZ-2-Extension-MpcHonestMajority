
#include <iostream>
#include <string.h>
#include <errno.h>

#include "spdz_ext_processor.h"
#include "ProtocolParty.h"

void * proc(void * arg)
{
	spdz_ext_processor * processor = (spdz_ext_processor *)arg;
	processor->run();
}

spdz_ext_processor::spdz_ext_processor()
 : runner(0), run_flag(false), start_on(false), party_id(-1), offline_size(-1)
{
	sem_init(&task, 0, 0);
	sem_init(&done, 0, 0);
}

spdz_ext_processor::~spdz_ext_processor()
{
	sem_destroy(&task);
	sem_destroy(&done);
}

int spdz_ext_processor::start(const int pid, const char * field, const int offline)
{
	if(run_flag)
	{
		std::cerr << "spdz_ext_processor::start this processor is already started" << std::endl;
		return -1;
	}

	party_id = pid;
	offline_size = offline;

	run_flag = true;
	int result = pthread_create(&runner, NULL, proc, this);
	if(0 != result)
	{
		char errmsg[512];
		std::cerr << "spdz_ext_processor::start pthread_create() failed with error " << result << " : " << strerror_r(result, errmsg, 512) << std::endl;
		run_flag = false;
		return -1;
	}

	std::cout << "spdz_ext_processor::start with pid " << party_id << std::endl;
	return 0;
}

int spdz_ext_processor::stop(const time_t timeout_sec)
{
	if(!run_flag)
	{
		std::cerr << "spdz_ext_processor::stop this processor is not running." << std::endl;
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
		std::cerr << "spdz_ext_processor::stop pthread_timedjoin_np() failed with error " << result << " : " << strerror_r(result, errmsg, 512) << std::endl;
		return -1;
	}

	std::cout << "spdz_ext_processor::stop with pid " << party_id << std::endl;
	return 0;
}

int spdz_ext_processor::start_open(const size_t share_count, const unsigned long * share_values)
{
	if(!run_flag)
	{
		std::cerr << "spdz_ext_processor::start_open this processor is not running." << std::endl;
		return -1;
	}

	if(start_on)
	{
		std::cerr << "spdz_ext_processor::start_open open is already started (a stop_open() call is required)." << std::endl;
		return -1;
	}

	std::cout << "spdz_ext_processor::start_open with share count " << share_count << std::endl;
	start_on = true;
	shares.assign(share_values, share_values + share_count);

	std::cout << "spdz_ext_processor::start_open posting task" << std::endl;
	sem_post(&task);
	return 0;
}

int spdz_ext_processor::stop_open(size_t * open_count, unsigned long ** open_values, const time_t timeout_sec)
{
	if(!run_flag)
	{
		std::cerr << "spdz_ext_processor::stop_open this processor is not running." << std::endl;
		return -1;
	}

	if(!start_on)
	{
		std::cerr << "spdz_ext_processor::stop_open open is not started." << std::endl;
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
		std::cerr << "spdz_ext_processor::stop_open sem_timedwait() failed with error " << result << " : " << strerror_r(result, errmsg, 512) << std::endl;
		return -1;
	}

	if(!opens.empty())
	{
		*open_values = new unsigned long[*open_count = opens.size()];
		memcpy(*open_values, &opens[0], (*open_count)*sizeof(unsigned long));
		opens.clear();
	}
	std::cout << "spdz_ext_processor::stop_open with open count " << *open_count << std::endl;

	start_on = false;
	return 0;
}

void spdz_ext_processor::run()
{
	std::cout << "spdz_ext_processor::run starting" << std::endl;
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
			std::cout << "spdz_ext_processor::run task acquired" << std::endl;

			std::vector<ZpMersenneLongElement> ext_shares, ext_opens;
			for(std::vector<unsigned long>::const_iterator i = shares.begin(); i != shares.end(); ++i)
			{
				ext_shares.push_back(ZpMersenneLongElement(*i));
			}
			ext_opens.resize(ext_shares.size());
			shares.clear();

			std::cout << "spdz_ext_processor::run party invoked" << std::endl;

			the_party->openShare((int)ext_shares.size(), ext_shares, ext_opens);

			std::cout << "spdz_ext_processor::run party done" << std::endl;

			opens.clear();

			if(the_party->verify())
			{
				std::cout << "spdz_ext_processor::run verify success for open count " << ext_opens.size() << std::endl;
				for(std::vector<ZpMersenneLongElement>::const_iterator i = ext_opens.begin(); i != ext_opens.end(); ++i)
				{
					opens.push_back(i->elem);
				}
			}
			else
			{
				std::cerr << "spdz_ext_processor::run verify failed - no open values returned." << std::endl;
			}

			std::cout << "spdz_ext_processor::start_open posting done" << std::endl;
			sem_post(&done);
		}
	}

	delete the_party;
}

