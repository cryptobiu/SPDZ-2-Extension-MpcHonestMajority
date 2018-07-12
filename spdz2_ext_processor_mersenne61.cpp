
#include "spdz2_ext_processor_mersenne61.h"

#include <syslog.h>

const u_int64_t spdz2_ext_processor_mersenne61::mersenne61 = 0x1FFFFFFFFFFFFFFF;

spdz2_ext_processor_mersenne61::spdz2_ext_processor_mersenne61()
: the_field(NULL), the_party(NULL)
{
	gmp_randinit_mt(the_gmp_rstate);
}

spdz2_ext_processor_mersenne61::~spdz2_ext_processor_mersenne61()
{
}

int spdz2_ext_processor_mersenne61::init(const int pid, const int num_of_parties, const int thread_id, const char * field,
		 	 	 	 	 	 	 	 	 const int open_count, const int mult_count, const int bits_count)
{
	spdz2_ext_processor_base::init(pid, num_of_parties, thread_id, field, open_count, mult_count, bits_count);
	the_field = new TemplateField<ZpMersenneLongElement>(0);
	the_party = new Protocol<ZpMersenneLongElement>(m_nparties, m_pid, open_count, mult_count, bits_count, the_field, get_parties_file());
	if(!the_party->offline())
	{
		syslog(LOG_ERR, "spdz2_ext_processor_mersenne61::init_protocol: protocol offline() failure.");
		return -1;
	}
	return 0;
}

int spdz2_ext_processor_mersenne61::term()
{
	delete the_party;
	the_party = NULL;
	delete the_field;
	the_field = NULL;
	return 0;
}

int spdz2_ext_processor_mersenne61::offline(const int offline_size)
{
	return (the_party->offline())? 0: -1;
}

int spdz2_ext_processor_mersenne61::input(const int input_of_pid, mpz_t input_value)
{
	std::map< int , shared_input_t >::iterator i = m_shared_inputs.find(input_of_pid);

	if(m_shared_inputs.end() != i && 0 < i->second.share_count && i->second.share_index < i->second.share_count)
	{
		mpz_set(input_value, i->second.shared_values[i->second.share_index++]);
		return 0;
	}
	syslog(LOG_ERR, "spdz2_ext_processor_mersenne61::input: failed to get input for pid %d.", input_of_pid);
	return -1;
}

int spdz2_ext_processor_mersenne61::triple(mpz_t a, mpz_t b, mpz_t c)
{
	std::vector<ZpMersenneLongElement> triple(3);
	if(the_party->triples(1, triple))
	{
		syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne61::protocol_triple: share a = %lu; share b = %lu; share c = %lu;", triple[0].elem, triple[1].elem, triple[2].elem);
		mpz_set_ui(a, triple[0].elem);
		mpz_set_ui(b, triple[1].elem);
		mpz_set_ui(c, triple[2].elem);
		return 0;
	}
	else
	{
		syslog(LOG_ERR, "spdz2_ext_processor_mersenne61::triple: protocol triples failure.");
	}
	return -1;
}

int spdz2_ext_processor_mersenne61::share_immediates(const int share_of_pid, const size_t value_count, const mpz_t * values, mpz_t * shares)
{
	std::vector<ZpMersenneLongElement> m61shares(value_count), m61values(value_count);
	if(share_of_pid == m_pid)
	{
		for(size_t i = 0; i < value_count; ++i)
		{
			m61values[i].elem = mpz_get_ui(values[i]);
		}
	}

	if(the_party->makeShare(share_of_pid, m61values, m61shares))
	{
		for(size_t i = 0; i < value_count; ++i)
		{
			mpz_set_ui(shares[i], m61shares[i].elem);
			syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne61::protocol_share_immediates: value [%lu] share[%lu] = %lu", m61values[i].elem, i, m61shares[i].elem);
		}
		return 0;
	}
	else
	{
		syslog(LOG_ERR, "spdz2_ext_processor_mersenne61::protocol_share_immediates: protocol share_immediates failure.");
	}
	return -1;
}

int spdz2_ext_processor_mersenne61::bit(mpz_t share)
{
	std::vector<ZpMersenneLongElement> zbit_shares(1);
	if(the_party->bits(1, zbit_shares))
	{
		mpz_set_ui(share, zbit_shares[0].elem);
		syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne61::bit: protocol bits share = %lu.", zbit_shares[0].elem);
		return 0;
	}
	else
	{
		syslog(LOG_ERR, "spdz2_ext_processor_mersenne61::bit: protocol bits failure.");
	}
	return -1;
}

int spdz2_ext_processor_mersenne61::inverse(mpz_t x, mpz_t y)
{
/*
1.      Non-interactively generate a share of a field element [x]				]
2.      Non-interactively generate a share of another filed element [r].		] All 3 are implemented below
3.      MPC multiply [u] = [x][r]												] using protocol_triple
4.      Open u=[u]
5.      Non-interactively inverse v=1/u
6.      Non-interactively multiply [y] =v [r]
7.		Now [y] [x] =1 holds.
*/

	int result = -1;
	mpz_t r, u, open_u, v, product;

	mpz_init(r);
	mpz_init(u);
	mpz_init(open_u);
	mpz_init(v);
	mpz_init(product);

	if(triple(x, r, u))
	{
		if(open(1, &u, &open_u, true))
		{
			value_inverse(open_u, v);
			mpz_mul(product, v, r);
			mpz_mod_ui(y, product, spdz2_ext_processor_mersenne61::mersenne61);
			result = 0;
		}
		else
			syslog(LOG_ERR, "spdz2_ext_processor_base::exec_inverse_synch: protocol_open() failed");
	}
	else
		syslog(LOG_ERR, "spdz2_ext_processor_base::exec_inverse_synch: protocol_triple() failed");

	mpz_clear(r);
	mpz_clear(u);
	mpz_clear(open_u);
	mpz_clear(v);
	mpz_clear(product);

	return result;
}

int spdz2_ext_processor_mersenne61::value_inverse(const mpz_t value, mpz_t inverse)
{
	mpz_t gcd, x, y, P;

	mpz_init(gcd);
	mpz_init(x);
	mpz_init(y);
	mpz_init(P);

	mpz_set_ui(P, spdz2_ext_processor_mersenne61::mersenne61);
	mpz_gcdext(gcd, x, y, value, P);
	mpz_mod(inverse, x, P);

	mpz_clear(gcd);
	mpz_clear(x);
	mpz_clear(y);
	mpz_clear(P);

	char szv[128], szi[128];
	syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne61::protocol_value_inverse: value = %s; inverse = %s;", mpz_get_str(szv, 10, value), mpz_get_str(szi, 10, inverse));

	return 0;
}

int spdz2_ext_processor_mersenne61::open(const size_t share_count, const mpz_t * share_values, mpz_t * opens, int verify)
{
	int result = -1;
	std::vector<ZpMersenneLongElement> m61shares(share_count), m61opens(share_count);
	syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne61::protocol_open: calling open for %u shares", (u_int32_t)share_count);
	for(size_t i = 0; i < share_count; i++)
	{
		m61shares[i].elem = mpz_get_ui(share_values[i]);
		syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne61::protocol_open() share value[%lu] = %lu", i, m61shares[i].elem);
	}

	if(the_party->openShare((int)share_count, m61shares, m61opens))
	{
		if(!verify || the_party->verify())
		{
			for(size_t i = 0; i < share_count; i++)
			{
				mpz_set_ui(opens[i], m61opens[i].elem);
				syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne61::protocol_open() opened value[%lu] = %lu", i, m61opens[i].elem);
			}
			result = 0;
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
	return result;

}

int spdz2_ext_processor_mersenne61::verify(int * error)
{
	return (the_party->verify())? 0: -1;
}

int spdz2_ext_processor_mersenne61::input(const int input_of_pid, const size_t num_of_inputs, mpz_t * inputs)
{
	int result = -1;
	std::map< int , shared_input_t >::iterator i = m_shared_inputs.find(input_of_pid);
	if(m_shared_inputs.end() != i)
	{
		if((i->second.share_count - i->second.share_index) >= num_of_inputs)
		{
			for(size_t j = 0; j < num_of_inputs; ++j)
			{
				mpz_set(inputs[j], i->second.shared_values[i->second.share_index++]);
			}
			result = 0;
		}
		else
		{
			syslog(LOG_ERR, "spdz2_ext_processor_base::exec_input_synch: not enough input for pid %d; required %lu; available %lu;",
					input_of_pid, num_of_inputs, (i->second.share_count - i->second.share_index));
		}
	}
	else
	{
		syslog(LOG_ERR, "spdz2_ext_processor_base::exec_input_synch: failed to get input for pid %d.", input_of_pid);
	}
	return result;
}

int spdz2_ext_processor_mersenne61::mult(const size_t share_count, const mpz_t * shares, mpz_t * products, int verify)
{
	int result = -1;
	size_t xy_pair_count = share_count/2;
	std::vector<ZpMersenneLongElement> x_shares(xy_pair_count), y_shares(xy_pair_count), xy_shares(xy_pair_count);

	for(size_t i = 0; i < xy_pair_count; ++i)
	{
		x_shares[i].elem = mpz_get_ui(shares[2*i]);
		y_shares[i].elem = mpz_get_ui(shares[2*i+1]);
		syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne61::protocol_mult: X-Y pair %lu: X=%lu Y=%lu", i, x_shares[i].elem, y_shares[i].elem);
	}

	if(the_party->multShares(xy_pair_count, x_shares, y_shares, xy_shares))
	{
		for(size_t i = 0; i < xy_pair_count; ++i)
		{
			mpz_set_ui(products[i], xy_shares[i].elem);
			syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne61::protocol_mult: X-Y product %lu: X*Y=%lu", i, xy_shares[i].elem);
		}
		result = 0;
	}
	else
	{
		syslog(LOG_ERR, "spdz2_ext_processor_mersenne61::protocol_mult: protocol mult failure.");
	}
	return result;
}

int spdz2_ext_processor_mersenne61::mix_add(mpz_t share, const mpz_t scalar)
{
	ZpMersenneLongElement input, output, arg;
	input.elem = mpz_get_ui(share);
	arg.elem = mpz_get_ui(scalar);
	if(Protocol<ZpMersenneLongElement>::addShareAndScalar(input, arg, output))
	{
		mpz_set_ui(share, output.elem);
		return 0;
	}
	syslog(LOG_ERR, "spdz2_ext_processor_mersenne61::mix_add: protocol addShareAndScalar failure.");
	return -1;
}

int spdz2_ext_processor_mersenne61::mix_sub_scalar(mpz_t share, const mpz_t scalar)
{
	ZpMersenneLongElement input, output, arg;
	input.elem = mpz_get_ui(share);
	arg.elem = mpz_get_ui(scalar);
	if(Protocol<ZpMersenneLongElement>::shareSubScalar(input, arg, output))
	{
		mpz_set_ui(share, output.elem);
		return 0;
	}
	syslog(LOG_ERR, "spdz2_ext_processor_mersenne61::mix_sub_scalar: protocol shareSubScalar failure.");
	return -1;
}

int spdz2_ext_processor_mersenne61::mix_sub_share(const mpz_t scalar, mpz_t share)
{
	ZpMersenneLongElement input, output, arg;
	input.elem = mpz_get_ui(share);
	arg.elem = mpz_get_ui(scalar);
	if(Protocol<ZpMersenneLongElement>::scalarSubShare(input, arg, output))
	{
		mpz_set_ui(share, output.elem);
		return 0;
	}
	syslog(LOG_ERR, "spdz2_ext_processor_mersenne61::mix_sub_share: protocol shareSubScalar failure.");
	return -1;
}
