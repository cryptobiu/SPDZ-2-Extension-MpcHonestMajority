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
	spdz2_ext_processor_mersenne61();
	virtual ~spdz2_ext_processor_mersenne61();

    virtual int mix_add(mpz_t * share, const mpz_t * scalar);
    virtual int mix_sub_scalar(mpz_t * share, const mpz_t * scalar);
    virtual int mix_sub_share(const mpz_t * scalar, mpz_t * share);

    static const u_int64_t mersenne61;
};
