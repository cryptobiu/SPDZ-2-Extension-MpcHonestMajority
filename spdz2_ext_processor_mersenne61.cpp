
#include "spdz2_ext_processor_mersenne61.h"

#include <syslog.h>

const u_int64_t spdz2_ext_processor_mersenne61::mersenne61 = 0x1FFFFFFFFFFFFFFF;

spdz2_ext_processor_mersenne61::spdz2_ext_processor_mersenne61()
 : spdz2_ext_processor_base()
 , the_field(NULL), the_party(NULL)
{
	gmp_randinit_mt(the_gmp_rstate);
}

spdz2_ext_processor_mersenne61::~spdz2_ext_processor_mersenne61()
{
}

int spdz2_ext_processor_mersenne61::mix_add(mpz_t * share, const mpz_t * scalar)
{
	char szsh[128], szsc[128];
	syslog(LOG_INFO, "spdz2_ext_processor_mersenne61::mix_add: (s)%s + (c)%s", mpz_get_str(szsh, 10, *share), mpz_get_str(szsc, 10, *scalar));
	ZpMersenneLongElement input, output, arg;
	input.elem = mpz_get_ui(*share);
	arg.elem = mpz_get_ui(*scalar);
	if(Protocol<ZpMersenneLongElement>::addShareAndScalar(input, arg, output))
	{
		mpz_set_ui(*share, output.elem);
		syslog(LOG_INFO, "spdz2_ext_processor_mersenne61::mix_add: result = (s)%s", mpz_get_str(szsh, 10, *share));
		return 0;
	}
	syslog(LOG_ERR, "spdz2_ext_processor_mersenne61::mix_add: protocol addShareAndScalar failure.");
	return -1;
}

int spdz2_ext_processor_mersenne61::mix_sub_scalar(mpz_t * share, const mpz_t * scalar)
{
	char szsh[128], szsc[128];
	syslog(LOG_INFO, "spdz2_ext_processor_mersenne61::mix_sub_scalar: (s)%s - (c)%s", mpz_get_str(szsh, 10, *share), mpz_get_str(szsc, 10, *scalar));
	ZpMersenneLongElement input, output, arg;
	input.elem = mpz_get_ui(*share);
	arg.elem = mpz_get_ui(*scalar);
	if(Protocol<ZpMersenneLongElement>::shareSubScalar(input, arg, output))
	{
		mpz_set_ui(*share, output.elem);
		syslog(LOG_INFO, "spdz2_ext_processor_mersenne61::mix_sub_scalar: result = (s)%s", mpz_get_str(szsh, 10, *share));
		return 0;
	}
	syslog(LOG_ERR, "spdz2_ext_processor_mersenne61::mix_sub_scalar: protocol shareSubScalar failure.");
	return -1;
}

int spdz2_ext_processor_mersenne61::mix_sub_share(const mpz_t * scalar, mpz_t * share)
{
	char szsh[128], szsc[128];
	syslog(LOG_INFO, "spdz2_ext_processor_mersenne61::mix_sub_share: (c)%s - (s)%s", mpz_get_str(szsc, 10, *scalar), mpz_get_str(szsh, 10, *share));
	ZpMersenneLongElement input, output, arg;
	input.elem = mpz_get_ui(*share);
	arg.elem = mpz_get_ui(*scalar);
	if(Protocol<ZpMersenneLongElement>::scalarSubShare(input, arg, output))
	{
		mpz_set_ui(*share, output.elem);
		syslog(LOG_INFO, "spdz2_ext_processor_mersenne61::mix_sub_share: result = (s)%s", mpz_get_str(szsh, 10, *share));
		return 0;
	}
	syslog(LOG_ERR, "spdz2_ext_processor_mersenne61::mix_sub_share: protocol shareSubScalar failure.");
	return -1;
}

int spdz2_ext_processor_mersenne61::init_protocol()
{
	the_field = new TemplateField<ZpMersenneLongElement>(0);
	the_party = new Protocol<ZpMersenneLongElement>(num_of_parties, m_party_id, m_offline_size, m_offline_size, the_field, input_file, "Parties_gfp.txt");
	if(!the_party->offline())
	{
		syslog(LOG_ERR, "spdz2_ext_processor_mersenne61::init_protocol: protocol offline() failure.");
		return -1;
	}
	return 0;
}

void spdz2_ext_processor_mersenne61::delete_protocol()
{
	delete the_party;
	the_party = NULL;
	delete the_field;
	the_field = NULL;
}

bool spdz2_ext_processor_mersenne61::protocol_offline()
{
	return the_party->offline();
}

bool spdz2_ext_processor_mersenne61::protocol_open()
{
	bool op_open_success = false;
	std::vector<ZpMersenneLongElement> ext_shares(open_share_value_count), ext_opens(open_share_value_count);
	syslog(LOG_INFO, "spdz2_ext_processor_mersenne61::protocol_open: calling open for %u shares", (u_int32_t)open_share_value_count);
	for(size_t i = 0; i < open_share_value_count; i++)
	{
		ext_shares[i].elem = mpz_get_ui(to_open_share_values[i]);
		syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne61::protocol_open() share value[%lu] = %lu", i, ext_shares[i].elem);
	}

	if(op_open_success = the_party->openShare((int)ext_shares.size(), ext_shares, ext_opens))
	{
		do_verify_open = false;
		if(!do_verify_open || the_party->verify())
		{
			for(size_t i = 0; i < open_share_value_count; i++)
			{
				mpz_set_ui(opened_share_values[i], ext_opens[i].elem);
				syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne61::protocol_open() opened value[%lu] = %lu", i, ext_opens[i].elem);
			}
		}
		else
		{
			syslog(LOG_ERR, "spdz2_ext_processor_mersenne61::protocol_open: verify failure.");
		}
	}
	else
	{
		syslog(LOG_ERR, "spdz2_ext_processor_mersenne61::protocol_open: openShare failure.");
	}
	return op_open_success;
}

bool spdz2_ext_processor_mersenne61::protocol_triple()
{
	bool op_triple_success = false;
	std::vector<ZpMersenneLongElement> triple(3);
	if(op_triple_success = the_party->triples(1, triple))
	{
		mpz_set_ui(*pa, triple[0].elem);
		mpz_set_ui(*pb, triple[1].elem);
		mpz_set_ui(*pc, triple[2].elem);
		syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne61::protocol_triple: share a = %lu; share b = %lu; share c = %lu;",
							triple[0].elem, triple[1].elem, triple[2].elem);
	}

	/**/
	{//test the triple with open
		std::vector<ZpMersenneLongElement> ext_shares(3), ext_opens(3);

		ext_shares[0].elem = mpz_get_ui(*pa);
		ext_shares[1].elem = mpz_get_ui(*pb);
		ext_shares[2].elem = mpz_get_ui(*pc);

		if(the_party->openShare((int)ext_shares.size(), ext_shares, ext_opens))
		{
			syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne61::protocol_triple: test open of triple a = %lu, open b = %lu, open c = %lu", ext_opens[0].elem, ext_opens[1].elem, ext_opens[2].elem);
		}
		else
		{
			syslog(LOG_ERR, "spdz2_ext_processor_mersenne61::protocol_triple: test open of triple failure");
		}
	}

	return op_triple_success;
}

bool spdz2_ext_processor_mersenne61::protocol_input()
{
	bool op_input_success = false;
	std::vector<ZpMersenneLongElement> input_value(1);
	if(op_input_success = the_party->input(input_party_id, input_value))
	{
		mpz_set_ui(*p_input_value, input_value[0].elem);
		syslog(LOG_INFO, "spdz2_ext_processor_mersenne61::protocol_input: input value %lu", input_value[0].elem);

		/**/
		{//test the input with open
			std::vector<ZpMersenneLongElement> ext_shares(1), ext_opens(1);
			ext_shares[0].elem = mpz_get_ui(*p_input_value);

			if(the_party->openShare((int)ext_shares.size(), ext_shares, ext_opens))
			{
				syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne61::protocol_input: test open input = %lu", ext_opens[0].elem);
			}
			else
			{
				syslog(LOG_ERR, "spdz2_ext_processor_mersenne61::protocol_input: test open of input failure");
			}
		}
	}
	else
	{
		syslog(LOG_ERR, "spdz2_ext_processor_mersenne61::protocol_input: protocol input failure.");
	}
	return op_input_success;
}

bool spdz2_ext_processor_mersenne61::protocol_input_asynch()
{
	bool op_input_asynch_success = false;

	std::vector<ZpMersenneLongElement> ext_inputs(intput_asynch_count);
	if(op_input_asynch_success = the_party->input(intput_asynch_party_id, ext_inputs))
	{
		for(size_t i = 0; i < intput_asynch_count; ++i)
		{
			mpz_set_ui(intput_asynch_values[i], ext_inputs[i].elem);
		}
	}
	else
	{
		syslog(LOG_ERR, "spdz2_ext_processor_mersenne61::protocol_input_asynch: protocol input failure.");
	}
	return op_input_asynch_success;
}

bool spdz2_ext_processor_mersenne61::protocol_mult()
{
	bool op_mult_success = false;
	size_t xy_pair_count = mult_share_count/2;
	std::vector<ZpMersenneLongElement> x_shares(xy_pair_count), y_shares(xy_pair_count), xy_shares(xy_pair_count);

	for(size_t i = 0; i < xy_pair_count; ++i)
	{
		x_shares[i].elem = mpz_get_ui(mult_shares[2*i]);
		y_shares[i].elem = mpz_get_ui(mult_shares[2*i+1]);
		syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne61::protocol_mult: X-Y pair %lu: X=%lu Y=%lu", i, x_shares[i].elem, y_shares[i].elem);
	}

	if(op_mult_success = the_party->multShares(xy_pair_count, x_shares, y_shares, xy_shares))
	{
		for(size_t i = 0; i < xy_pair_count; ++i)
		{
			mpz_set_ui(mult_products[i], xy_shares[i].elem);
			syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne61::protocol_mult: X-Y product %lu: X*Y=%lu", i, xy_shares[i].elem);
		}
	}
	else
	{
		syslog(LOG_ERR, "spdz2_ext_processor_mersenne61::protocol_mult: protocol mult failure.");
	}
	return op_mult_success;
}

bool spdz2_ext_processor_mersenne61::protocol_share_immediates()
{
	bool op_share_immediates_success = false;
	std::vector<ZpMersenneLongElement> shares(immediates_count);
	vector<std::string> str_immediates_values;
	load_share_immediates_strings(str_immediates_values);

	if(op_share_immediates_success = the_party->load_share_immediates(0, shares, str_immediates_values))
	{
		for(size_t i = 0; i < immediates_count; ++i)
		{
			mpz_set_ui(immediates_shares[i], shares[i].elem);
			syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne61::protocol_share_immediates: share[%lu] = %lu", i, shares[i].elem);
		}
	}
	else
	{
		syslog(LOG_ERR, "spdz2_ext_processor_mersenne61::protocol_share_immediates: protocol share_immediates failure.");
	}
	return op_share_immediates_success;
}

bool spdz2_ext_processor_mersenne61::protocol_share_immediate()
{
	bool op_share_immediate_success = false;
	char sz[128];
	std::vector<ZpMersenneLongElement> shares(1);
	vector<std::string> str_immediates_value(1);
	str_immediates_value[0] = mpz_get_str(sz, 10, *immediate_value);

	if(op_share_immediate_success = the_party->load_share_immediates(0, shares, str_immediates_value))
	{
		mpz_set_ui(*immediate_share, shares[0].elem);
		syslog(LOG_INFO, "spdz2_ext_processor_mersenne61::protocol_share_immediate: share value %lu", shares[0].elem);

		/**/
		{//test the input with open
			std::vector<ZpMersenneLongElement> ext_shares(1), ext_opens(1);
			ext_shares[0].elem = mpz_get_ui(*immediate_share);

			if(the_party->openShare((int)ext_shares.size(), ext_shares, ext_opens))
			{
				syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne61::protocol_share_immediate: test open share_immediate = %lu", ext_opens[0].elem);
			}
			else
			{
				syslog(LOG_ERR, "spdz2_ext_processor_mersenne61::protocol_share_immediate: test open of share_immediate failure");
			}
		}
	}
	else
	{
		syslog(LOG_ERR, "spdz2_ext_processor_mersenne61::protocol_share_immediate: protocol load_share_immediates failure.");
	}
	return op_share_immediate_success;
}

bool spdz2_ext_processor_mersenne61::protocol_random_value(mpz_t * value) const
{
	mpz_t max_num;
	mpz_init(max_num);
	mpz_set_ui(max_num, spdz2_ext_processor_mersenne61::mersenne61);

	mpz_urandomm(*value, the_gmp_rstate, max_num);

	mpz_clear(max_num);

	char sz[128];
	syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne61::protocol_random_value: random value = %s", mpz_get_str(sz, 10, *value));

	return true;
}

bool spdz2_ext_processor_mersenne61::protocol_value_inverse(const mpz_t * value, mpz_t * inverse) const
{
	mpz_t gcd, x, y, P;

	mpz_init(gcd);
	mpz_init(x);
    mpz_init(y);
    mpz_init(P);

    mpz_set_ui(P, spdz2_ext_processor_mersenne61::mersenne61);

    mpz_gcdext(gcd, x, y, *value, P);
    mpz_mod(*inverse, x, P);

    mpz_clear(gcd);
    mpz_clear(x);
    mpz_clear(y);
    mpz_clear(P);

    char szv[128], szi[128];
    syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne61::protocol_value_inverse: value = %s; inverse = %s;", mpz_get_str(szv, 10, *value),
    		mpz_get_str(szi, 10, *inverse));

	return true;
}
