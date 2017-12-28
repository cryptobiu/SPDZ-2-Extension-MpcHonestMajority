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
	virtual void delete_protocol();
	virtual bool protocol_offline();
	virtual bool protocol_open();
	virtual bool protocol_triple();
	virtual bool protocol_input();
	virtual bool protocol_input_asynch();
	virtual bool protocol_mult();
	virtual bool protocol_share_immediates();
	virtual bool protocol_share_immediate();
	virtual bool protocol_random_value(mpz_t * value) const;
	virtual bool protocol_value_inverse(const mpz_t * value, mpz_t * inverse) const;

public:
	spdz2_ext_processor_gf2n(const int bits);
	virtual ~spdz2_ext_processor_gf2n();

    virtual int mix_add(mpz_t * share, const mpz_t * scalar);
    virtual int mix_sub_scalar(mpz_t * share, const mpz_t * scalar);
    virtual int mix_sub_share(const mpz_t * scalar, mpz_t * share);
};

