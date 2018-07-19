#pragma once

#include "spdz2_ext_processor_base.h"

#include "Protocol.h"
#include "Mersenne127.h"
#include <gmp.h>

class spdz2_ext_processor_mersenne127 : public spdz2_ext_processor_base
{
	TemplateField<Mersenne127> * the_field;
	Protocol<Mersenne127> * the_party;
	gmp_randstate_t the_gmp_rstate;

public:
	spdz2_ext_processor_mersenne127();
	virtual ~spdz2_ext_processor_mersenne127();

	int init(const int pid, const int num_of_parties, const int thread_id, const char * field,
			 const int open_count, const int mult_count, const int bits_count);
	int term();

	int offline(const int offline_size);
	int triple(mpz_t a, mpz_t b, mpz_t c);
	int share_immediates(const int share_of_pid, const size_t value_count, const mpz_t * values, mpz_t * shares);
    int bit(mpz_t share);
    int inverse(mpz_t share_value, mpz_t share_inverse);
    int open(const size_t share_count, const mpz_t * share_values, mpz_t * opens, int verify);
	int verify(int * error);
    int mult(const size_t share_count, const mpz_t * shares, mpz_t * products, int verify);
    int mix_add(mpz_t share, const mpz_t scalar);
    int mix_sub_scalar(mpz_t share, const mpz_t scalar);
    int mix_sub_share(const mpz_t scalar, mpz_t share);
    std::string get_parties_file();

    static int inverse_value(const mpz_t value, mpz_t inverse);
};
