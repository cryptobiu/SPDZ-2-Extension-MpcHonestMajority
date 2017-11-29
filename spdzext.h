
#ifndef SPDZEXT_H_
#define SPDZEXT_H_

#include <stdlib.h>

extern "C"
{
	int init(void ** handle, const int pid, const char * field, const int offline_size = 0);
	int offline(void * handle, const int offline_size);
	int start_open(void * handle, const size_t share_count, const unsigned long * shares);
	int stop_open(void * handle, size_t * open_count, unsigned long ** opens);
	int triple(void * handle, unsigned long * a, unsigned long * b, unsigned long * c);
	int input(void * handle, const int input_of_pid, unsigned long * input_value);
	int term(void * handle);

	unsigned long test_conversion(const unsigned long);
}

#endif /* SPDZEXT_H_ */
