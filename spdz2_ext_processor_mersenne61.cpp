
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
	LC(m_logcat).info("%s.", __FUNCTION__);
	return 0;
}

int spdz2_ext_processor_mersenne61::get_P(mpz_t P)
{
	LC(m_logcat).info("%s.", __FUNCTION__);
	mpz_set_ui(P, spdz2_ext_processor_mersenne61::mersenne61);
	return 0;
}

int spdz2_ext_processor_mersenne61::offline(const int offline_size)
{
	LC(m_logcat).info("%s.", __FUNCTION__);
	return (the_party->offline())? 0: -1;
}

int spdz2_ext_processor_mersenne61::triple(mp_limb_t * a, mp_limb_t * b, mp_limb_t * c)
{
	LC(m_logcat).info("%s.", __FUNCTION__);
	std::vector<ZpMersenneLongElement> triple(3);
	if(the_party->triples(1, triple))
	{
		LC(m_logcat).debug("%s: share a = %lu; share b = %lu; share c = %lu;",
				__FUNCTION__, triple[0].elem, triple[1].elem, triple[2].elem);
		*a = triple[0].elem;
		memset(a + 1, 0, 3 * sizeof(mp_limb_t));
		*b = triple[1].elem;
		memset(b + 1, 0, 3 * sizeof(mp_limb_t));
		*c = triple[2].elem;
		memset(c + 1, 0, 3 * sizeof(mp_limb_t));
		return 0;
	}
	else
	{
		LC(m_logcat).error("%s: protocol triples failure.", __FUNCTION__);
	}
	return -1;
}

int spdz2_ext_processor_mersenne61::closes(const int share_of_pid, const size_t value_count, const mp_limb_t * values, mp_limb_t * shares)
{
	LC(m_logcat).info("%s.", __FUNCTION__);
	std::vector<ZpMersenneLongElement> m61shares(value_count), m61values(value_count);
	if(share_of_pid == m_pid)
	{
		for(size_t i = 0; i < value_count; ++i)
		{
			m61values[i].elem = values[2*i];
		}
	}

	if(the_party->makeShare(share_of_pid, m61values, m61shares))
	{
		for(size_t i = 0; i < value_count; ++i)
		{
			shares[4*i] = m61shares[i].elem;
			memset(shares + 4*i + 1, 0, 3 * sizeof(mp_limb_t));
			LC(m_logcat).debug("%s: value [%lu] share[%lu] = %lu", __FUNCTION__, m61values[i].elem, i, m61shares[i].elem);
		}
		return 0;
	}
	else
	{
		LC(m_logcat).error("%s: protocol makeShare failure.", __FUNCTION__);
	}
	return -1;
}

int spdz2_ext_processor_mersenne61::bit(mp_limb_t * share)
{
	LC(m_logcat).info("%s.", __FUNCTION__);
	std::vector<ZpMersenneLongElement> zbit_shares(1);
	if(the_party->bits(1, zbit_shares))
	{
		LC(m_logcat).debug("%s: protocol bits share = %lu.", __FUNCTION__, zbit_shares[0].elem);
		*share = zbit_shares[0].elem;
		memset(share + 1, 0, 3 * sizeof(mp_limb_t));
		return 0;
	}
	else
	{
		LC(m_logcat).error("%s: protocol bits failure.", __FUNCTION__);
	}
	return -1;
}

int spdz2_ext_processor_mersenne61::open(const size_t share_count, const mp_limb_t * share_values, mp_limb_t * opens, int verify)
{
	LC(m_logcat).info("%s.", __FUNCTION__);
	int result = -1;
	std::vector<ZpMersenneLongElement> m61shares(share_count), m61opens(share_count);
	LC(m_logcat).debug("%s: calling open for %u shares", __FUNCTION__, (u_int32_t)share_count);
	for(size_t i = 0; i < share_count; i++)
	{
		m61shares[i].elem = share_values[4*i];
		LC(m_logcat).debug("%s: share value[%lu] = %lu", __FUNCTION__, i, m61shares[i].elem);
	}

	if(the_party->openShare((int)share_count, m61shares, m61opens))
	{
		if(!verify || the_party->verify())
		{
			for(size_t i = 0; i < share_count; i++)
			{
				opens[2*i] = m61opens[i].elem;
				opens[2*i + 1] = 0;
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
	LC(m_logcat).info("%s.", __FUNCTION__);
	return (the_party->verify())? 0: -1;
}

int spdz2_ext_processor_mersenne61::mult(const size_t share_count, const mp_limb_t * shares, mp_limb_t * products, int verify)
{
	LC(m_logcat).info("%s called for %lu shares.", __FUNCTION__, share_count);
	int result = -1;
	size_t xy_pair_count = share_count/2;
	std::vector<ZpMersenneLongElement> x_shares(xy_pair_count), y_shares(xy_pair_count), xy_shares(xy_pair_count);

	for(size_t i = 0; i < xy_pair_count; ++i)
	{
		x_shares[i].elem = *(shares + 4*(2*i));
		y_shares[i].elem = *(shares + 4*(2*i+1));
		//x_shares[i].elem = mpz_get_ui(shares[2*i]);
		//y_shares[i].elem = mpz_get_ui(shares[2*i+1]);
		LC(m_logcat).debug("%s: X-Y pair %lu: X=%lu Y=%lu", __FUNCTION__, i, x_shares[i].elem, y_shares[i].elem);
	}

	if(the_party->multShares(xy_pair_count, x_shares, y_shares, xy_shares))
	{
		for(size_t i = 0; i < xy_pair_count; ++i)
		{
			products[4*i] = xy_shares[i].elem;
			memset(products + 4*i + 1, 0, 3 * sizeof(mp_limb_t));
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
	LC(m_logcat).info("%s.", __FUNCTION__);
	ZpMersenneLongElement input, output, arg;
	input.elem = *share;
	arg.elem = *scalar;
	output = input + arg;
	*sum = output.elem;
	memset(sum + 1, 0, 3 * sizeof(mp_limb_t));
	return 0;
}

int spdz2_ext_processor_mersenne61::mix_sub_scalar(const mp_limb_t * share, const mp_limb_t * scalar, mp_limb_t * diff)
{
	LC(m_logcat).info("%s.", __FUNCTION__);
	ZpMersenneLongElement input, output, arg;
	input.elem = *share;
	arg.elem = *scalar;
	output = input - arg;
	*diff = output.elem;
	memset(diff + 1, 0, 3 * sizeof(mp_limb_t));
	return 0;
}

int spdz2_ext_processor_mersenne61::mix_sub_share(const mp_limb_t * scalar, const mp_limb_t * share, mp_limb_t * diff)
{
	LC(m_logcat).info("%s.", __FUNCTION__);
	ZpMersenneLongElement input, output, arg;
	input.elem = *share;
	arg.elem = *scalar;
	output = arg - input;
	*diff = output.elem;
	memset(diff + 1, 0, 3 * sizeof(mp_limb_t));
	return 0;
}

int spdz2_ext_processor_mersenne61::mix_mul(const mp_limb_t * share, const mp_limb_t * scalar, mp_limb_t * product)
{
	LC(m_logcat).info("%s.", __FUNCTION__);
	ZpMersenneLongElement input, output, arg;
	input.elem = *share;
	arg.elem = *scalar;
	output = input * arg;
	*product = output.elem;
	memset(product + 1, 0, 3 * sizeof(mp_limb_t));
	return 0;
}

int spdz2_ext_processor_mersenne61::adds(const mp_limb_t * share1, const mp_limb_t * share2, mp_limb_t * sum)
{
	LC(m_logcat).info("%s.", __FUNCTION__);
	ZpMersenneLongElement __share1, __share2, __sum;
	__share1.elem = *share1;
	__share2.elem = *share2;
	__sum = __share1 + __share2;
	*sum = __sum.elem;
	memset(sum + 1, 0, 3 * sizeof(mp_limb_t));
	return 0;
}

int spdz2_ext_processor_mersenne61::subs(const mp_limb_t * share1, const mp_limb_t * share2, mp_limb_t * diff)
{
	LC(m_logcat).info("%s.", __FUNCTION__);
	ZpMersenneLongElement __share1, __share2, __diff;
	__share1.elem = *share1;
	__share2.elem = *share2;
	__diff = __share1 - __share2;
	*diff = __diff.elem;
	memset(diff + 1, 0, 3 * sizeof(mp_limb_t));
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
