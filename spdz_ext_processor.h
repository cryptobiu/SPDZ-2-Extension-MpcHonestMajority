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

	int start(const int pid, const int num_of_parties, const char * field, const int offline_size = 0);
	int stop(const time_t timeout_sec = 2);

	int offline(const int offline_size, const time_t timeout_sec = 2);

	int start_open(const size_t share_count, const u_int64_t * share_values, int verify);
	int stop_open(size_t * open_count, u_int64_t ** open_values, const time_t timeout_sec = 2);

	int triple(u_int64_t * a, u_int64_t * b, u_int64_t * c, const time_t timeout_sec = 2);

	int input(const int input_of_pid, u_int64_t * input_value);

	int start_verify(int * error);
	int stop_verify(const time_t timeout_sec = 2);

    int start_input(const int input_of_pid, const size_t num_of_inputs);
    int stop_input(size_t * input_count, u_int64_t ** inputs);

    int start_mult(const size_t share_count, const u_int64_t * shares, int verify);
    int stop_mult(size_t * product_count, u_int64_t ** products);
};


#endif /* SPDZ_EXT_PROCESSOR_H_ */
