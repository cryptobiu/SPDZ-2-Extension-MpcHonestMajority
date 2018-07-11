#include "spdz2_ext_processor_mersenne127.h"

#include <syslog.h>

//spdz2_ext_processor_mersenne127::spdz2_ext_processor_mersenne127()
// : spdz2_ext_processor_base()
// , the_field(NULL), the_party(NULL)
//{
//	gmp_randinit_mt(the_gmp_rstate);
//}
//
//spdz2_ext_processor_mersenne127::~spdz2_ext_processor_mersenne127()
//{
//}
//
//int spdz2_ext_processor_mersenne127::mix_add(mpz_t * share, const mpz_t * scalar)
//{
//	char szsh[128], szsc[128];
//	syslog(LOG_INFO, "spdz2_ext_processor_mersenne127::mix_add: (s)%s + (c)%s", mpz_get_str(szsh, 10, *share), mpz_get_str(szsc, 10, *scalar));
//	Mersenne127 input(*share), output, arg(*scalar);
//	if(Protocol<Mersenne127>::addShareAndScalar(input, arg, output))
//	{
//		mpz_set(*share, *output.get_mpz_t());
//		syslog(LOG_INFO, "spdz2_ext_processor_mersenne127::mix_add: result = (s)%s", mpz_get_str(szsh, 10, *share));
//		return 0;
//	}
//	syslog(LOG_ERR, "spdz2_ext_processor_mersenne127::mix_add: protocol addShareAndScalar failure.");
//	return -1;
//}
//
//int spdz2_ext_processor_mersenne127::mix_sub_scalar(mpz_t * share, const mpz_t * scalar)
//{
//	char szsh[128], szsc[128];
//	syslog(LOG_INFO, "spdz2_ext_processor_mersenne127::mix_sub_scalar: (s)%s - (c)%s", mpz_get_str(szsh, 10, *share), mpz_get_str(szsc, 10, *scalar));
//	Mersenne127 input(*share), output, arg(*scalar);
//	if(Protocol<Mersenne127>::shareSubScalar(input, arg, output))
//	{
//		mpz_set(*share, *output.get_mpz_t());
//		syslog(LOG_INFO, "spdz2_ext_processor_mersenne127::mix_sub_scalar: result = (s)%s", mpz_get_str(szsh, 10, *share));
//		return 0;
//	}
//	syslog(LOG_ERR, "spdz2_ext_processor_mersenne127::mix_sub_scalar: protocol shareSubScalar failure.");
//	return -1;
//}
//
//int spdz2_ext_processor_mersenne127::mix_sub_share(const mpz_t * scalar, mpz_t * share)
//{
//	char szsh[128], szsc[128];
//	syslog(LOG_INFO, "spdz2_ext_processor_mersenne127::mix_sub_share: (c)%s - (s)%s", mpz_get_str(szsc, 10, *scalar), mpz_get_str(szsh, 10, *share));
//	Mersenne127 input(*share), output, arg(*scalar);
//	if(Protocol<Mersenne127>::scalarSubShare(input, arg, output))
//	{
//		mpz_set(*share, *output.get_mpz_t());
//		syslog(LOG_INFO, "spdz2_ext_processor_mersenne127::mix_sub_share: result = (s)%s", mpz_get_str(szsh, 10, *share));
//		return 0;
//	}
//	syslog(LOG_ERR, "spdz2_ext_processor_mersenne127::mix_sub_share: protocol shareSubScalar failure.");
//	return -1;
//}
//
//int spdz2_ext_processor_mersenne127::init_protocol(const int open_count, const int mult_count, const int bits_count)
//{
//	start_setup_measure();
//	the_field = new TemplateField<Mersenne127>(0);
//	the_party = new Protocol<Mersenne127>(m_num_of_parties, m_party_id, open_count, mult_count, bits_count, the_field, get_parties_file());
//	start_offline_measure();
//	if(!the_party->offline())
//	{
//		syslog(LOG_ERR, "spdz2_ext_processor_mersenne127::init_protocol: protocol offline() failure.");
//		return -1;
//	}
//	return 0;
//}
//
//int spdz2_ext_processor_mersenne127::delete_protocol()
//{
//	delete the_party;
//	the_party = NULL;
//	delete the_field;
//	the_field = NULL;
//	return 0;
//}
//
//bool spdz2_ext_processor_mersenne127::protocol_offline()
//{
//	return the_party->offline();
//}
//
//bool spdz2_ext_processor_mersenne127::protocol_open(const size_t value_count, const mpz_t * shares, mpz_t * opens, bool verify)
//{
//	bool success = false;
//	char sz[128];
//
//	if(m_aux1.size() < value_count) m_aux1.resize(value_count);
//	if(m_aux2.size() < value_count) m_aux2.resize(value_count);
//
//	syslog(LOG_INFO, "spdz2_ext_processor_mersenne127::protocol_open: calling open for %u shares", (u_int32_t)value_count);
//	for(size_t i = 0; i < value_count; i++)
//	{
//		m_aux1[i].set_mpz_t(shares + i);
//		syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne127::protocol_open() share value[%lu] = %s", i, mpz_get_str(sz, 10, shares[i]));
//	}
//
//	if(success = the_party->openShare((int)value_count, m_aux1, m_aux2))
//	{
//		if(!verify || the_party->verify())
//		{
//			for(size_t i = 0; i < value_count; i++)
//			{
//				mpz_set(opens[i], *m_aux2[i].get_mpz_t());
//				syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne127::protocol_open() opened value[%lu] = %s", i, mpz_get_str(sz, 10, opens[i]));
//			}
//		}
//		else
//		{
//			syslog(LOG_ERR, "spdz2_ext_processor_mersenne127::protocol_open: verify failure.");
//		}
//	}
//	else
//	{
//		syslog(LOG_ERR, "spdz2_ext_processor_mersenne127::protocol_open: openShare failure.");
//	}
//	return success;
//}
//
//bool spdz2_ext_processor_mersenne127::protocol_triple(mpz_t * A, mpz_t * B, mpz_t * C)
//{
//	bool success = false;
//	char sza[128], szb[128], szc[128];
//	std::vector<Mersenne127> triple(3);
//	if(success = the_party->triples(1, triple))
//	{
//		mpz_set(*A, *triple[0].get_mpz_t());
//		mpz_set(*B, *triple[1].get_mpz_t());
//		mpz_set(*C, *triple[2].get_mpz_t());
//		syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne127::protocol_triple: share a = %s; share b = %s; share c = %s;",
//				mpz_get_str(sza, 10, *A), mpz_get_str(szb, 10, *B), mpz_get_str(szc, 10, *C));
//	}
//
//	/*
//	{//test the triple with open
//		std::vector<Mersenne127> ext_shares(3), ext_opens(3);
//
//		ext_shares[0].set_mpz_t(A);
//		ext_shares[1].set_mpz_t(B);
//		ext_shares[2].set_mpz_t(C);
//
//		if(the_party->openShare((int)ext_shares.size(), ext_shares, ext_opens))
//		{
//			syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne127::protocol_triple: test open of triple a = %s, open b = %s, open c = %s",
//					mpz_get_str(sza, 10, *ext_opens[0].get_mpz_t()), mpz_get_str(szb, 10, *ext_opens[1].get_mpz_t()), mpz_get_str(szc, 10, *ext_opens[2].get_mpz_t()));
//		}
//		else
//		{
//			syslog(LOG_ERR, "spdz2_ext_processor_mersenne127::protocol_triple: test open of triple failure");
//		}
//	}*/
//
//	return success;
//}
//
//bool spdz2_ext_processor_mersenne127::protocol_mult(const size_t count, const mpz_t * input, mpz_t * output, bool verify)
//{
//	bool success = false;
//	char szx[128], szy[128];
//	size_t xy_pair_count = count/2;
//
//	if(m_aux1.size() < xy_pair_count) m_aux1.resize(xy_pair_count);
//	if(m_aux2.size() < xy_pair_count) m_aux2.resize(xy_pair_count);
//	if(m_aux3.size() < xy_pair_count) m_aux3.resize(xy_pair_count);
//
//	for(size_t i = 0; i < xy_pair_count; ++i)
//	{
//		m_aux1[i].set_mpz_t(input + (2*i));
//		m_aux2[i].set_mpz_t(input + (2*i+1));
//		syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne127::protocol_mult: X-Y pair %lu: X=%s Y=%s",
//				i, mpz_get_str(szx, 10, *m_aux1[i].get_mpz_t()), mpz_get_str(szy, 10, *m_aux2[i].get_mpz_t()));
//	}
//
//	if(success = the_party->multShares(xy_pair_count, m_aux1, m_aux2, m_aux3))
//	{
//		for(size_t i = 0; i < xy_pair_count; ++i)
//		{
//			mpz_set(output[i], *m_aux3[i].get_mpz_t());
//			syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne127::protocol_mult: X-Y product %lu: X*Y=%s", i, mpz_get_str(szx, 10, *m_aux3[i].get_mpz_t()));
//		}
//	}
//	else
//	{
//		syslog(LOG_ERR, "spdz2_ext_processor_mersenne127::protocol_mult: protocol mult failure.");
//	}
//	return success;
//}
//
//bool spdz2_ext_processor_mersenne127::protocol_share(const int pid, const size_t count, const mpz_t * input, mpz_t * output)
//{
//	bool success = false;
//	char sz[128];
//	std::vector<Mersenne127> values(count), shares(count);
//
//	for(size_t i = 0; i < count; ++i)
//	{
//		values[i].set_mpz_t(input + i);
//	}
//
//	if(success = the_party->makeShare(pid, values, shares))
//	{
//		for(size_t i = 0; i < count; ++i)
//		{
//			mpz_set(output[i], *shares[i].get_mpz_t());
//			syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne127::protocol_share: share[%lu] = %s", i, mpz_get_str(sz, 10, output[i]));
//		}
//	}
//	else
//	{
//		syslog(LOG_ERR, "spdz2_ext_processor_mersenne127::protocol_share: protocol share failure.");
//	}
//
//	return success;
//}
//
//bool spdz2_ext_processor_mersenne127::protocol_random_value(mpz_t * value)
//{
//	mpz_t max_num;
//	mpz_init(max_num);
//	mpz_set_str(max_num, Mersenne127::M127, 10);
//
//	mpz_urandomm(*value, the_gmp_rstate, max_num);
//
//	mpz_clear(max_num);
//
//	char sz[128];
//	syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne127::protocol_random_value: random value = %s", mpz_get_str(sz, 10, *value));
//
//	return true;
//}
//
//bool spdz2_ext_processor_mersenne127::protocol_value_inverse(const mpz_t * value, mpz_t * inverse)
//{
//	mpz_t gcd, x, y, P;
//
//	mpz_init(gcd);
//	mpz_init(x);
//    mpz_init(y);
//    mpz_init(P);
//
//    mpz_set_str(P, Mersenne127::M127, 10);
//
//    mpz_gcdext(gcd, x, y, *value, P);
//    mpz_mod(*inverse, x, P);
//
//    mpz_clear(gcd);
//    mpz_clear(x);
//    mpz_clear(y);
//    mpz_clear(P);
//
//    char szv[128], szi[128];
//    syslog(LOG_DEBUG, "spdz2_ext_processor_mersenne127::protocol_value_inverse: value = %s; inverse = %s;",
//    		mpz_get_str(szv, 10, *value), mpz_get_str(szi, 10, *inverse));
//
//	return true;
//}
//
//bool spdz2_ext_processor_mersenne127::protocol_verify(int * error)
//{
//	return (0 == (*error = (the_party->verify())? 0: -1))? true: false;
//}
//
//bool spdz2_ext_processor_mersenne127::protocol_bits(const size_t count, mpz_t * bit_shares)
//{
//	bool success = false;
//
//	if(m_aux1.size() < count) m_aux1.resize(count);
//
//	if(success = the_party->bits(count, m_aux1))
//	{
//		for(size_t i = 0; i < count; ++i)
//		{
//			mpz_set(bit_shares[i], *m_aux1[i].get_mpz_t());
//		}
//	}
//	else
//	{
//		syslog(LOG_ERR, "spdz2_ext_processor_mersenne127::protocol_bits: protocol bits failure.");
//	}
//	return success;
//}
//
//bool spdz2_ext_processor_mersenne127::protocol_value_mult(const mpz_t * op1, const mpz_t * op2, mpz_t * product)
//{
//	mpz_t P, temp;
//    mpz_init(P);
//    mpz_init(temp);
//
//    mpz_set_str(P, Mersenne127::M127, 10);
//
//    mpz_mul(temp, *op1, *op2);
//    mpz_mod(*product, temp, P);
//
//    mpz_clear(P);
//    mpz_clear(temp);
//    return true;
//}
//
//std::string spdz2_ext_processor_mersenne127::get_parties_file()
//{
//	return "Parties_gfp.txt";
//}
//
//std::string spdz2_ext_processor_mersenne127::get_syslog_name()
//{
//	char buffer[32];
//	snprintf(buffer, 32, "spdz2x_gfp_m127_%d_%d", m_party_id, m_thread_id);
//	return std::string(buffer);
//}
