
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
	syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne61::mix_add: (s)%s + (c)%s", mpz_get_str(szsh, 10, *share), mpz_get_str(szsc, 10, *scalar));
	ZpMersenneLongElement input, output, arg;
	input.elem = mpz_get_ui(*share);
	arg.elem = mpz_get_ui(*scalar);
	if(Protocol<ZpMersenneLongElement>::addShareAndScalar(input, arg, output))
	{
		mpz_set_ui(*share, output.elem);
		syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne61::mix_add: result = (s)%s", mpz_get_str(szsh, 10, *share));
		return 0;
	}
	syslog(LOG_ERR, "spdz2_ext_processor_mersenne61::mix_add: protocol addShareAndScalar failure.");
	return -1;
}

int spdz2_ext_processor_mersenne61::mix_sub_scalar(mpz_t * share, const mpz_t * scalar)
{
	char szsh[128], szsc[128];
	syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne61::mix_sub_scalar: (s)%s - (c)%s", mpz_get_str(szsh, 10, *share), mpz_get_str(szsc, 10, *scalar));
	ZpMersenneLongElement input, output, arg;
	input.elem = mpz_get_ui(*share);
	arg.elem = mpz_get_ui(*scalar);
	if(Protocol<ZpMersenneLongElement>::shareSubScalar(input, arg, output))
	{
		mpz_set_ui(*share, output.elem);
		syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne61::mix_sub_scalar: result = (s)%s", mpz_get_str(szsh, 10, *share));
		return 0;
	}
	syslog(LOG_ERR, "spdz2_ext_processor_mersenne61::mix_sub_scalar: protocol shareSubScalar failure.");
	return -1;
}

int spdz2_ext_processor_mersenne61::mix_sub_share(const mpz_t * scalar, mpz_t * share)
{
	char szsh[128], szsc[128];
	syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne61::mix_sub_share: (c)%s - (s)%s", mpz_get_str(szsc, 10, *scalar), mpz_get_str(szsh, 10, *share));
	ZpMersenneLongElement input, output, arg;
	input.elem = mpz_get_ui(*share);
	arg.elem = mpz_get_ui(*scalar);
	if(Protocol<ZpMersenneLongElement>::scalarSubShare(input, arg, output))
	{
		mpz_set_ui(*share, output.elem);
		syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne61::mix_sub_share: result = (s)%s", mpz_get_str(szsh, 10, *share));
		return 0;
	}
	syslog(LOG_ERR, "spdz2_ext_processor_mersenne61::mix_sub_share: protocol shareSubScalar failure.");
	return -1;
}

int spdz2_ext_processor_mersenne61::init_protocol(const int open_count, const int mult_count, const int bits_count)
{
	syslog(LOG_NOTICE, "spdz2_ext_processor_mersenne61::init_protocol: starting setup [%s]", spdz2_ext_processor_base::get_time_stamp().c_str());
	the_field = new TemplateField<ZpMersenneLongElement>(0);
	the_party = new Protocol<ZpMersenneLongElement>(m_num_of_parties, m_party_id, open_count, mult_count, bits_count, the_field, input_file, "Parties_gfp.txt");
	syslog(LOG_NOTICE, "spdz2_ext_processor_mersenne61::init_protocol: starting offline [%s]", spdz2_ext_processor_base::get_time_stamp().c_str());
	if(!the_party->offline())
	{
		syslog(LOG_ERR, "spdz2_ext_processor_mersenne61::init_protocol: protocol offline() failure.");
		return -1;
	}
	syslog(LOG_NOTICE, "spdz2_ext_processor_mersenne61::init_protocol: starting online [%s]", spdz2_ext_processor_base::get_time_stamp().c_str());
	return 0;
}

int spdz2_ext_processor_mersenne61::delete_protocol()
{
	syslog(LOG_NOTICE, "spdz2_ext_processor_mersenne61::delete_protocol: online done [%s]", spdz2_ext_processor_base::get_time_stamp().c_str());
	delete the_party;
	the_party = NULL;
	delete the_field;
	the_field = NULL;
	return 0;
}

bool spdz2_ext_processor_mersenne61::protocol_offline()
{
	return the_party->offline();
}

bool spdz2_ext_processor_mersenne61::protocol_open(const size_t value_count, const mpz_t * shares, mpz_t * opens, bool verify)
{
	bool success = false;
	std::vector<ZpMersenneLongElement> ext_shares(value_count), ext_opens(value_count);
	syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne61::protocol_open: calling open for %u shares", (u_int32_t)value_count);
	for(size_t i = 0; i < value_count; i++)
	{
		ext_shares[i].elem = mpz_get_ui(shares[i]);
		syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne61::protocol_open() share value[%lu] = %lu", i, ext_shares[i].elem);
	}

	if(success = the_party->openShare((int)ext_shares.size(), ext_shares, ext_opens))
	{
		if(!verify || the_party->verify())
		{
			for(size_t i = 0; i < value_count; i++)
			{
				mpz_set_ui(opens[i], ext_opens[i].elem);
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
	return success;
}

bool spdz2_ext_processor_mersenne61::protocol_triple(mpz_t * A, mpz_t * B, mpz_t * C)
{
	bool op_triple_success = false;
	std::vector<ZpMersenneLongElement> triple(3);
	if(op_triple_success = the_party->triples(1, triple))
	{
		mpz_set_ui(*A, triple[0].elem);
		mpz_set_ui(*B, triple[1].elem);
		mpz_set_ui(*C, triple[2].elem);
		syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne61::protocol_triple: share a = %lu; share b = %lu; share c = %lu;", triple[0].elem, triple[1].elem, triple[2].elem);
	}

	/*
	{//test the triple with open
		std::vector<ZpMersenneLongElement> ext_shares(3), ext_opens(3);

		ext_shares[0].elem = mpz_get_ui(*A);
		ext_shares[1].elem = mpz_get_ui(*B);
		ext_shares[2].elem = mpz_get_ui(*C);

		if(the_party->openShare((int)ext_shares.size(), ext_shares, ext_opens))
		{
			syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne61::protocol_triple: test open of triple a = %lu, open b = %lu, open c = %lu", ext_opens[0].elem, ext_opens[1].elem, ext_opens[2].elem);
		}
		else
		{
			syslog(LOG_ERR, "spdz2_ext_processor_mersenne61::protocol_triple: test open of triple failure");
		}
	}*/

	return op_triple_success;
}

bool spdz2_ext_processor_mersenne61::protocol_mult(const size_t count, const mpz_t * input, mpz_t * output, bool verify)
{
	bool success = false;
	size_t xy_pair_count = count/2;
	std::vector<ZpMersenneLongElement> x_shares(xy_pair_count), y_shares(xy_pair_count), xy_shares(xy_pair_count);

	for(size_t i = 0; i < xy_pair_count; ++i)
	{
		x_shares[i].elem = mpz_get_ui(input[2*i]);
		y_shares[i].elem = mpz_get_ui(input[2*i+1]);
		syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne61::protocol_mult: X-Y pair %lu: X=%lu Y=%lu", i, x_shares[i].elem, y_shares[i].elem);
	}

	if(success = the_party->multShares(xy_pair_count, x_shares, y_shares, xy_shares))
	{
		for(size_t i = 0; i < xy_pair_count; ++i)
		{
			mpz_set_ui(output[i], xy_shares[i].elem);
			syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne61::protocol_mult: X-Y product %lu: X*Y=%lu", i, xy_shares[i].elem);
		}
	}
	else
	{
		syslog(LOG_ERR, "spdz2_ext_processor_mersenne61::protocol_mult: protocol mult failure.");
	}
	return success;
}

bool spdz2_ext_processor_mersenne61::protocol_share(const int pid, const size_t count, const mpz_t * input, mpz_t * output)
{
	bool success = false;
	std::vector<ZpMersenneLongElement> shares(count), values(count);

	for(size_t i = 0; i < count; ++i)
	{
		values[i].elem = mpz_get_ui(input[i]);
	}

	if(success = the_party->makeShare(pid, values, shares))
	{
		for(size_t i = 0; i < count; ++i)
		{
			mpz_set_ui(output[i], shares[i].elem);
			syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne61::protocol_share_immediates: value [%lu] share[%lu] = %lu", values[i].elem, i, shares[i].elem);
		}
	}
	else
	{
		syslog(LOG_ERR, "spdz2_ext_processor_mersenne61::protocol_share_immediates: protocol share_immediates failure.");
	}
	return success;
}

bool spdz2_ext_processor_mersenne61::protocol_random_value(mpz_t * value)
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

bool spdz2_ext_processor_mersenne61::protocol_value_inverse(const mpz_t * value, mpz_t * inverse)
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
    syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne61::protocol_value_inverse: value = %s; inverse = %s;", mpz_get_str(szv, 10, *value), mpz_get_str(szi, 10, *inverse));

	return true;
}

bool spdz2_ext_processor_mersenne61::protocol_verify(int * error)
{
	*error = (the_party->verify())? 0: -1;
}

bool spdz2_ext_processor_mersenne61::protocol_bits(const size_t count, mpz_t * bit_shares)
{
	bool success = false;
	char sz[128];
	std::vector<ZpMersenneLongElement> zbit_shares(count);

	if(success = the_party->bits(count, zbit_shares))
	{
		for(size_t i = 0; i < count; ++i)
		{
			mpz_set_ui(bit_shares[i], zbit_shares[i].elem);
			syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne61::protocol_bits: protocol bits share = %s.", mpz_get_str(sz, 10, bit_shares[i]));
		}
	}
	else
	{
		syslog(LOG_ERR, "spdz2_ext_processor_mersenne61::protocol_bits: protocol bits failure.");
	}
	return success;
}

std::string spdz2_ext_processor_mersenne61::get_syslog_name()
{
	char buffer[32];
	snprintf(buffer, 32, "spdz2x_gfp_m61_%d_%d", m_party_id, m_thread_id);
	return std::string(buffer);
}
