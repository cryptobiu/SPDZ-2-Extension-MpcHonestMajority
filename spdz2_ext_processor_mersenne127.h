#pragma once

#include "spdz2_ext_processor_base.h"

#include "Protocol.h"
#include "Mersenne127.h"
#include <gmp.h>

class spdz2_ext_processor_mersenne127 : public spdz2_ext_processor_base
{
	TemplateField<ZpMersenne127Element> * the_field;
	Protocol<ZpMersenne127Element> * the_party;
	gmp_randstate_t the_gmp_rstate;

public:
	spdz2_ext_processor_mersenne127();
	virtual ~spdz2_ext_processor_mersenne127();

	int init(const int pid, const int num_of_parties, const int thread_id, const char * field,
			 const int open_count, const int mult_count, const int bits_count, int log_level = 700);
	int term();

	int get_P(mpz_t P);
	int offline(const int offline_size);
	int triple(mp_limb_t * a, mp_limb_t * b, mp_limb_t * c);
	int share_immediates(const int share_of_pid, const size_t value_count, const mp_limb_t * values, mp_limb_t * shares);
    int bit(mp_limb_t * share);
    int open(const size_t share_count, const mp_limb_t * share_values, mp_limb_t * opens, int verify);
	int verify(int * error);
    int mult(const size_t share_count, const mp_limb_t * shares, mp_limb_t * products, int verify);
    int mix_add(const mp_limb_t * share, const mp_limb_t * scalar, mp_limb_t * sum);
    int mix_sub_scalar(const mp_limb_t * share, const mp_limb_t * scalar, mp_limb_t * diff);
    int mix_sub_share(const mp_limb_t * scalar, const mp_limb_t * share, mp_limb_t * diff);
    int mix_mul(const mp_limb_t * share, const mp_limb_t * scalar, mp_limb_t * product);
    int adds(const mp_limb_t * share1, const mp_limb_t * share2, mp_limb_t * sum);
    int subs(const mp_limb_t * share1, const mp_limb_t * share2, mp_limb_t * diff);
    std::string get_parties_file();
    std::string get_log_file();
    std::string get_log_category();

};
