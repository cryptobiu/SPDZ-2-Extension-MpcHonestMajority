
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
	int opens(void * handle, const size_t share_count, const mpz_t * shares, mpz_t * opens, int verify);
	int triple(void * handle, mpz_t a, mpz_t b, mpz_t c);
	int verify(void * handle, int * error);
    int input(void * handle, const int input_of_pid, const size_t num_of_inputs, mpz_t * inputs);
    int mult(void * handle, const size_t share_count, const mpz_t * shares, mpz_t * products, int verify);

    int mix_add(void * handle, mpz_t share, const mpz_t scalar);
    int mix_sub_scalar(void * handle, mpz_t share, const mpz_t scalar);
    int mix_sub_share(void * handle, const mpz_t scalar, mpz_t share);
    int mix_mul(void * handle, mpz_t share, const mpz_t scalar);
    int adds(void * handle, mpz_t share1, const mpz_t share2);

    int share_immediates(void * handle, const int party_id, const size_t value_count, const mpz_t * values, mpz_t * shares);

    int bit(void * handle, mpz_t share);
    int inverse(void * handle, mpz_t share_value, mpz_t share_inverse);

    /*
    mpz_t gfp_conversion(const mpz_t * value);
    mpz_t gfp_add(const mpz_t *, const mpz_t *);
    mpz_t gfp_sub(const mpz_t *, const mpz_t *);
    mpz_t gfp_mult(const mpz_t *, const mpz_t *);

    mpz_t gf2n_conversion(const mpz_t value);
    mpz_t gf2n_add(const mpz_t *, const mpz_t *);
    mpz_t gf2n_sub(const mpz_t *, const mpz_t *);
    mpz_t gf2n_mult(const mpz_t *, const mpz_t *);
    */
}

#endif /* SPDZEXT_H_ */
