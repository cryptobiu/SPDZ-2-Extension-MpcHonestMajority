#pragma once

#include "spdz2_ext_processor_base.h"

#include "Protocol.h"
#include "ZpMersenneLongElement.h"
#include <gmp.h>

class spdz2_ext_processor_mersenne61 : public spdz2_ext_processor_base
{
	TemplateField<ZpMersenneLongElement> * the_field;
	Protocol<ZpMersenneLongElement> * the_party;
	mutable gmp_randstate_t the_gmp_rstate;

	std::vector<ZpMersenneLongElement> m_A_vector, m_B_vector, m_C_vector;

protected:

	virtual int init_protocol(const int open_count, const int mult_count, const int bits_count);
	virtual int delete_protocol();
	virtual bool protocol_offline();
	virtual bool protocol_share(const int pid, const size_t count, const mpz_t * input, mpz_t * output);
	virtual bool protocol_triple(mpz_t * A, mpz_t * B, mpz_t * C);
	virtual bool protocol_random_value(mpz_t * value);
	virtual bool protocol_value_inverse(const mpz_t * value, mpz_t * inverse);
	virtual bool protocol_open(const size_t value_count, const mpz_t * shares, mpz_t * opens, bool verify);
	virtual bool protocol_verify(int * error);
	virtual bool protocol_mult(const size_t count, const mpz_t * input, mpz_t * output, bool verify);
	virtual bool protocol_bits(const size_t count, mpz_t * bit_shares);
	virtual bool protocol_value_mult(const mpz_t * op1, const mpz_t * op2, mpz_t * product);

	virtual std::string get_parties_file();
	virtual std::string get_syslog_name();

public:
	spdz2_ext_processor_mersenne61();
	virtual ~spdz2_ext_processor_mersenne61();

    virtual int mix_add(mpz_t * share, const mpz_t * scalar);
    virtual int mix_sub_scalar(mpz_t * share, const mpz_t * scalar);
    virtual int mix_sub_share(const mpz_t * scalar, mpz_t * share);

    static const u_int64_t mersenne61;
};
