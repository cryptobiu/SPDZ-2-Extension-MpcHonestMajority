
#include "spdzext_width_defs.h"
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
	mpz_set_ui(P, spdz2_ext_processor_mersenne61::mersenne61);
	return 0;
}

int spdz2_ext_processor_mersenne61::offline(const int offline_size)
{
	return (the_party->offline())? 0: -1;
}

int spdz2_ext_processor_mersenne61::triple(mp_limb_t * a, mp_limb_t * b, mp_limb_t * c)
{
	std::vector<ZpMersenneLongElement> triple(3 * GFP_VECTOR);
	if(the_party->triples(GFP_VECTOR, triple))
	{
		for(size_t i = 0; i < GFP_VECTOR; ++i)
		{
			a[i*2] = triple[3*i].elem;
			a[i*2+1] = 0;
			b[i*2] = triple[3*i+1].elem;
			b[i*2+1] = 0;
			c[i*2] = triple[3*i+2].elem;
			c[i*2+1] = 0;
			LC(m_logcat + ".acct").debug("%s: (%lu) a=%lu; b=%lu; c=%lu;", __FUNCTION__, i, a[i*2], b[i*2], c[i*2]);
		}
		memset(a + GFP_LIMBS, 0, GFP_BYTES);//memset-0 the 2nd GFP @a
		memset(b + GFP_LIMBS, 0, GFP_BYTES);//memset-0 the 2nd GFP @b
		memset(c + GFP_LIMBS, 0, GFP_BYTES);//memset-0 the 2nd GFP @c
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
	std::vector<ZpMersenneLongElement> m61shares(GFP_VECTOR*value_count), m61values(GFP_VECTOR*value_count);
	if(share_of_pid == m_pid)
	{
		for(size_t i = 0; i < GFP_VECTOR*value_count; ++i)
		{
			m61values[i].elem = values[2*i];
		}
	}

	if(the_party->makeShare(share_of_pid, m61values, m61shares))
	{
		for(size_t i = 0; i < value_count; ++i)
		{
			mp_limb_t * share = shares + (i*2*GFP_LIMBS);
			for(size_t j = 0; j < GFP_VECTOR; ++j)
			{
				share[j*2] = m61shares[i*GFP_VECTOR+j].elem;
				share[j*2+1] = 0;
			}
			memset(share + GFP_LIMBS, 0, GFP_BYTES);
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
	std::vector<ZpMersenneLongElement> zbit_shares(GFP_VECTOR);
	if(the_party->bits(GFP_VECTOR, zbit_shares))
	{
		for(size_t i = 0; i < GFP_VECTOR; ++i)
		{
			share[2*i] = zbit_shares[i].elem;
			share[2*i+1] = 0;
		}
		memset(share + GFP_LIMBS, 0, GFP_BYTES);
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
	int result = -1;
	std::vector<ZpMersenneLongElement> m61shares(GFP_VECTOR*share_count), m61opens(GFP_VECTOR*share_count);

	for(size_t i = 0; i < share_count; ++i)
	{
		const mp_limb_t * share = share_values + i*2*GFP_LIMBS;

		for(size_t j = 0; j < GFP_VECTOR; ++j)
		{
			m61shares[i*GFP_VECTOR+j].elem = share[2*j];
		}
	}

	if(the_party->openShare((int)GFP_VECTOR*share_count, m61shares, m61opens))
	{
		if(!verify || the_party->verify())
		{
			for(size_t i = 0; i < GFP_VECTOR*share_count; ++i)
			{
				opens[2*i] = m61opens[i].elem;
				opens[2*i+1] = 0;
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

int spdz2_ext_processor_mersenne61::mult(const size_t share_count, const mp_limb_t * xshares, const mp_limb_t * yshares, mp_limb_t * products, int verify)
{
	LC(m_logcat).info("%s called for %lu shares.", __FUNCTION__, share_count);
	int result = -1;
	std::vector<ZpMersenneLongElement> x_shares(GFP_VECTOR*share_count), y_shares(GFP_VECTOR*share_count), xy_shares(GFP_VECTOR*share_count);

	for(size_t i = 0; i < share_count; ++i)
	{
		const mp_limb_t * xshare = xshares + i*2*GFP_LIMBS;
		const mp_limb_t * yshare = yshares + i*2*GFP_LIMBS;
		for(size_t j = 0; j < GFP_VECTOR; ++j)
		{
			x_shares[i*GFP_VECTOR+j].elem = xshare[2*j];
			y_shares[i*GFP_VECTOR+j].elem = yshare[2*j];
		}
	}

	if(the_party->multShares(GFP_VECTOR*share_count, x_shares, y_shares, xy_shares))
	{
		for(size_t i = 0; i < share_count; ++i)
		{
			mp_limb_t * product = products + i*2*GFP_LIMBS;
			for(size_t j = 0; j < GFP_VECTOR; ++j)
			{
				product[2*j] = xy_shares[i*GFP_VECTOR+j].elem;
				product[2*j+1] = 0;
			}
			memset(product + GFP_LIMBS, 0, GFP_BYTES);
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
	for(size_t i = 0; i < GFP_VECTOR; ++i)
	{
		ZpMersenneLongElement input, output, arg;
		input.elem = share[i*2];
		arg.elem = scalar[i*2];
		output = input + arg;
		sum[i*2] = output.elem;
		sum[i*2+1] = 0;
		LC(m_logcat + ".acct").debug("%s: sh=%lu; sc=%lu; su=%lu;", __FUNCTION__, share[i*2], scalar[i*2], sum[i*2]);
	}
	memset(sum + GFP_LIMBS, 0, GFP_BYTES);
	return 0;
}

int spdz2_ext_processor_mersenne61::mix_sub_scalar(const mp_limb_t * share, const mp_limb_t * scalar, mp_limb_t * diff)
{
	for(size_t i = 0; i < GFP_VECTOR; ++i)
	{
		ZpMersenneLongElement input, output, arg;
		input.elem = share[i*2];
		arg.elem = scalar[i*2];
		output = input - arg;
		diff[i*2] = output.elem;
		diff[i*2+1] = 0;
		LC(m_logcat + ".acct").debug("%s: sh=%lu; sc=%lu; df=%lu;", __FUNCTION__, share[i*2], scalar[i*2], diff[i*2]);
	}
	memset(diff + GFP_LIMBS, 0, GFP_BYTES);
	return 0;
}

int spdz2_ext_processor_mersenne61::mix_sub_share(const mp_limb_t * scalar, const mp_limb_t * share, mp_limb_t * diff)
{
	for(size_t i = 0; i < GFP_VECTOR; ++i)
	{
		ZpMersenneLongElement input, output, arg;
		input.elem = share[i*2];
		arg.elem = scalar[i*2];
		output = arg - input;
		diff[i*2] = output.elem;
		diff[i*2+1] = 0;
		LC(m_logcat + ".acct").debug("%s: sc=%lu; sh=%lu; df=%lu;", __FUNCTION__, share[i*2], scalar[i*2], diff[i*2]);
	}
	memset(diff + GFP_LIMBS, 0, GFP_BYTES);
	return 0;
}

int spdz2_ext_processor_mersenne61::mix_mul(const mp_limb_t * share, const mp_limb_t * scalar, mp_limb_t * product)
{
	for(size_t i = 0; i < GFP_VECTOR; ++i)
	{
		ZpMersenneLongElement input, output, arg;
		input.elem = share[i*2];
		arg.elem = scalar[i*2];
		output = input * arg;
		product[i*2] = output.elem;
		product[i*2+1] = 0;
		LC(m_logcat + ".acct").debug("%s: sh=%lu; sc=%lu; pd=%lu;", __FUNCTION__, share[i*2], scalar[i*2], product[i*2]);
	}
	memset(product + GFP_LIMBS, 0, GFP_BYTES);
	return 0;
}

int spdz2_ext_processor_mersenne61::adds(const mp_limb_t * share1, const mp_limb_t * share2, mp_limb_t * sum)
{
	for(size_t i = 0; i < GFP_VECTOR; ++i)
	{
		ZpMersenneLongElement __share1, __share2, __sum;
		__share1.elem = share1[i*2];
		__share2.elem = share2[i*2];
		__sum = __share1 + __share2;
		sum[i*2] = __sum.elem;
		sum[i*2+1] = 0;
		LC(m_logcat + ".acct").debug("%s: sh1=%lu; sh2=%lu; sum=%lu;", __FUNCTION__, share1[i*2], share2[i*2], sum[i*2]);
	}
	memset(sum + GFP_LIMBS, 0, GFP_BYTES);
	return 0;
}

int spdz2_ext_processor_mersenne61::subs(const mp_limb_t * share1, const mp_limb_t * share2, mp_limb_t * diff)
{
	for(size_t i = 0; i < GFP_VECTOR; ++i)
	{
		ZpMersenneLongElement __share1, __share2, __diff;
		__share1.elem = share1[i*2];
		__share2.elem = share2[i*2];
		__diff = __share1 - __share2;
		diff[i*2] = __diff.elem;
		diff[i*2+1] = 0;
		LC(m_logcat + ".acct").debug("%s: sh1=%lu; sh2=%lu; dif=%lu;", __FUNCTION__, share1[i*2], share2[i*2], diff[i*2]);
	}
	memset(diff + GFP_LIMBS, 0, GFP_BYTES);
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
