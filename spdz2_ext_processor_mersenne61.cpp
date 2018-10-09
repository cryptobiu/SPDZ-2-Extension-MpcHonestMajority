
#include "spdz2_ext_processor_mersenne61.h"

#include <log4cpp/Category.hh>

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
		 	 	 	 	 	 	 	 	 const int open_count, const int mult_count, const int bits_count, int log_level)
{
	if(0 == spdz2_ext_processor_base::init(pid, num_of_parties, thread_id, field, open_count, mult_count, bits_count, log_level))
	{
		the_field = new TemplateField<ZpMersenneLongElement>(0);
		the_party = new Protocol<ZpMersenneLongElement>(m_nparties, m_pid, open_count, mult_count, bits_count, the_field, get_parties_file());
		if(the_party->offline())
		{
			if(0 == load_inputs())
			{
				LC(m_logcat).info("%s: init() success", __FUNCTION__);
				return 0;
			}
			else
				LC(m_logcat).error("%s: load_inputs() failure", __FUNCTION__);
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

int spdz2_ext_processor_mersenne61::triple(mp_limb_t * a, mp_limb_t * b, mp_limb_t * c)
{
	std::vector<ZpMersenneLongElement> triple(3);
	if(the_party->triples(1, triple))
	{
		LC(m_logcat).debug("%s: share a = %lu; share b = %lu; share c = %lu;",
				__FUNCTION__, triple[0].elem, triple[1].elem, triple[2].elem);
		*a = triple[0].elem;
		*b = triple[1].elem;
		*c = triple[2].elem;
		return 0;
	}
	else
	{
		LC(m_logcat).error("%s: protocol triples failure.", __FUNCTION__);
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
			LC(m_logcat).debug("%s: value [%lu] share[%lu] = %lu", __FUNCTION__, m61values[i].elem, i, m61shares[i].elem);
		}
		return 0;
	}
	else
	{
		LC(m_logcat).error("%s: protocol share_immediates failure.", __FUNCTION__);
	}
	return -1;
}

int spdz2_ext_processor_mersenne61::bit(mp_limb_t * share)
{
	std::vector<ZpMersenneLongElement> zbit_shares(1);
	if(the_party->bits(1, zbit_shares))
	{
		LC(m_logcat).debug("%s: protocol bits share = [%016lX].", __FUNCTION__, zbit_shares[0].elem);

		share[0] = zbit_shares[0].elem;
		share[1] = 0;

		LC(m_logcat).debug("%s: converted protocol bits share = [%016lX:%016lX].", __FUNCTION__, share[0], share[1]);
		return 0;
	}
	else
	{
		LC(m_logcat).error("%s: protocol bits failure.", __FUNCTION__);
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
	mp_limb_t u[4], __open_u[2], __r[4], __x[4];
	u[0] = u[1] = u[2] = u[3] = __open_u[0] = __open_u[1] = 0;
	__r[0] = __r[1] = __r[2] = __r[3] = __x[0] = __x[1] = __x[2] = __x[3] = 0;
	mpz_t r, open_u, v, product;

	mpz_init(r);
	mpz_init(open_u);
	mpz_init(v);
	mpz_init(product);

	if(triple(__x, __r, u))
	{
		mpz_import(r, 1, -1, 8, 0, 0, __r);
		mpz_import(x, 1, -1, 8, 0, 0, __x);
		if(open(1, u, __open_u, true))
		{
			mpz_import(open_u, 1, -1, 8, 0, 0, __open_u);
			inverse_value(open_u, v);
			if(LC(m_logcat).isDebugEnabled())
			{
				char szv[128], szi[128];
				LC(m_logcat).debug("%s: value = %s; inverse = %s;", __FUNCTION__, mpz_get_str(szv, 10, open_u), mpz_get_str(szi, 10, v));
			}
			mpz_mul(product, v, r);
			mpz_mod_ui(y, product, spdz2_ext_processor_mersenne61::mersenne61);
			result = 0;
		}
		else
			LC(m_logcat).error("%s: protocol open() failed.", __FUNCTION__);
	}
	else
		LC(m_logcat).error("%s: protocol triple() failed.", __FUNCTION__);

	mpz_clear(r);
	mpz_clear(open_u);
	mpz_clear(v);
	mpz_clear(product);

	return result;
}

int spdz2_ext_processor_mersenne61::inverse_value(const mpz_t value, mpz_t inverse)
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

	return 0;
}

int spdz2_ext_processor_mersenne61::open(const size_t share_count, const mp_limb_t * share_values, mp_limb_t * opens, int verify)
{
	int result = -1;
	std::vector<ZpMersenneLongElement> m61shares(share_count), m61opens(share_count);
	LC(m_logcat).debug("%s: calling open for %u shares", __FUNCTION__, (u_int32_t)share_count);
	for(size_t i = 0; i < share_count; i++)
	{
		m61shares[i].elem = share_values[4*i];//mpz_get_ui(share_values[i]);
		LC(m_logcat).debug("%s: share value[%lu] = %lu", __FUNCTION__, i, m61shares[i].elem);
	}

	if(the_party->openShare((int)share_count, m61shares, m61opens))
	{
		if(!verify || the_party->verify())
		{
			for(size_t i = 0; i < share_count; i++)
			{
				opens[2*i] = m61opens[i].elem;
				LC(m_logcat).debug("%s: opened value[%lu] = %lu", __FUNCTION__, i, m61opens[i].elem);
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

int spdz2_ext_processor_mersenne61::verify(int * error)
{
	return (the_party->verify())? 0: -1;
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
		LC(m_logcat).debug("%s: X-Y pair %lu: X=%lu Y=%lu", __FUNCTION__, i, x_shares[i].elem, y_shares[i].elem);
	}

	if(the_party->multShares(xy_pair_count, x_shares, y_shares, xy_shares))
	{
		for(size_t i = 0; i < xy_pair_count; ++i)
		{
			mpz_set_ui(products[i], xy_shares[i].elem);
			LC(m_logcat).debug("%s: X-Y product %lu: X*Y=%lu", __FUNCTION__, i, xy_shares[i].elem);
		}
		result = 0;
	}
	else
	{
		LC(m_logcat).error("%s: protocol mult failure.", __FUNCTION__);
	}
	return result;
}

int spdz2_ext_processor_mersenne61::mix_add(const mp_limb_t * share, const mp_limb_t * scalar, mp_limb_t * sum)
{
	ZpMersenneLongElement input, output, arg;
	input.elem = *share;
	arg.elem = *scalar;
	if(Protocol<ZpMersenneLongElement>::addShareAndScalar(input, arg, output))
	{
		*sum = output.elem;
		return 0;
	}
	LC(m_logcat).error("%s: protocol addShareAndScalar failure.", __FUNCTION__);
	return -1;
}

int spdz2_ext_processor_mersenne61::mix_sub_scalar(const mp_limb_t * share, const mp_limb_t * scalar, mp_limb_t * diff)
{
	ZpMersenneLongElement input, output, arg;
	input.elem = *share;
	arg.elem = *scalar;
	if(Protocol<ZpMersenneLongElement>::shareSubScalar(input, arg, output))
	{
		*diff = output.elem;
		return 0;
	}
	LC(m_logcat).error("%s: protocol shareSubScalar failure.", __FUNCTION__);
	return -1;
}

int spdz2_ext_processor_mersenne61::mix_sub_share(const mp_limb_t * scalar, const mp_limb_t * share, mp_limb_t * diff)
{
	ZpMersenneLongElement input, output, arg;
	input.elem = *share;
	arg.elem = *scalar;
	if(Protocol<ZpMersenneLongElement>::scalarSubShare(input, arg, output))
	{
		*diff = output.elem;
		return 0;
	}
	LC(m_logcat).error("%s: protocol scalarSubShare failure.", __FUNCTION__);
	return -1;
}

int spdz2_ext_processor_mersenne61::mix_mul(mpz_t share, const mpz_t scalar)
{
	mpz_mul(share, share, scalar);
	mpz_mod_ui(share, share, spdz2_ext_processor_mersenne61::mersenne61);
	return 0;
}

int spdz2_ext_processor_mersenne61::adds(mpz_t share1, const mpz_t share2)
{
	mpz_add(share1, share1, share2);
	mpz_mod_ui(share1, share1, spdz2_ext_processor_mersenne61::mersenne61);
	return 0;
}

int spdz2_ext_processor_mersenne61::subs(mpz_t share1, const mpz_t share2)
{
	mpz_sub(share1, share1, share2);
	mpz_mod_ui(share1, share1, spdz2_ext_processor_mersenne61::mersenne61);
	return 0;
}

std::string spdz2_ext_processor_mersenne61::get_parties_file()
{
	return "parties_gfp61.txt";
}

std::string spdz2_ext_processor_mersenne61::get_log_file()
{
	char buffer[128];
	snprintf(buffer, 128, "spdz2_x_m61_%d_%d.log", m_pid, m_thid);
	return std::string(buffer);
}

std::string spdz2_ext_processor_mersenne61::get_log_category()
{
	return "m61";
}
