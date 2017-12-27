
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

int spdz2_ext_processor_mersenne61::mix_add(u_int64_t * share, u_int64_t scalar)
{
	syslog(LOG_INFO, "spdz2_ext_processor_mersenne61::mix_add: (s)%lu + (c)%lu", *share, scalar);
	ZpMersenneLongElement input, output, arg;
	input.elem = *share;
	arg.elem = scalar;
	if(Protocol<ZpMersenneLongElement>::addShareAndScalar(input, arg, output))
	{
		*share = output.elem;
		syslog(LOG_INFO, "spdz2_ext_processor_mersenne61::mix_add: result = (s)%lu", *share);
		return 0;
	}
	syslog(LOG_ERR, "spdz2_ext_processor_mersenne61::mix_add: protocol addShareAndScalar failure.");
	return -1;
}

int spdz2_ext_processor_mersenne61::mix_sub_scalar(u_int64_t * share, u_int64_t scalar)
{
	syslog(LOG_INFO, "spdz2_ext_processor_mersenne61::mix_sub_scalar: (s)%lu - (c)%lu", *share, scalar);
	ZpMersenneLongElement input, output, arg;
	input.elem = *share;
	arg.elem = scalar;
	if(Protocol<ZpMersenneLongElement>::shareSubScalar(input, arg, output))
	{
		*share = output.elem;
		syslog(LOG_INFO, "spdz2_ext_processor_mersenne61::mix_sub_scalar: result = (s)%lu", *share);
		return 0;
	}
	syslog(LOG_ERR, "spdz2_ext_processor_mersenne61::mix_sub_scalar: protocol shareSubScalar failure.");
	return -1;
}

int spdz2_ext_processor_mersenne61::mix_sub_share(u_int64_t scalar, u_int64_t * share)
{
	syslog(LOG_INFO, "spdz2_ext_processor_mersenne61::mix_sub_share: (c)%lu - (s)%lu", scalar, *share);
	ZpMersenneLongElement input, output, arg;
	input.elem = *share;
	arg.elem = scalar;
	if(Protocol<ZpMersenneLongElement>::scalarSubShare(input, arg, output))
	{
		*share = output.elem;
		syslog(LOG_INFO, "spdz2_ext_processor_mersenne61::mix_sub_share: result = (s)%lu", *share);
		return 0;
	}
	syslog(LOG_ERR, "spdz2_ext_processor_mersenne61::mix_sub_share: protocol shareSubScalar failure.");
	return -1;
}

int spdz2_ext_processor_mersenne61::init_protocol()
{
	the_field = new TemplateField<ZpMersenneLongElement>(0);
	the_party = new Protocol<ZpMersenneLongElement>(num_of_parties, party_id, offline_size, offline_size, the_field, input_file, "Parties_gfp.txt");
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
	std::vector<ZpMersenneLongElement> ext_shares, ext_opens;
	syslog(LOG_INFO, "spdz2_ext_processor_mersenne61::protocol_open: calling open for %u shares", (u_int32_t)shares.size());
	for(std::vector<u_int64_t>::const_iterator i = shares.begin(); i != shares.end(); ++i)
	{
		ext_shares.push_back(ZpMersenneLongElement(*i));
		syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne61::protocol_open() share value %lu", *i);
	}
	ext_opens.resize(ext_shares.size());
	shares.clear();
	opens.clear();

	if(op_open_success = the_party->openShare((int)ext_shares.size(), ext_shares, ext_opens))
	{
		do_verify_open = false;
		if(!do_verify_open || the_party->verify())
		{
			syslog(LOG_INFO, "spdz2_ext_processor_mersenne61::protocol_open: verify open for %u opens", (u_int32_t)ext_opens.size());
			for(std::vector<ZpMersenneLongElement>::const_iterator i = ext_opens.begin(); i != ext_opens.end(); ++i)
			{
				syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne61::protocol_open() open value %lu", (u_int64_t)i->elem);
				opens.push_back(i->elem);
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
		ext_shares.clear();
		ext_opens.clear();
	}
	return op_open_success;
}

bool spdz2_ext_processor_mersenne61::protocol_triple()
{
	bool op_triple_success = false;
	std::vector<ZpMersenneLongElement> triple(3);
	if(op_triple_success = the_party->triples(1, triple))
	{
		*pa = triple[0].elem;
		*pb = triple[1].elem;
		*pc = triple[2].elem;
		syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne61::protocol_triple: share a = %lu; share b = %lu; share c = %lu;", *pa, *pb, *pc);
	}

	/**/
	{//test the triple with open
		std::vector<ZpMersenneLongElement> ext_shares(3), ext_opens(3);
		ext_shares[0].elem = *pa;
		ext_shares[1].elem = *pb;
		ext_shares[2].elem = *pc;

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
		*p_input_value = input_value[0].elem;
		syslog(LOG_INFO, "spdz2_ext_processor_mersenne61::protocol_input: input value %lu", *p_input_value);

		/**/
		{//test the input with open
			std::vector<ZpMersenneLongElement> ext_shares(1), ext_opens(1);
			ext_shares[0].elem = *p_input_value;

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
	input_values.clear();
	input_values.resize(num_of_inputs, 0);

	std::vector<ZpMersenneLongElement> ext_inputs(num_of_inputs);
	if(op_input_asynch_success = the_party->input(intput_asynch_party_id, ext_inputs))
	{
		for(std::vector<ZpMersenneLongElement>::const_iterator i = ext_inputs.begin(); i != ext_inputs.end(); ++i)
		{
			input_values.push_back(i->elem);
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
	size_t xy_pair_count =  mult_values.size()/2;
	std::vector<ZpMersenneLongElement> x_shares(xy_pair_count), y_shares(xy_pair_count), xy_shares(xy_pair_count);

	for(size_t i = 0; i < xy_pair_count; ++i)
	{
		x_shares[i].elem = mult_values[2*i];
		y_shares[i].elem = mult_values[2*i+1];
		syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne61::protocol_mult: X-Y pair %lu: X=%lu Y=%lu", i, x_shares[i].elem, y_shares[i].elem);
	}

	if(op_mult_success = the_party->multShares(xy_pair_count, x_shares, y_shares, xy_shares))
	{
		for(size_t i = 0; i < xy_pair_count; ++i)
		{
			products.push_back(xy_shares[i].elem);
			syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne61::protocol_mult: X-Y product %lu: X*Y=%lu", i, products[i]);
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
	size_t value_count =  immediates_values.size();
	std::vector<ZpMersenneLongElement> shares(value_count);

	if(op_share_immediates_success = the_party->load_share_immediates(0, shares, immediates_values))
	{
		for(size_t i = 0; i < value_count; ++i)
		{
			immediates_shares.push_back(shares[i].elem);
			syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne61::protocol_share_immediates: share[%lu] = %lu", i, immediates_shares[i]);
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
	std::vector<ZpMersenneLongElement> shares(1);
	if(op_share_immediate_success = the_party->load_share_immediates(0, shares, immediate_value))
	{
		*p_immediate_share = shares[0].elem;
		syslog(LOG_INFO, "spdz2_ext_processor_mersenne61::protocol_share_immediate: share value %lu", *p_immediate_share);

		/**/
		{//test the input with open
			std::vector<ZpMersenneLongElement> ext_shares(1), ext_opens(1);
			ext_shares[0].elem = *p_immediate_share;

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

bool spdz2_ext_processor_mersenne61::protocol_random_value(u_int64_t * value)
{
	mpz_t max_num;
	mpz_init(max_num);
	mpz_set_ui(max_num, spdz2_ext_processor_mersenne61::mersenne61);

	mpz_t random_num;
	mpz_init(random_num);

	mpz_urandomm(random_num, the_gmp_rstate, max_num);
	*value = mpz_get_ui(random_num);

	mpz_clear(max_num);
	mpz_clear(random_num);

	syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne61::protocol_random_value: random value = %lu", *value);

	return true;
}

bool spdz2_ext_processor_mersenne61::protocol_value_inverse(const u_int64_t value, u_int64_t * inverse)
{
	mpz_t gcd, x, y, A, P;

	mpz_init ( gcd );
    mpz_init ( x );
    mpz_init ( y );
    mpz_init ( A );
    mpz_init ( P );

    mpz_set_ui(A, value);
    mpz_set_ui(P, spdz2_ext_processor_mersenne61::mersenne61);

    mpz_gcdext(gcd, x, y, A, P);
    int64_t s_inverse = mpz_get_si(x);
    if(s_inverse < 0) s_inverse += spdz2_ext_processor_mersenne61::mersenne61;
    *inverse = (u_int64_t)s_inverse;

    mpz_clear(gcd);
    mpz_clear(x);
    mpz_clear(y);
    mpz_clear(A);
    mpz_clear(P);

    syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne61::protocol_value_inverse: value = %lu; inverse = %lu;", value, *inverse);

	return true;
}
