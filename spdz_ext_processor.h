#ifndef SPDZ_EXT_PROCESSOR_H_
#define SPDZ_EXT_PROCESSOR_H_

#include <pthread.h>
#include <semaphore.h>
#include <vector>

class spdz_ext_processor_cc_imp;

class spdz_ext_processor_ifc
{
	spdz_ext_processor_cc_imp * impl;
public:
	spdz_ext_processor_ifc();
	~spdz_ext_processor_ifc();

	int start(const int pid, const char * field, const int offline_size);
	int stop(const time_t timeout_sec = 2);

	int start_open(const size_t share_count, const unsigned long * share_values);
	int stop_open(size_t * open_count, unsigned long ** open_values, const time_t timeout_sec = 2);
};


#endif /* SPDZ_EXT_PROCESSOR_H_ */
