#ifndef SPDZ_EXT_PROCESSOR_H_
#define SPDZ_EXT_PROCESSOR_H_

#include <pthread.h>
#include <semaphore.h>
#include <vector>

class spdz_ext_processor
{
	pthread_t runner;
	bool run_flag, start_on;
	sem_t task, done;
	std::vector<unsigned long> shares, opens;

	int party_id, offline_size;

	void run();

public:
	spdz_ext_processor();
	~spdz_ext_processor();

	int start(const int pid, const char * field, const int offline_size);
	int stop(const time_t timeout_sec = 2);

	int start_open(const size_t share_count, const unsigned long * share_values);
	int stop_open(size_t * open_count, unsigned long ** open_values, const time_t timeout_sec = 2);

	friend void * proc(void * arg);
};



#endif /* SPDZ_EXT_PROCESSOR_H_ */
