
#ifndef SPDZEXT_H_
#define SPDZEXT_H_

#include <stdlib.h>
#include <gmp.h>

extern "C"
{
	int init(void ** handle, const int pid, const int num_of_parties, const int thread_id,
			const char * field, const int open_count, const int mult_count, const int bits_count);

	int term(void * handle);

	int offline(void * handle, const int offline_size);
	int opens(void * handle, const size_t share_count, const mp_limb_t * shares, mp_limb_t * opens, int verify);
    int closes(void * handle, const int party_id, const size_t value_count, const mp_limb_t * values, mp_limb_t * shares);
	int triple(void * handle, mp_limb_t * a, mp_limb_t * b, mp_limb_t * c);
	int verify(void * handle, int * error);
    int input(void * handle, const int input_of_pid, const size_t num_of_inputs, mp_limb_t * inputs);
    int mult(void * handle, const size_t share_count, const mp_limb_t * shares, mp_limb_t * products, int verify);

    int mix_add(void * handle, const mp_limb_t * share, const mp_limb_t * scalar, mp_limb_t * sum);
    int mix_sub_scalar(void * handle, const mp_limb_t * share, const mp_limb_t * scalar, mp_limb_t * diff);
    int mix_sub_share(void * handle, const mp_limb_t * scalar, const mp_limb_t * share, mp_limb_t * diff);
    int mix_mul(void * handle, const mp_limb_t * share, const mp_limb_t * scalar, mp_limb_t * product);
    int adds(void * handle, const mp_limb_t * share1, const mp_limb_t * share2, mp_limb_t * sum);
    int subs(void * handle, const mp_limb_t * share1, const mp_limb_t * share2, mp_limb_t * diff);

    int bit(void * handle, mp_limb_t * share);
    int inverse(void * handle, mp_limb_t * share_value, mp_limb_t * share_inverse);
}

#endif /* SPDZEXT_H_ */
