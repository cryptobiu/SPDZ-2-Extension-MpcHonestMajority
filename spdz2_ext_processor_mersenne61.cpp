
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
	std::vector<ZpMersenneLongElement> triple(3 * GFP_VALS);
	if(the_party->triples(GFP_VALS, triple))
	{
		for(size_t i = 0; i < GFP_VALS; ++i)
		{
			a[i*VAL_LIMBS] = triple[3*i].elem;
			memset(a + i*VAL_LIMBS + 1, 0, VAL_BYTES - sizeof(mp_limb_t));

			b[i*VAL_LIMBS] = triple[3*i+1].elem;
			memset(b + i*VAL_LIMBS + 1, 0, VAL_BYTES - sizeof(mp_limb_t));

			c[i*VAL_LIMBS] = triple[3*i+2].elem;
			memset(c + i*VAL_LIMBS + 1, 0, VAL_BYTES - sizeof(mp_limb_t));

			LC(m_logcat + ".acct").debug("%s: (%lu) a=%lu; b=%lu; c=%lu;", __FUNCTION__, i, a[i*VAL_LIMBS], b[i*VAL_LIMBS], c[i*VAL_LIMBS]);
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
	std::vector<ZpMersenneLongElement> m61shares(GFP_VALS*value_count), m61values(GFP_VALS*value_count);

	if(share_of_pid == m_pid)
	{
		for(size_t i = 0; i < value_count; ++i)
		{
			const mp_limb_t * value = values + i*GFP_LIMBS;
			for(size_t j = 0; j < GFP_VALS; ++j)
			{
				m61values[i*GFP_VALS+j].elem = value[j*VAL_LIMBS];
			}
		}
	}

	if(the_party->makeShare(share_of_pid, m61values, m61shares))
	{
		for(size_t i = 0; i < value_count; ++i)
		{
			mp_limb_t * share = shares + (i*SHR_LIMBS);
			for(size_t j = 0; j < GFP_VALS; ++j)
			{
				share[j*VAL_LIMBS] = m61shares[i*GFP_VALS+j].elem;
				memset(share + j*VAL_LIMBS + 1, 0, VAL_BYTES - sizeof(mp_limb_t));
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
	std::vector<ZpMersenneLongElement> zbit_shares(GFP_VALS);
	if(the_party->bits(GFP_VALS, zbit_shares))
	{
		for(size_t i = 0; i < GFP_VALS; ++i)
		{
			share[i*VAL_LIMBS] = zbit_shares[i].elem;
			memset(share + i*VAL_LIMBS + 1, 0, VAL_BYTES - sizeof(mp_limb_t));
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
	std::vector<ZpMersenneLongElement> m61shares(GFP_VALS*share_count), m61opens(GFP_VALS*share_count);

	for(size_t i = 0; i < share_count; ++i)
	{
		const mp_limb_t * share = share_values + i*SHR_LIMBS;
		for(size_t j = 0; j < GFP_VALS; ++j)
		{
			m61shares[i*GFP_VALS+j].elem = share[j*VAL_LIMBS];
		}
	}

	if(the_party->openShare((int)GFP_VALS*share_count, m61shares, m61opens))
	{
		if(!verify || the_party->verify())
		{
			for(size_t i = 0; i < share_count; ++i)
			{
				mp_limb_t * open = opens + i*GFP_LIMBS;
				for(size_t j = 0; j < GFP_VALS; ++j)
				{
					open[j*VAL_LIMBS] = m61opens[i*GFP_VALS+j].elem;
					memset(open + j*VAL_LIMBS + 1, 0, VAL_BYTES - sizeof(mp_limb_t));
				}
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
	std::vector<ZpMersenneLongElement> x_shares(GFP_VALS*share_count), y_shares(GFP_VALS*share_count), xy_shares(GFP_VALS*share_count);

	for(size_t i = 0; i < share_count; ++i)
	{
		const mp_limb_t * xshare = xshares + i*SHR_LIMBS;
		const mp_limb_t * yshare = yshares + i*SHR_LIMBS;
		for(size_t j = 0; j < GFP_VALS; ++j)
		{
			x_shares[i*GFP_VALS+j].elem = xshare[j*VAL_LIMBS];
			y_shares[i*GFP_VALS+j].elem = yshare[j*VAL_LIMBS];
		}
	}

	if(the_party->multShares(GFP_VALS*share_count, x_shares, y_shares, xy_shares))
	{
		for(size_t i = 0; i < share_count; ++i)
		{
			mp_limb_t * product = products + i*SHR_LIMBS;
			for(size_t j = 0; j < GFP_VALS; ++j)
			{
				product[j*VAL_LIMBS] = xy_shares[i*GFP_VALS+j].elem;
				memset(product + j*VAL_LIMBS + 1, 0, VAL_BYTES - sizeof(mp_limb_t));
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
	for(size_t i = 0; i < GFP_VALS; ++i)
	{
		ZpMersenneLongElement input, output, arg;
		input.elem = share[i*VAL_LIMBS];
		arg.elem = scalar[i*VAL_LIMBS];
		output = input + arg;
		sum[i*VAL_LIMBS] = output.elem;
		memset(sum + i*VAL_LIMBS + 1, 0, VAL_BYTES - sizeof(mp_limb_t));
		LC(m_logcat + ".acct").debug("%s: sh=%lu; sc=%lu; su=%lu;", __FUNCTION__, share[i*2], scalar[i*2], sum[i*2]);
	}
	memset(sum + GFP_LIMBS, 0, GFP_BYTES);
	return 0;
}

int spdz2_ext_processor_mersenne61::mix_sub_scalar(const mp_limb_t * share, const mp_limb_t * scalar, mp_limb_t * diff)
{
	for(size_t i = 0; i < GFP_VALS; ++i)
	{
		ZpMersenneLongElement input, output, arg;
		input.elem = share[i*VAL_LIMBS];
		arg.elem = scalar[i*VAL_LIMBS];
		output = input - arg;
		diff[i*VAL_LIMBS] = output.elem;
		memset(diff + i*VAL_LIMBS + 1, 0, VAL_BYTES - sizeof(mp_limb_t));
		LC(m_logcat + ".acct").debug("%s: sh=%lu; sc=%lu; df=%lu;", __FUNCTION__, share[i*2], scalar[i*2], diff[i*2]);
	}
	memset(diff + GFP_LIMBS, 0, GFP_BYTES);
	return 0;
}

int spdz2_ext_processor_mersenne61::mix_sub_share(const mp_limb_t * scalar, const mp_limb_t * share, mp_limb_t * diff)
{
	for(size_t i = 0; i < GFP_VALS; ++i)
	{
		ZpMersenneLongElement input, output, arg;
		input.elem = share[i*VAL_LIMBS];
		arg.elem = scalar[i*VAL_LIMBS];
		output = arg - input;
		diff[i*VAL_LIMBS] = output.elem;
		memset(diff + i*VAL_LIMBS + 1, 0, VAL_BYTES - sizeof(mp_limb_t));
		LC(m_logcat + ".acct").debug("%s: sc=%lu; sh=%lu; df=%lu;", __FUNCTION__, share[i*2], scalar[i*2], diff[i*2]);
	}
	memset(diff + GFP_LIMBS, 0, GFP_BYTES);
	return 0;
}

int spdz2_ext_processor_mersenne61::mix_mul(const mp_limb_t * share, const mp_limb_t * scalar, mp_limb_t * product)
{
	for(size_t i = 0; i < GFP_VALS; ++i)
	{
		ZpMersenneLongElement input, output, arg;
		input.elem = share[i*VAL_LIMBS];
		arg.elem = scalar[i*VAL_LIMBS];
		output = input * arg;
		product[i*VAL_LIMBS] = output.elem;
		memset(product + i*VAL_LIMBS + 1, 0, VAL_BYTES - sizeof(mp_limb_t));
		LC(m_logcat + ".acct").debug("%s: sh=%lu; sc=%lu; pd=%lu;", __FUNCTION__, share[i*2], scalar[i*2], product[i*2]);
	}
	memset(product + GFP_LIMBS, 0, GFP_BYTES);
	return 0;
}

int spdz2_ext_processor_mersenne61::adds(const mp_limb_t * share1, const mp_limb_t * share2, mp_limb_t * sum)
{
	for(size_t i = 0; i < GFP_VALS; ++i)
	{
		ZpMersenneLongElement __share1, __share2, __sum;
		__share1.elem = share1[i*VAL_LIMBS];
		__share2.elem = share2[i*VAL_LIMBS];
		__sum = __share1 + __share2;
		sum[i*VAL_LIMBS] = __sum.elem;
		memset(sum + i*VAL_LIMBS + 1, 0, VAL_BYTES - sizeof(mp_limb_t));
		LC(m_logcat + ".acct").debug("%s: sh1=%lu; sh2=%lu; sum=%lu;", __FUNCTION__, share1[i*2], share2[i*2], sum[i*2]);
	}
	memset(sum + GFP_LIMBS, 0, GFP_BYTES);
	return 0;
}

int spdz2_ext_processor_mersenne61::subs(const mp_limb_t * share1, const mp_limb_t * share2, mp_limb_t * diff)
{
	for(size_t i = 0; i < GFP_VALS; ++i)
	{
		ZpMersenneLongElement __share1, __share2, __diff;
		__share1.elem = share1[i*VAL_LIMBS];
		__share2.elem = share2[i*VAL_LIMBS];
		__diff = __share1 - __share2;
		diff[i*VAL_LIMBS] = __diff.elem;
		memset(diff + i*VAL_LIMBS + 1, 0, VAL_BYTES - sizeof(mp_limb_t));
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
