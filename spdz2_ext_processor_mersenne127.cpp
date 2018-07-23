#include "spdz2_ext_processor_mersenne127.h"

#include <log4cpp/Category.hh>

spdz2_ext_processor_mersenne127::spdz2_ext_processor_mersenne127()
: the_field(NULL), the_party(NULL)
{
	gmp_randinit_mt(the_gmp_rstate);
}

spdz2_ext_processor_mersenne127::~spdz2_ext_processor_mersenne127()
{
}

int spdz2_ext_processor_mersenne127::init(const int pid, const int num_of_parties, const int thread_id, const char * field,
		 	 	 	 	 	 	 	 	 const int open_count, const int mult_count, const int bits_count)
{
	if(0 == spdz2_ext_processor_base::init(pid, num_of_parties, thread_id, field, open_count, mult_count, bits_count))
	{
		the_field = new TemplateField<Mersenne127>(0);
		the_party = new Protocol<Mersenne127>(m_nparties, m_pid, open_count, mult_count, bits_count, the_field, get_parties_file());
		if(the_party->offline())
		{
			return 0;
		}
		else
			LC(m_logcat).error("%s: protocol offline() failure.", __FUNCTION__);
		delete the_party;
		delete the_field;
	}
	else
		LC(m_logcat).error("%s: base init() failure.", __FUNCTION__);
	return -1;
}

int spdz2_ext_processor_mersenne127::term()
{
	delete the_party;
	the_party = NULL;
	delete the_field;
	the_field = NULL;
	return 0;
}

int spdz2_ext_processor_mersenne127::offline(const int offline_size)
{
	return (the_party->offline())? 0: -1;
}

int spdz2_ext_processor_mersenne127::triple(mpz_t a, mpz_t b, mpz_t c)
{
	std::vector<Mersenne127> triple(3);
	if(the_party->triples(1, triple))
	{
		mpz_set(a, *triple[0].get_mpz_t());
		mpz_set(b, *triple[1].get_mpz_t());
		mpz_set(c, *triple[2].get_mpz_t());
		return 0;
	}
	else
	{
		LC(m_logcat).error("%s: protocol triples failure.", __FUNCTION__);
	}
	return -1;
}

int spdz2_ext_processor_mersenne127::share_immediates(const int share_of_pid, const size_t value_count, const mpz_t * values, mpz_t * shares)
{
	std::vector<Mersenne127> m127shares(value_count), m127values(value_count);
	if(share_of_pid == m_pid)
	{
		for(size_t i = 0; i < value_count; ++i)
		{
			m127values[i].set_mpz_t(values + i);
		}
	}

	if(the_party->makeShare(share_of_pid, m127values, m127shares))
	{
		for(size_t i = 0; i < value_count; ++i)
		{
			mpz_set(shares[i], *m127shares[i].get_mpz_t());
		}
		return 0;
	}
	else
	{
		LC(m_logcat).error("%s: protocol share_immediates failure.", __FUNCTION__);
	}
	return -1;
}

int spdz2_ext_processor_mersenne127::bit(mpz_t share)
{
	std::vector<Mersenne127> zbit_shares(1);
	if(the_party->bits(1, zbit_shares))
	{
		mpz_set(share, *zbit_shares[0].get_mpz_t());
		return 0;
	}
	else
	{
		LC(m_logcat).error("%s: protocol bits failure.", __FUNCTION__);
	}
	return -1;
}

int spdz2_ext_processor_mersenne127::inverse(mpz_t x, mpz_t y)
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
			inverse_value(open_u, v);
			if(LC(m_logcat).isDebugEnabled())
			{
				char szv[128], szi[128];
				LC(m_logcat).debug("%s: value = %s; inverse = %s;", __FUNCTION__, mpz_get_str(szv, 10, open_u), mpz_get_str(szi, 10, v));
			}

			mpz_mul(product, v, r);

			mpz_t M127;
			mpz_init(M127);
			mpz_set_str(M127, Mersenne127::M127, 10);
			mpz_mod(y, product, M127);
			mpz_clear(M127);
			result = 0;
		}
		else
			LC(m_logcat).error("%s: protocol open() failed", __FUNCTION__);
	}
	else
		LC(m_logcat).error("%s: protocol triple() failed", __FUNCTION__);

	mpz_clear(r);
	mpz_clear(u);
	mpz_clear(open_u);
	mpz_clear(v);
	mpz_clear(product);

	return result;
}

int spdz2_ext_processor_mersenne127::inverse_value(const mpz_t value, mpz_t inverse)
{
	mpz_t gcd, x, y, P;

	mpz_init(gcd);
	mpz_init(x);
	mpz_init(y);
	mpz_init(P);

	mpz_set_str(P, Mersenne127::M127, 10);
	mpz_gcdext(gcd, x, y, value, P);
	mpz_mod(inverse, x, P);

	mpz_clear(gcd);
	mpz_clear(x);
	mpz_clear(y);
	mpz_clear(P);

	return 0;
}

int spdz2_ext_processor_mersenne127::open(const size_t share_count, const mpz_t * share_values, mpz_t * opens, int verify)
{
	int result = -1;
	std::vector<Mersenne127> m127shares(share_count), m127opens(share_count);
	LC(m_logcat).debug("%s: calling open for %u shares", __FUNCTION__, (u_int32_t)share_count);
	for(size_t i = 0; i < share_count; i++)
	{
		m127shares[i].set_mpz_t(share_values + i);
	}

	if(the_party->openShare((int)share_count, m127shares, m127opens))
	{
		if(!verify || the_party->verify())
		{
			for(size_t i = 0; i < share_count; i++)
			{
				mpz_set(opens[i], *m127opens[i].get_mpz_t());
			}
			result = 0;
		}
		else
		{
			LC(m_logcat).error("%s: verify failure.", __FUNCTION__);
		}
	}
	else
	{
		LC(m_logcat).error("%s: openShare failure.", __FUNCTION__);
	}
	return result;

}

int spdz2_ext_processor_mersenne127::verify(int * error)
{
	return (the_party->verify())? 0: -1;
}

int spdz2_ext_processor_mersenne127::mult(const size_t share_count, const mpz_t * shares, mpz_t * products, int verify)
{
	int result = -1;
	size_t xy_pair_count = share_count/2;
	std::vector<Mersenne127> x_shares(xy_pair_count), y_shares(xy_pair_count), xy_shares(xy_pair_count);

	for(size_t i = 0; i < xy_pair_count; ++i)
	{
		x_shares[i].set_mpz_t(shares + 2*i);
		y_shares[i].set_mpz_t(shares + 2*i + 1);
	}

	if(the_party->multShares(xy_pair_count, x_shares, y_shares, xy_shares))
	{
		for(size_t i = 0; i < xy_pair_count; ++i)
		{
			mpz_set(products[i], *xy_shares[i].get_mpz_t());
		}
		result = 0;
	}
	else
	{
		LC(m_logcat).error("%s: protocol mult failure.", __FUNCTION__);
	}
	return result;
}

int spdz2_ext_processor_mersenne127::mix_add(mpz_t share, const mpz_t scalar)
{
	Mersenne127 input(share), output, arg(scalar);
	if(Protocol<Mersenne127>::addShareAndScalar(input, arg, output))
	{
		mpz_set(share, *output.get_mpz_t());
		return 0;
	}
	LC(m_logcat).error("%s: protocol addShareAndScalar failure.", __FUNCTION__);
	return -1;
}

int spdz2_ext_processor_mersenne127::mix_sub_scalar(mpz_t share, const mpz_t scalar)
{
	Mersenne127 input(share), output, arg(scalar);
	if(Protocol<Mersenne127>::shareSubScalar(input, arg, output))
	{
		mpz_set(share, *output.get_mpz_t());
		return 0;
	}
	LC(m_logcat).error("%s: protocol shareSubScalar failure.", __FUNCTION__);
	return -1;
}

int spdz2_ext_processor_mersenne127::mix_sub_share(const mpz_t scalar, mpz_t share)
{
	Mersenne127 input(share), output, arg(scalar);
	if(Protocol<Mersenne127>::scalarSubShare(input, arg, output))
	{
		mpz_set(share, *output.get_mpz_t());
		return 0;
	}
	LC(m_logcat).error("%s: protocol scalarSubShare failure.", __FUNCTION__);
	return -1;
}

std::string spdz2_ext_processor_mersenne127::get_parties_file()
{
	return "parties_gfp127.txt";
}

std::string spdz2_ext_processor_mersenne127::get_log_file()
{
	char buffer[128];
	snprintf(buffer, 128, "spdz2_x_m127_%d_%d.log", m_pid, m_thid);
	return std::string(buffer);
}

std::string spdz2_ext_processor_mersenne127::get_log_category()
{
	return "m127";
}
