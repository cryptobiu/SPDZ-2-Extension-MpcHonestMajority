#pragma once

#include "spdz2_ext_processor_base.h"

#include "Protocol.h"

class spdz2_ext_processor_gf2n : public spdz2_ext_processor_base
{
	int gf2n_bits;
	TemplateField<GF2E> * the_field;
	Protocol<GF2E> * the_party;

	void mpz2gf2e(const mpz_t * mpz_value, GF2E & gf2e_value);
	void gf2e2mpz(GF2E & gf2e_value, mpz_t * mpz_value);
	std::string trace(GF2E & value);
protected:

	virtual int init_protocol();
	virtual int delete_protocol();
	virtual bool protocol_offline();
	virtual bool protocol_share(const int pid, const size_t count, const mpz_t * input, mpz_t * output);
	virtual bool protocol_triple(mpz_t * A, mpz_t * B, mpz_t * C);
	virtual bool protocol_random_value(mpz_t * value);
	virtual bool protocol_value_inverse(const mpz_t * value, mpz_t * inverse);
	virtual bool protocol_open(const size_t value_count, const mpz_t * shares, mpz_t * opens, bool verify);
	virtual bool protocol_verify(int * error);
	virtual bool protocol_mult(const size_t count, const mpz_t * input, mpz_t * output, bool verify);

public:
	spdz2_ext_processor_gf2n(const int bits);
	virtual ~spdz2_ext_processor_gf2n();

    virtual int mix_add(mpz_t * share, const mpz_t * scalar);
    virtual int mix_sub_scalar(mpz_t * share, const mpz_t * scalar);
    virtual int mix_sub_share(const mpz_t * scalar, mpz_t * share);
};

