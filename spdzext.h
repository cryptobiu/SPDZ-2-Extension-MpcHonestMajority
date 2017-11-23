
#ifndef SPDZEXT_H_
#define SPDZEXT_H_

#include <stdlib.h>

extern "C"
{
	int init(const int pid, const char * field, const int offline_size);
	int start_open(const size_t share_count, const unsigned long * shares, size_t * open_count, unsigned long ** opens);
	int stop_open();
	int term(void *);

	unsigned long test_conversion(const unsigned long);
}

#endif /* SPDZEXT_H_ */
