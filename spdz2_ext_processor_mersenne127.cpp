#include "spdz2_ext_processor_mersenne127.h"

#include <syslog.h>

template <>
TemplateField<Mersenne127>::TemplateField(long fieldParam)
{
    this->elementSizeInBytes = 16;//round up to the next byte
    this->elementSizeInBits = 127;

    auto randomKey = prg.generateKey(128);
    prg.setKey(randomKey);

    m_ZERO = new Mersenne127(0);
    m_ONE = new Mersenne127(1);
}

template <>
void TemplateField<Mersenne127>::elementToBytes(unsigned char* elemenetInBytes, Mersenne127& element)
{
	size_t count = 0;
	u_int8_t buffer[16];
	mpz_export((void *)buffer, &count, 1, 1, 1, 0, *element.get_mpz_t());
	memcpy(elemenetInBytes, buffer, 16);
}

template <>
Mersenne127 TemplateField<Mersenne127>::bytesToElement(unsigned char* elemenetInBytes)
{
	mpz_t value;
	mpz_init(value);
	mpz_import(value, 16, 1, 1, 1, 0, (const void *)elemenetInBytes);
	Mersenne127 element(value);
    return element;
}

template <>
Mersenne127 TemplateField<Mersenne127>::GetElement(long b)
{
	if(b == 1)		return *m_ONE;
    if(b == 0)		return *m_ZERO;
    else
    {
    	Mersenne127 element(b);
        return element;
    }
}

spdz2_ext_processor_mersenne127::spdz2_ext_processor_mersenne127()
 : spdz2_ext_processor_base()
 , the_field(NULL), the_party(NULL)
{
	gmp_randinit_mt(the_gmp_rstate);
}

spdz2_ext_processor_mersenne127::~spdz2_ext_processor_mersenne127()
{
}

int spdz2_ext_processor_mersenne127::mix_add(mpz_t * share, const mpz_t * scalar)
{
	char szsh[128], szsc[128];
	syslog(LOG_INFO, "spdz2_ext_processor_mersenne127::mix_add: (s)%s + (c)%s", mpz_get_str(szsh, 10, *share), mpz_get_str(szsc, 10, *scalar));
	Mersenne127 input(*share), output, arg(*scalar);
	if(Protocol<Mersenne127>::addShareAndScalar(input, arg, output))
	{
		mpz_set(*share, *output.get_mpz_t());
		syslog(LOG_INFO, "spdz2_ext_processor_mersenne127::mix_add: result = (s)%s", mpz_get_str(szsh, 10, *share));
		return 0;
	}
	syslog(LOG_ERR, "spdz2_ext_processor_mersenne127::mix_add: protocol addShareAndScalar failure.");
	return -1;
}

int spdz2_ext_processor_mersenne127::mix_sub_scalar(mpz_t * share, const mpz_t * scalar)
{
	char szsh[128], szsc[128];
	syslog(LOG_INFO, "spdz2_ext_processor_mersenne127::mix_sub_scalar: (s)%s - (c)%s", mpz_get_str(szsh, 10, *share), mpz_get_str(szsc, 10, *scalar));
	Mersenne127 input(*share), output, arg(*scalar);
	if(Protocol<Mersenne127>::shareSubScalar(input, arg, output))
	{
		mpz_set(*share, *output.get_mpz_t());
		syslog(LOG_INFO, "spdz2_ext_processor_mersenne127::mix_sub_scalar: result = (s)%s", mpz_get_str(szsh, 10, *share));
		return 0;
	}
	syslog(LOG_ERR, "spdz2_ext_processor_mersenne127::mix_sub_scalar: protocol shareSubScalar failure.");
	return -1;
}

int spdz2_ext_processor_mersenne127::mix_sub_share(const mpz_t * scalar, mpz_t * share)
{
	char szsh[128], szsc[128];
	syslog(LOG_INFO, "spdz2_ext_processor_mersenne127::mix_sub_share: (c)%s - (s)%s", mpz_get_str(szsc, 10, *scalar), mpz_get_str(szsh, 10, *share));
	Mersenne127 input(*share), output, arg(*scalar);
	if(Protocol<Mersenne127>::scalarSubShare(input, arg, output))
	{
		mpz_set(*share, *output.get_mpz_t());
		syslog(LOG_INFO, "spdz2_ext_processor_mersenne127::mix_sub_share: result = (s)%s", mpz_get_str(szsh, 10, *share));
		return 0;
	}
	syslog(LOG_ERR, "spdz2_ext_processor_mersenne127::mix_sub_share: protocol shareSubScalar failure.");
	return -1;
}

int spdz2_ext_processor_mersenne127::init_protocol()
{
	the_field = new TemplateField<Mersenne127>(0);
	the_party = new Protocol<Mersenne127>(num_of_parties, m_party_id, m_offline_size, m_offline_size, the_field, input_file, "Parties_gfp.txt");
	if(!the_party->offline())
	{
		syslog(LOG_ERR, "spdz2_ext_processor_mersenne127::init_protocol: protocol offline() failure.");
		return -1;
	}
	return 0;
}

void spdz2_ext_processor_mersenne127::delete_protocol()
{
	delete the_party;
	the_party = NULL;
	delete the_field;
	the_field = NULL;
}

bool spdz2_ext_processor_mersenne127::protocol_offline()
{
	return the_party->offline();
}

bool spdz2_ext_processor_mersenne127::protocol_open()
{
	bool op_open_success = false;
	char sz[128];
	std::vector<Mersenne127> ext_shares(open_share_value_count), ext_opens(open_share_value_count);
	syslog(LOG_INFO, "spdz2_ext_processor_mersenne127::protocol_open: calling open for %u shares", (u_int32_t)open_share_value_count);
	for(size_t i = 0; i < open_share_value_count; i++)
	{
		ext_shares[i].set_mpz_t(to_open_share_values + i);
		syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne127::protocol_open() share value[%lu] = %s", i, mpz_get_str(sz, 10, to_open_share_values[i]));
	}

	if(op_open_success = the_party->openShare((int)ext_shares.size(), ext_shares, ext_opens))
	{
		do_verify_open = false;
		if(!do_verify_open || the_party->verify())
		{
			for(size_t i = 0; i < open_share_value_count; i++)
			{
				mpz_set(opened_share_values[i], *ext_opens[i].get_mpz_t());
				syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne127::protocol_open() opened value[%lu] = %s", i, mpz_get_str(sz, 10, opened_share_values[i]));
			}
		}
		else
		{
			syslog(LOG_ERR, "spdz2_ext_processor_mersenne127::protocol_open: verify failure.");
		}
	}
	else
	{
		syslog(LOG_ERR, "spdz2_ext_processor_mersenne127::protocol_open: openShare failure.");
	}
	return op_open_success;
}

bool spdz2_ext_processor_mersenne127::protocol_triple()
{
	bool op_triple_success = false;
	char sza[128], szb[128], szc[128];
	std::vector<Mersenne127> triple(3);
	if(op_triple_success = the_party->triples(1, triple))
	{
		mpz_set(*pa, *triple[0].get_mpz_t());
		mpz_set(*pb, *triple[1].get_mpz_t());
		mpz_set(*pc, *triple[2].get_mpz_t());
		syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne127::protocol_triple: share a = %s; share b = %s; share c = %s;",
				mpz_get_str(sza, 10, *pa), mpz_get_str(szb, 10, *pb), mpz_get_str(szc, 10, *pc));
	}

	/**/
	{//test the triple with open
		std::vector<Mersenne127> ext_shares(3), ext_opens(3);

		ext_shares[0].set_mpz_t(pa);
		ext_shares[1].set_mpz_t(pb);
		ext_shares[2].set_mpz_t(pc);

		if(the_party->openShare((int)ext_shares.size(), ext_shares, ext_opens))
		{
			syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne127::protocol_triple: test open of triple a = %s, open b = %s, open c = %s",
					mpz_get_str(sza, 10, *ext_opens[0].get_mpz_t()), mpz_get_str(szb, 10, *ext_opens[1].get_mpz_t()), mpz_get_str(szc, 10, *ext_opens[2].get_mpz_t()));
		}
		else
		{
			syslog(LOG_ERR, "spdz2_ext_processor_mersenne127::protocol_triple: test open of triple failure");
		}
	}

	return op_triple_success;
}

bool spdz2_ext_processor_mersenne127::protocol_input()
{
	bool op_input_success = false;
	char sz[128];
	std::vector<Mersenne127> input_value(1);
	if(op_input_success = the_party->input(input_party_id, input_value))
	{
		mpz_set(*p_input_value, *input_value[0].get_mpz_t());
		syslog(LOG_INFO, "spdz2_ext_processor_mersenne127::protocol_input: input value %s", mpz_get_str(sz, 10, *input_value[0].get_mpz_t()));

		/**/
		{//test the input with open
			std::vector<Mersenne127> ext_shares(1), ext_opens(1);
			ext_shares[0].set_mpz_t(p_input_value);

			if(the_party->openShare((int)ext_shares.size(), ext_shares, ext_opens))
			{
				syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne127::protocol_input: test open input = %s", mpz_get_str(sz, 10, *ext_opens[0].get_mpz_t()));
			}
			else
			{
				syslog(LOG_ERR, "spdz2_ext_processor_mersenne127::protocol_input: test open of input failure");
			}
		}
	}
	else
	{
		syslog(LOG_ERR, "spdz2_ext_processor_mersenne127::protocol_input: protocol input failure.");
	}
	return op_input_success;
}

bool spdz2_ext_processor_mersenne127::protocol_input_asynch()
{
	bool op_input_asynch_success = false;

	std::vector<Mersenne127> ext_inputs(intput_asynch_count);
	if(op_input_asynch_success = the_party->input(intput_asynch_party_id, ext_inputs))
	{
		for(size_t i = 0; i < intput_asynch_count; ++i)
		{
			mpz_set(intput_asynch_values[i], *ext_inputs[i].get_mpz_t());
		}
	}
	else
	{
		syslog(LOG_ERR, "spdz2_ext_processor_mersenne127::protocol_input_asynch: protocol input failure.");
	}
	return op_input_asynch_success;
}

bool spdz2_ext_processor_mersenne127::protocol_mult()
{
	bool op_mult_success = false;
	char szx[128], szy[128];
	size_t xy_pair_count = mult_share_count/2;
	std::vector<Mersenne127> x_shares(xy_pair_count), y_shares(xy_pair_count), xy_shares(xy_pair_count);

	for(size_t i = 0; i < xy_pair_count; ++i)
	{
		x_shares[i].set_mpz_t(mult_shares+ (2*i));
		y_shares[i].set_mpz_t(mult_shares + (2*i+1));
		syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne127::protocol_mult: X-Y pair %lu: X=%s Y=%s",
				i, mpz_get_str(szx, 10, *x_shares[i].get_mpz_t()), mpz_get_str(szy, 10, *y_shares[i].get_mpz_t()));
	}

	if(op_mult_success = the_party->multShares(xy_pair_count, x_shares, y_shares, xy_shares))
	{
		for(size_t i = 0; i < xy_pair_count; ++i)
		{
			mpz_set(mult_products[i], *xy_shares[i].get_mpz_t());
			syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne127::protocol_mult: X-Y product %lu: X*Y=%s", i, mpz_get_str(szx, 10, *xy_shares[i].get_mpz_t()));
		}
	}
	else
	{
		syslog(LOG_ERR, "spdz2_ext_processor_mersenne127::protocol_mult: protocol mult failure.");
	}
	return op_mult_success;
}

bool spdz2_ext_processor_mersenne127::protocol_share_immediates()
{
	bool op_share_immediates_success = false;
	char sz[128];
	std::vector<Mersenne127> shares(immediates_count);
	vector<std::string> str_immediates_values;
	load_share_immediates_strings(str_immediates_values);

	if(op_share_immediates_success = the_party->load_share_immediates(0, shares, str_immediates_values))
	{
		for(size_t i = 0; i < immediates_count; ++i)
		{
			mpz_set(immediates_shares[i], *shares[i].get_mpz_t());
			syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne127::protocol_share_immediates: share[%lu] = %s", i, mpz_get_str(sz, 10, immediates_shares[i]));
		}
	}
	else
	{
		syslog(LOG_ERR, "spdz2_ext_processor_mersenne127::protocol_share_immediates: protocol share_immediates failure.");
	}
	return op_share_immediates_success;
}

bool spdz2_ext_processor_mersenne127::protocol_share_immediate()
{
	bool op_share_immediate_success = false;
	char sz[128];
	std::vector<Mersenne127> shares(1);
	vector<std::string> str_immediates_value(1);
	str_immediates_value[0] = mpz_get_str(sz, 10, *immediate_value);

	if(op_share_immediate_success = the_party->load_share_immediates(0, shares, str_immediates_value))
	{
		mpz_set(*immediate_share, *shares[0].get_mpz_t());
		syslog(LOG_INFO, "spdz2_ext_processor_mersenne127::protocol_share_immediate: share value %s",
				mpz_get_str(sz, 10, *shares[0].get_mpz_t()));

		/**/
		{//test the input with open
			std::vector<Mersenne127> ext_shares(1), ext_opens(1);
			ext_shares[0].set_mpz_t(immediate_share);

			if(the_party->openShare((int)ext_shares.size(), ext_shares, ext_opens))
			{
				syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne127::protocol_share_immediate: test open share_immediate = %s",
						mpz_get_str(sz, 10, *ext_opens[0].get_mpz_t()));
			}
			else
			{
				syslog(LOG_ERR, "spdz2_ext_processor_mersenne127::protocol_share_immediate: test open of share_immediate failure");
			}
		}
	}
	else
	{
		syslog(LOG_ERR, "spdz2_ext_processor_mersenne127::protocol_share_immediate: protocol load_share_immediates failure.");
	}
	return op_share_immediate_success;
}

bool spdz2_ext_processor_mersenne127::protocol_random_value(mpz_t * value) const
{
	mpz_t max_num;
	mpz_init(max_num);
	mpz_set_str(max_num, Mersenne127::M127, 10);

	mpz_urandomm(*value, the_gmp_rstate, max_num);

	mpz_clear(max_num);

	char sz[128];
	syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne127::protocol_random_value: random value = %s", mpz_get_str(sz, 10, *value));

	return true;
}

bool spdz2_ext_processor_mersenne127::protocol_value_inverse(const mpz_t * value, mpz_t * inverse) const
{
	mpz_t gcd, x, y, P;

	mpz_init(gcd);
	mpz_init(x);
    mpz_init(y);
    mpz_init(P);

    mpz_set_str(P, Mersenne127::M127, 10);

    mpz_gcdext(gcd, x, y, *value, P);
    mpz_mod(*inverse, x, P);

    mpz_clear(gcd);
    mpz_clear(x);
    mpz_clear(y);
    mpz_clear(P);

    char szv[128], szi[128];
    syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne127::protocol_value_inverse: value = %s; inverse = %s;",
    		mpz_get_str(szv, 10, *value), mpz_get_str(szi, 10, *inverse));

	return true;
}
