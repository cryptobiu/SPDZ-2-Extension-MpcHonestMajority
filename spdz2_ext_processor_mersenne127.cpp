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
	std::vector<ZpMersenne127Element> triple(3);
	if(the_party->triples(1, triple))
	{
		memcpy(a, (mp_limb_t *)triple[0], 2 * sizeof(mp_limb_t));
		memcpy(b, (mp_limb_t *)triple[1], 2 * sizeof(mp_limb_t));
		memcpy(c, (mp_limb_t *)triple[2], 2 * sizeof(mp_limb_t));
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
	std::vector<ZpMersenne127Element> m127shares(value_count), m127values(value_count);
	if(share_of_pid == m_pid)
	{
		for(size_t i = 0; i < value_count; ++i)
		{
			memcpy((mp_limb_t*)m127values[i], values + 2*i, 2*sizeof(mp_limb_t));
		}
	}

	if(the_party->makeShare(share_of_pid, m127values, m127shares))
	{
		for(size_t i = 0; i < value_count; ++i)
		{
			memcpy(shares + 4*i, (mp_limb_t*)m127shares[i], 2*sizeof(mp_limb_t));
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
	std::vector<ZpMersenne127Element> zbit_shares(1);
	if(the_party->bits(1, zbit_shares))
	{
		memcpy(share, &zbit_shares[0], 16);
		LC(m_logcat).debug("%s: bit share = [%llu:%llu].", __FUNCTION__, share[0], share[1]);
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
	std::vector<ZpMersenne127Element> m127shares(share_count), m127opens(share_count);
	LC(m_logcat).debug("%s: calling open for %u shares", __FUNCTION__, (u_int32_t)share_count);
	for(size_t i = 0; i < share_count; i++)
	{
		memcpy((mp_limb_t *)m127shares[i], share_values + 4*i, 2 * sizeof(mp_limb_t));
	}

	if(the_party->openShare((int)share_count, m127shares, m127opens))
	{
		if(!verify || the_party->verify())
		{
			memcpy(opens, m127opens.data(), share_count * 2 * sizeof(mp_limb_t));
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

int spdz2_ext_processor_mersenne127::mult(const size_t share_count, const mp_limb_t * shares, mp_limb_t * products, int verify)
{
	int result = -1;
	size_t xy_pair_count = share_count/2;
	std::vector<ZpMersenne127Element> x_shares(xy_pair_count), y_shares(xy_pair_count), xy_shares(xy_pair_count);

	for(size_t i = 0; i < xy_pair_count; ++i)
	{
		memcpy((mp_limb_t*)x_shares[i], shares + 4*(2*i), 2*sizeof(mp_limb_t));
		memcpy((mp_limb_t*)y_shares[i], shares + 4*(2*i+1), 2*sizeof(mp_limb_t));
		//x_shares[i].set_mpz_t(shares[2*i]);
		//y_shares[i].set_mpz_t(shares[2*i + 1]);
	}

	if(the_party->multShares(xy_pair_count, x_shares, y_shares, xy_shares))
	{
		for(size_t i = 0; i < xy_pair_count; ++i)
		{
			memcpy(products + 4*i, (mp_limb_t*)xy_shares[i], 2*sizeof(mp_limb_t));
			//xy_shares[i].get_mpz_t(products[i]);
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
	ZpMersenne127Element input, output, arg;
	memcpy((mp_limb_t*)input, share, 2 * sizeof(mp_limb_t));
	memcpy((mp_limb_t*)arg, scalar, 2 * sizeof(mp_limb_t));
	output = input + arg;
	memcpy(sum, (mp_limb_t*)output, 2 * sizeof(mp_limb_t));
	return 0;
}

int spdz2_ext_processor_mersenne127::mix_sub_scalar(const mp_limb_t * share, const mp_limb_t * scalar, mp_limb_t * diff)
{
	ZpMersenne127Element input, output, arg;
	memcpy((mp_limb_t*)input, share, 2 * sizeof(mp_limb_t));
	memcpy((mp_limb_t*)arg, scalar, 2 * sizeof(mp_limb_t));
	output = input - arg;
	memcpy(diff, (mp_limb_t*)output, 2 * sizeof(mp_limb_t));
	return 0;
}

int spdz2_ext_processor_mersenne127::mix_sub_share(const mp_limb_t * scalar, const mp_limb_t * share, mp_limb_t * diff)
{
	ZpMersenne127Element input, output, arg;
	memcpy((mp_limb_t*)input, share, 2 * sizeof(mp_limb_t));
	memcpy((mp_limb_t*)arg, scalar, 2 * sizeof(mp_limb_t));
	output = arg - input;
	memcpy(diff, (mp_limb_t*)output, 2 * sizeof(mp_limb_t));
	return 0;
}

int spdz2_ext_processor_mersenne127::mix_mul(const mp_limb_t * share, const mp_limb_t * scalar, mp_limb_t * product)
{
	ZpMersenne127Element input, output, arg;
	memcpy((mp_limb_t*)input, share, 2 * sizeof(mp_limb_t));
	memcpy((mp_limb_t*)arg, scalar, 2 * sizeof(mp_limb_t));
	output = input * arg;
	memcpy(product, (mp_limb_t*)output, 2 * sizeof(mp_limb_t));
	return 0;
}

int spdz2_ext_processor_mersenne127::adds(const mp_limb_t * share1, const mp_limb_t * share2, mp_limb_t * sum)
{
	ZpMersenne127Element __share1, __share2, __sum;
	memcpy((mp_limb_t*)__share1, share1, 2 * sizeof(mp_limb_t));
	memcpy((mp_limb_t*)__share2, share2, 2 * sizeof(mp_limb_t));
	__sum = __share1 + __share2;
	memcpy((mp_limb_t*)sum, __sum, 2 * sizeof(mp_limb_t));
	return 0;
}

int spdz2_ext_processor_mersenne127::subs(const mp_limb_t * share1, const mp_limb_t * share2, mp_limb_t * diff)
{
	ZpMersenne127Element __share1, __share2, __diff;
	memcpy((mp_limb_t*)__share1, share1, 2 * sizeof(mp_limb_t));
	memcpy((mp_limb_t*)__share2, share2, 2 * sizeof(mp_limb_t));
	__diff = __share1 - __share2;
	memcpy((mp_limb_t*)diff, __diff, 2 * sizeof(mp_limb_t));
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
