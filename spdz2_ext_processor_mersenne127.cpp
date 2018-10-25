
#include "spdzext_width_defs.h"
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
		 	 	 	 	 	 	 	 	 const int open_count, const int mult_count, const int bits_count, int log_level)
{
	LC(m_logcat).notice("%s: init() start.", __FUNCTION__);
	if(0 == spdz2_ext_processor_base::init(pid, num_of_parties, thread_id, field, open_count, mult_count, bits_count, log_level))
	{
		the_field = new TemplateField<ZpMersenne127Element>(0);
		the_party = new Protocol<ZpMersenne127Element>(m_nparties, m_pid, open_count, mult_count, bits_count, the_field, get_parties_file());
		LC(m_logcat).notice("%s: offline() start.", __FUNCTION__);
		if(the_party->offline())
		{
			LC(m_logcat).notice("%s: load_inputs() start.", __FUNCTION__);
			if(0 == load_inputs())
			{
				LC(m_logcat).notice("%s: init() success", __FUNCTION__);
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

int spdz2_ext_processor_mersenne127::term()
{
	LC(m_logcat).notice("%s: term() start.", __FUNCTION__);
	delete the_party;
	the_party = NULL;
	delete the_field;
	the_field = NULL;
	return 0;
}

int spdz2_ext_processor_mersenne127::get_P(mpz_t P)
{
	ZpMersenne127Element::get_mpz_t_p(P);
	return 0;
}

int spdz2_ext_processor_mersenne127::offline(const int offline_size)
{
	return (the_party->offline())? 0: -1;
}

int spdz2_ext_processor_mersenne127::triple(mp_limb_t * a, mp_limb_t * b, mp_limb_t * c)
{
	std::vector<ZpMersenne127Element> triple(3 * GFP_VECTOR);
	if(the_party->triples(GFP_VECTOR, triple))
	{
		for(size_t i = 0; i < GFP_VECTOR; ++i)
		{
			triple[3*i].get_mp_limb_t(a+2*i);
			triple[3*i+1].get_mp_limb_t(b+2*i);
			triple[3*i+2].get_mp_limb_t(c+2*i);
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

int spdz2_ext_processor_mersenne127::closes(const int share_of_pid, const size_t value_count, const mp_limb_t * values, mp_limb_t * shares)
{
	char buffer[512];
	std::vector<ZpMersenne127Element> m127shares(GFP_VECTOR*value_count), m127values(GFP_VECTOR*value_count);
	if(share_of_pid == m_pid)
	{
		for(size_t i = 0; i < GFP_VECTOR*value_count; ++i)
		{
			m127values[i].set_mp_limb_t(values+ 2*i);
		}
	}

	if(the_party->makeShare(share_of_pid, m127values, m127shares))
	{
		for(size_t i = 0; i < value_count; ++i)
		{
			mp_limb_t * share = shares + (i*2*GFP_LIMBS);
			for(size_t j = 0; j < GFP_VECTOR; ++j)
			{
				m127shares[i*GFP_VECTOR+j].get_mp_limb_t(share+j*2);
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

int spdz2_ext_processor_mersenne127::bit(mp_limb_t * share)
{
	std::vector<ZpMersenne127Element> zbit_shares(GFP_VECTOR);
	if(the_party->bits(GFP_VECTOR, zbit_shares))
	{
		for(size_t i = 0; i < GFP_VECTOR; ++i)
			zbit_shares[i].get_mp_limb_t(share+2*i);
		memset(share + GFP_LIMBS, 0, GFP_BYTES);
		return 0;
	}
	else
	{
		LC(m_logcat).error("%s: protocol bits failure.", __FUNCTION__);
	}
	return -1;
}

int spdz2_ext_processor_mersenne127::open(const size_t share_count, const mp_limb_t * share_values, mp_limb_t * opens, int verify)
{
	int result = -1;
	char buffer[512];
	std::vector<ZpMersenne127Element> m127shares(GFP_VECTOR*share_count), m127opens(GFP_VECTOR*share_count);

	for(size_t i = 0; i < share_count; ++i)
	{
		const mp_limb_t * share = share_values + i*2*GFP_LIMBS;
		for(size_t j = 0; j < GFP_VECTOR; ++j)
		{
			m127shares[i*GFP_VECTOR+j].set_mp_limb_t(share+2*j);
		}
	}

	if(the_party->openShare((int)share_count, m127shares, m127opens))
	{
		if(!verify || the_party->verify())
		{
			memcpy(opens, m127opens.data(), share_count * GFP_BYTES);
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

int spdz2_ext_processor_mersenne127::mult(const size_t share_count, const mp_limb_t * xshares, const mp_limb_t * yshares, mp_limb_t * products, int verify)
{
	int result = -1;
	std::vector<ZpMersenne127Element> x_shares(GFP_VECTOR*share_count), y_shares(GFP_VECTOR*share_count), xy_shares(GFP_VECTOR*share_count);

	for(size_t i = 0; i < share_count; ++i)
	{
		const mp_limb_t * xshare = xshares + i*2*GFP_LIMBS;
		const mp_limb_t * yshare = yshares + i*2*GFP_LIMBS;
		for(size_t j = 0; j < GFP_VECTOR; ++j)
		{
			x_shares[i*GFP_VECTOR+j].set_mp_limb_t(xshare+2*j);
			y_shares[i*GFP_VECTOR+j].set_mp_limb_t(yshare+2*j);
		}
	}

	if(the_party->multShares(share_count, x_shares, y_shares, xy_shares))
	{
		for(size_t i = 0; i < share_count; ++i)
		{
			mp_limb_t * product = products + i*2*GFP_LIMBS;
			for(size_t j = 0; j < GFP_VECTOR; ++j)
			{
				xy_shares[i*GFP_VECTOR+j].get_mp_limb_t(product+2*j);
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

int spdz2_ext_processor_mersenne127::mix_add(const mp_limb_t * share, const mp_limb_t * scalar, mp_limb_t * sum)
{
	for(size_t i = 0; i < GFP_VECTOR; ++i)
	{
		ZpMersenne127Element input, output, arg;
		input.set_mp_limb_t(share+i*2);
		arg.set_mp_limb_t(scalar+i*2);
		output = input + arg;
		output.get_mp_limb_t(sum+i*2);
	}
	memset(sum + GFP_LIMBS, 0, GFP_BYTES);
	return 0;
}

int spdz2_ext_processor_mersenne127::mix_sub_scalar(const mp_limb_t * share, const mp_limb_t * scalar, mp_limb_t * diff)
{
	for(size_t i = 0; i < GFP_VECTOR; ++i)
	{
		ZpMersenne127Element input, output, arg;
		input.set_mp_limb_t(share+i*2);
		arg.set_mp_limb_t(scalar+i*2);
		output = input - arg;
		output.get_mp_limb_t(diff+i*2);
	}
	memset(diff + GFP_LIMBS, 0, GFP_BYTES);
	return 0;
}

int spdz2_ext_processor_mersenne127::mix_sub_share(const mp_limb_t * scalar, const mp_limb_t * share, mp_limb_t * diff)
{
	for(size_t i = 0; i < GFP_VECTOR; ++i)
	{
		ZpMersenne127Element input, output, arg;
		input.set_mp_limb_t(share+i*2);
		arg.set_mp_limb_t(scalar+i*2);
		output = arg - input;
		output.get_mp_limb_t(diff+i*2);
	}
	memset(diff + GFP_LIMBS, 0, GFP_BYTES);
	return 0;
}

int spdz2_ext_processor_mersenne127::mix_mul(const mp_limb_t * share, const mp_limb_t * scalar, mp_limb_t * product)
{
	for(size_t i = 0; i < GFP_VECTOR; ++i)
	{
		ZpMersenne127Element input, output, arg;
		input.set_mp_limb_t(share+i*2);
		arg.set_mp_limb_t(scalar+i*2);
		output = input * arg;
		output.get_mp_limb_t(product+i*2);
	}
	memset(product + GFP_LIMBS, 0, GFP_BYTES);
	return 0;
}

int spdz2_ext_processor_mersenne127::adds(const mp_limb_t * share1, const mp_limb_t * share2, mp_limb_t * sum)
{
	for(size_t i = 0; i < GFP_VECTOR; ++i)
	{
		ZpMersenne127Element __share1, __share2, __sum;
		__share1.set_mp_limb_t(share1+i*2);
		__share2.set_mp_limb_t(share2+i*2);
		__sum = __share1 + __share2;
		__sum.get_mp_limb_t(sum+i*2);
	}
	memset(sum + GFP_LIMBS, 0, GFP_BYTES);
	return 0;
}

int spdz2_ext_processor_mersenne127::subs(const mp_limb_t * share1, const mp_limb_t * share2, mp_limb_t * diff)
{
	for(size_t i = 0; i < GFP_VECTOR; ++i)
	{
		ZpMersenne127Element __share1, __share2, __diff;
		__share1.set_mp_limb_t(share1+i*2);
		__share2.set_mp_limb_t(share2+i*2);
		__diff = __share1 - __share2;
		__diff.get_mp_limb_t(diff+i*2);
	}
	memset(diff + GFP_LIMBS, 0, GFP_BYTES);
	return 0;
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
