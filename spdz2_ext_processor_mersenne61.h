#pragma once

#include "spdz2_ext_processor_base.h"

#include "Protocol.h"
#include "ZpMersenneLongElement.h"
#include <gmp.h>

class spdz2_ext_processor_mersenne61 : public spdz2_ext_processor_base
{
	TemplateField<ZpMersenneLongElement> * the_field;
	Protocol<ZpMersenneLongElement> * the_party;
	gmp_randstate_t the_gmp_rstate;

public:
	spdz2_ext_processor_mersenne61();
	virtual ~spdz2_ext_processor_mersenne61();

	int init(const int pid, const int num_of_parties, const int thread_id, const char * field,
			 const int open_count, const int mult_count, const int bits_count, int log_level = 700);
	int term();

	int offline(const int offline_size);
	int triple(mp_limb_t * a, mp_limb_t * b, mp_limb_t * c);
	int share_immediates(const int share_of_pid, const size_t value_count, const mpz_t * values, mpz_t * shares);
    int bit(mp_limb_t * share);
    int inverse(mpz_t share_value, mpz_t share_inverse);
    int open(const size_t share_count, const mp_limb_t * share_values, mp_limb_t * opens, int verify);
	int verify(int * error);
    int mult(const size_t share_count, const mpz_t * shares, mpz_t * products, int verify);
    int mix_add(const mp_limb_t * share, const mp_limb_t * scalar, mp_limb_t * sum);
    int mix_sub_scalar(const mp_limb_t * share, const mp_limb_t * scalar, mp_limb_t * diff);
    int mix_sub_share(const mpz_t scalar, mpz_t share);
    int mix_mul(mpz_t share, const mpz_t scalar);
    int adds(mpz_t share1, const mpz_t share2);
    int subs(mpz_t share1, const mpz_t share2);
    std::string get_parties_file();
	std::string get_log_file();
	std::string get_log_category();

	static int inverse_value(const mpz_t value, mpz_t inverse);
	static const u_int64_t mersenne61;
};
