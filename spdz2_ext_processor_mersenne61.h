#pragma once

#include "spdz2_ext_processor_base.h"

#include "Protocol.h"
#include "ZpMersenneLongElement.h"

class spdz2_ext_processor_mersenne61 : public spdz2_ext_processor_base
{
	TemplateField<ZpMersenneLongElement> * the_field;
	Protocol<ZpMersenneLongElement> * the_party;
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

public:
	spdz2_ext_processor_mersenne61();
	virtual ~spdz2_ext_processor_mersenne61();

    virtual int mix_add(u_int64_t * share, u_int64_t scalar);
    virtual int mix_sub_scalar(u_int64_t * share, u_int64_t scalar);
    virtual int mix_sub_share(u_int64_t scalar, u_int64_t * share);
};
