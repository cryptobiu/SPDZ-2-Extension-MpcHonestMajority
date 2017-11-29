
#ifndef SPDZEXT_H_
#define SPDZEXT_H_

#include <stdlib.h>

extern "C"
{
	int init(void ** handle, const int pid, const char * field, const int offline_size = 0);
	int term(void * handle);
	int offline(void * handle, const int offline_size);
	int start_open(void * handle, const size_t share_count, const unsigned long * shares, int verify);
	int stop_open(void * handle, size_t * open_count, unsigned long ** opens);
	int triple(void * handle, unsigned long * a, unsigned long * b, unsigned long * c);
	int input(void * handle, const int input_of_pid, unsigned long * input_value);
	int start_verify(void * handle, int * error);
	int stop_verify(void * handle);
    int start_input(void * handle, const int input_of_pid, const size_t num_of_inputs);
    int stop_input(void * handle, size_t * input_count, unsigned long ** inputs);

	unsigned long test_conversion(const unsigned long);
}

#endif /* SPDZEXT_H_ */
