
#ifndef SPDZEXT_H_
#define SPDZEXT_H_

#include <stdlib.h>

extern "C"
{
	int init(const int pid, const char * field, const int offline_size);
	int start_open(const size_t share_count, const char ** shares, size_t * secret_count, char *** secrets);
	int stop_open(void *);
	int term(void *);
}

#endif /* SPDZEXT_H_ */
