
#include "spdz2_ext_processor_gf2n.h"

#include <syslog.h>

spdz2_ext_processor_gf2n::spdz2_ext_processor_gf2n(const int bits)
: spdz2_ext_processor_base(), the_field(NULL), the_party(NULL), gf2n_bits(bits)
{
	mpz_init(m_field);
}

spdz2_ext_processor_gf2n::~spdz2_ext_processor_gf2n()
{
	mpz_clear(m_field);
}

void spdz2_ext_processor_gf2n::mpz2gf2e(const mpz_t mpz_value, GF2E & gf2e_value)
{
	u_int64_t v = mpz_get_ui(mpz_value);
	gf2e_value = the_field->GetElement(v);
}

void spdz2_ext_processor_gf2n::gf2e2mpz(GF2E & gf2e_value, mpz_t mpz_value)
{
	u_int64_t v = 0;
	the_field->elementToBytes((u_int8_t*)&v, gf2e_value);
	mpz_set_ui(mpz_value, v);
}

std::string spdz2_ext_processor_gf2n::trace(GF2E & value)
{
	std::stringstream ss;
	ss << value;
	return ss.str();
}

int spdz2_ext_processor_gf2n::init(const int pid, const int num_of_parties, const int thread_id, const char * field,
			 	 	 	 	 	   const int open_count, const int mult_count, const int bits_count)
{
	spdz2_ext_processor_base::init(pid, num_of_parties, thread_id, field, open_count, mult_count, bits_count);
	the_field = new TemplateField<GF2E>(gf2n_bits);
	the_party = new Protocol<GF2E>(m_nparties, m_pid, open_count, mult_count, bits_count, the_field, get_parties_file());
	if(!the_party->offline())
	{
		syslog(LOG_ERR, "spdz2_ext_processor_gf2n::init_protocol: protocol offline() failure.");
		return -1;
	}
	mpz_ui_pow_ui(m_field, 2, bits_count);
	return 0;

}

int spdz2_ext_processor_gf2n::term()
{
	delete the_party;
	the_party = NULL;
	delete the_field;
	the_field = NULL;
	return 0;
}

int spdz2_ext_processor_gf2n::offline(const int offline_size)
{
	return (the_party->offline())? 0: -1;
}

int spdz2_ext_processor_gf2n::triple(mpz_t a, mpz_t b, mpz_t c)
{
	std::vector<GF2E> triple(3);
	if(the_party->triples(1, triple))
	{
		syslog(LOG_DEBUG, "spdz2_ext_processor_gf2n::protocol_triple: share a = %s; share b = %s; share c = %s;", trace(triple[0]).c_str(), trace(triple[1]).c_str(), trace(triple[2]).c_str());
		gf2e2mpz(triple[0], a);
		gf2e2mpz(triple[1], b);
		gf2e2mpz(triple[2], c);
		return 0;
	}
	else
	{
		syslog(LOG_ERR, "spdz2_ext_processor_gf2n::triple: protocol triples failure.");
	}
	return -1;
}

int spdz2_ext_processor_gf2n::share_immediates(const int share_of_pid, const size_t value_count, const mpz_t * values, mpz_t * shares)
{
	std::vector<GF2E> gf2shares(value_count), gf2values(value_count);
	if(share_of_pid == m_pid)
	{
		for(size_t i = 0; i < value_count; ++i)
		{
			mpz2gf2e(values[i], gf2values[i]);
		}
	}

	if(the_party->makeShare(share_of_pid, gf2values, gf2shares))
	{
		for(size_t i = 0; i < value_count; ++i)
		{
			gf2e2mpz(gf2shares[i], shares[i]);
			syslog(LOG_DEBUG, "spdz2_ext_processor_gf2n::protocol_share_immediates: %lu) value = %s; share = %s", i, trace(gf2values[i]).c_str(), trace(gf2shares[i]).c_str());
		}
		return 0;
	}
	else
	{
		syslog(LOG_ERR, "spdz2_ext_processor_gf2n::protocol_share_immediates: protocol share_immediates failure.");
	}
	return -1;
}

int spdz2_ext_processor_gf2n::bit(mpz_t share)
{
	std::vector<GF2E> zbit_shares(1);
	if(the_party->bits(1, zbit_shares))
	{
		gf2e2mpz(zbit_shares[0], share);
		syslog(LOG_DEBUG, "spdz2_ext_processor_gf2n::bit: protocol bits share = %s.", trace(zbit_shares[0]).c_str());
		return 0;
	}
	else
	{
		syslog(LOG_ERR, "spdz2_ext_processor_gf2n::bit: protocol bits failure.");
	}
	return -1;
}

int spdz2_ext_processor_gf2n::inverse(mpz_t x, mpz_t y)
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
			mpz_mul(product, v, r);
			mpz_mod(y, product, m_field);
			result = 0;
		}
		else
			syslog(LOG_ERR, "spdz2_ext_processor_gf2n::inverse: protocol open() failed");
	}
	else
		syslog(LOG_ERR, "spdz2_ext_processor_gf2n::inverse: protocol triple() failed");

	mpz_clear(r);
	mpz_clear(u);
	mpz_clear(open_u);
	mpz_clear(v);
	mpz_clear(product);

	return result;
}

int spdz2_ext_processor_gf2n::inverse_value(const mpz_t value, mpz_t inverse) const
{
	mpz_t gcd, x, y, P;

	mpz_init(gcd);
	mpz_init(x);
	mpz_init(y);
	mpz_init(P);

	mpz_set(P, m_field);
	mpz_gcdext(gcd, x, y, value, P);
	mpz_mod(inverse, x, P);

	mpz_clear(gcd);
	mpz_clear(x);
	mpz_clear(y);
	mpz_clear(P);

	char szv[128], szi[128];
	syslog(LOG_DEBUG, "spdz2_ext_processor_gf2n::protocol_value_inverse: value = %s; inverse = %s;", mpz_get_str(szv, 10, value), mpz_get_str(szi, 10, inverse));

	return 0;
}

int spdz2_ext_processor_gf2n::open(const size_t share_count, const mpz_t * share_values, mpz_t * opens, int verify)
{
	int result = -1;
	std::vector<GF2E> gf2shares(share_count), gf2opens(share_count);
	syslog(LOG_DEBUG, "spdz2_ext_processor_gf2n::protocol_open: calling open for %u shares", (u_int32_t)share_count);
	for(size_t i = 0; i < share_count; i++)
	{
		mpz2gf2e(share_values[i], gf2shares[i]);
		syslog(LOG_DEBUG, "spdz2_ext_processor_gf2n::protocol_open() share value[%lu] = %s", i, trace(gf2shares[i]).c_str());
	}

	if(the_party->openShare((int)share_count, gf2shares, gf2opens))
	{
		if(!verify || the_party->verify())
		{
			for(size_t i = 0; i < share_count; i++)
			{
				gf2e2mpz(gf2opens[i], opens[i]);
				syslog(LOG_DEBUG, "spdz2_ext_processor_gf2n::protocol_open() opened value[%lu] = %s", i, trace(gf2opens[i]).c_str());
			}
			result = 0;
		}
		else
		{
			syslog(LOG_ERR, "spdz2_ext_processor_gf2n::protocol_open: verify failure.");
		}
	}
	else
	{
		syslog(LOG_ERR, "spdz2_ext_processor_gf2n::protocol_open: openShare failure.");
	}
	return result;
}

int spdz2_ext_processor_gf2n::verify(int * error)
{
	return (the_party->verify())? 0: -1;
}

int spdz2_ext_processor_gf2n::mult(const size_t share_count, const mpz_t * shares, mpz_t * products, int verify)
{
	int result = -1;
	size_t xy_pair_count = share_count/2;
	std::vector<GF2E> x_shares(xy_pair_count), y_shares(xy_pair_count), xy_shares(xy_pair_count);

	for(size_t i = 0; i < xy_pair_count; ++i)
	{
		mpz2gf2e(shares[2*i], x_shares[i]);
		mpz2gf2e(shares[2*i+1], y_shares[i]);
		syslog(LOG_DEBUG, "spdz2_ext_processor_gf2n::protocol_mult: X-Y pair %lu: X=%s Y=%s", i, trace(x_shares[i]).c_str(), trace(y_shares[i]).c_str());
	}

	if(the_party->multShares(xy_pair_count, x_shares, y_shares, xy_shares))
	{
		for(size_t i = 0; i < xy_pair_count; ++i)
		{
			gf2e2mpz(xy_shares[i], products[i]);
			syslog(LOG_DEBUG, "spdz2_ext_processor_gf2n::protocol_mult: X-Y product %lu: X*Y=%s", i, trace(xy_shares[i]).c_str());
		}
		result = 0;
	}
	else
	{
		syslog(LOG_ERR, "spdz2_ext_processor_gf2n::protocol_mult: protocol mult failure.");
	}
	return result;
}


//int spdz2_ext_processor_gf2n::mix_add(mpz_t * share, const mpz_t * scalar)
//{
//	char szsh[128], szsc[128];
//	syslog(LOG_INFO, "spdz2_ext_processor_gf2n::mix_add: (s)%s + (c)%s", mpz_get_str(szsh, 10, *share), mpz_get_str(szsc, 10, *scalar));
//	GF2E input, output, arg;
//	mpz2gf2e(share, input);
//	mpz2gf2e(scalar, arg);
//	if(Protocol<GF2E>::addShareAndScalar(input, arg, output))
//	{
//		gf2e2mpz(output, share);
//		syslog(LOG_INFO, "spdz2_ext_processor_gf2n::mix_add: result = (s)%s", mpz_get_str(szsh, 10, *share));
//		return 0;
//	}
//	syslog(LOG_ERR, "spdz2_ext_processor_gf2n::mix_add: protocol addShareAndScalar failure.");
//	return -1;
//}
//
//int spdz2_ext_processor_gf2n::mix_sub_scalar(mpz_t * share, const mpz_t * scalar)
//{
//	char szsh[128], szsc[128];
//	syslog(LOG_INFO, "spdz2_ext_processor_gf2n::mix_sub_scalar: (s)%s - (c)%s", mpz_get_str(szsh, 10, *share), mpz_get_str(szsc, 10, *scalar));
//	GF2E input, output, arg;
//	mpz2gf2e(share, input);
//	mpz2gf2e(scalar, arg);
//	if(Protocol<GF2E>::shareSubScalar(input, arg, output))
//	{
//		gf2e2mpz(output, share);
//		syslog(LOG_INFO, "spdz2_ext_processor_gf2n::mix_sub_scalar: result = (s)%s", mpz_get_str(szsh, 10, *share));
//		return 0;
//	}
//	syslog(LOG_ERR, "spdz2_ext_processor_gf2n::mix_sub_scalar: protocol shareSubScalar failure.");
//	return -1;
//}
//
//int spdz2_ext_processor_gf2n::mix_sub_share(const mpz_t * scalar, mpz_t * share)
//{
//	char szsh[128], szsc[128];
//	syslog(LOG_INFO, "spdz2_ext_processor_gf2n::mix_sub_share: (c)%s - (s)%s", mpz_get_str(szsc, 10, *scalar), mpz_get_str(szsh, 10, *share));
//	GF2E input, output, arg;
//	mpz2gf2e(share, input);
//	mpz2gf2e(scalar, arg);
//	if(Protocol<GF2E>::scalarSubShare(input, arg, output))
//	{
//		gf2e2mpz(output, share);
//		syslog(LOG_INFO, "spdz2_ext_processor_gf2n::mix_sub_share: result = (s)%s", mpz_get_str(szsh, 10, *share));
//		return 0;
//	}
//	syslog(LOG_ERR, "spdz2_ext_processor_gf2n::mix_sub_share: protocol shareSubScalar failure.");
//	return -1;
//}
//
//int spdz2_ext_processor_gf2n::init_protocol(const int open_count, const int mult_count, const int bits_count)
//{
//	start_setup_measure();
//	the_field = new TemplateField<GF2E>(gf2n_bits);
//	the_party = new Protocol<GF2E>(m_num_of_parties, m_party_id, open_count, mult_count, bits_count, the_field, get_parties_file());
//	start_offline_measure();
//	if(!the_party->offline())
//	{
//		syslog(LOG_ERR, "spdz2_ext_processor_gf2n::init_protocol: protocol offline() failure.");
//		return -1;
//	}
//	return 0;
//}
//
//int spdz2_ext_processor_gf2n::delete_protocol()
//{
//	delete the_party;
//	the_party = NULL;
//	delete the_field;
//	the_field = NULL;
//	return 0;
//}
//
//bool spdz2_ext_processor_gf2n::protocol_offline()
//{
//	return the_party->offline();
//}
//
//bool spdz2_ext_processor_gf2n::protocol_open(const size_t value_count, const mpz_t * shares, mpz_t * opens, bool verify)
//{
//	bool success = false;
//	char sz[128];
//	std::vector<GF2E> ext_shares(value_count), ext_opens(value_count);
//	syslog(LOG_INFO, "spdz2_ext_processor_gf2n::protocol_open: calling open for %lu shares", value_count);
//	for(size_t i = 0; i < value_count; ++i)
//	{
//		mpz2gf2e(shares + i, ext_shares[i]);
//		syslog(LOG_DEBUG, "spdz2_ext_processor_gf2n::protocol_open() share value[%lu] = %s", i, mpz_get_str(sz, 10, shares[i]));
//	}
//
//	if(success = the_party->openShare((int)ext_shares.size(), ext_shares, ext_opens))
//	{
//		if(!verify || the_party->verify())
//		{
//			syslog(LOG_INFO, "spdz2_ext_processor_gf2n::protocol_open: verify open for %u opens", (u_int32_t)ext_opens.size());
//			for(size_t i = 0; i < value_count; ++i)
//			{
//				gf2e2mpz(ext_opens[i], opens + i);
//				syslog(LOG_DEBUG, "spdz2_ext_processor_gf2n::protocol_open() open value[%lu] = %s", i, mpz_get_str(sz, 10, opens[i]));
//			}
//		}
//		else
//		{
//			syslog(LOG_ERR, "spdz2_ext_processor_gf2n::protocol_open: verify failure.");
//		}
//	}
//	else
//	{
//		syslog(LOG_ERR, "spdz2_ext_processor_gf2n::protocol_open: openShare failure.");
//	}
//	return success;
//}
//
//bool spdz2_ext_processor_gf2n::protocol_triple(mpz_t * A, mpz_t * B, mpz_t * C)
//{
//	bool success = false;
//	char sza[128], szb[128], szc[128];
//	std::vector<GF2E> triple(3);
//	if(success = the_party->triples(1, triple))
//	{
//		gf2e2mpz(triple[0], A);
//		gf2e2mpz(triple[1], B);
//		gf2e2mpz(triple[2], C);
//
//		syslog(LOG_DEBUG, "spdz2_ext_processor_gf2n::protocol_triple: share a = %s; share b = %s; share c = %s;",
//				mpz_get_str(sza, 10, *A), mpz_get_str(szb, 10, *B), mpz_get_str(szc, 10, *C));
//	}
//
//	/**/
//	{//test the triple with open
//		std::vector<GF2E> ext_shares(3), ext_opens(3);
//		mpz2gf2e(A, ext_shares[0]);
//		mpz2gf2e(B, ext_shares[1]);
//		mpz2gf2e(C, ext_shares[2]);
//
//		if(the_party->openShare((int)ext_shares.size(), ext_shares, ext_opens))
//		{
//			mpz_t opa, opb, opc;
//			mpz_init(opa);
//			mpz_init(opb);
//			mpz_init(opc);
//			gf2e2mpz(ext_opens[0], &opa);
//			gf2e2mpz(ext_opens[1], &opb);
//			gf2e2mpz(ext_opens[2], &opc);
//
//			syslog(LOG_DEBUG, "spdz2_ext_processor_gf2n::protocol_triple: open a = %s; open b = %s; open c = %s;",
//					mpz_get_str(sza, 10, opa), mpz_get_str(szb, 10, opb), mpz_get_str(szc, 10, opc));
//		}
//		else
//		{
//			syslog(LOG_ERR, "spdz2_ext_processor_gf2n::protocol_triple: test open of triple failure");
//		}
//	}
//
//	return success;
//}
//
//bool spdz2_ext_processor_gf2n::protocol_mult(const size_t count, const mpz_t * input, mpz_t * output, bool verify)
//{
//	bool op_mult_success = false;
//	char szx[128], szy[128];
//	size_t xy_pair_count = count/2;
//	std::vector<GF2E> x_shares(xy_pair_count), y_shares(xy_pair_count), xy_shares(xy_pair_count);
//
//	for(size_t i = 0; i < xy_pair_count; ++i)
//	{
//		mpz2gf2e(input + (2*i), x_shares[i]);
//		mpz2gf2e(input + (2*i+1), y_shares[i]);
//		syslog(LOG_DEBUG, "spdz2_ext_processor_gf2n::protocol_mult: X-Y pair %lu: X=%s Y=%s",
//				i, mpz_get_str(szx, 10, input[2*i]), mpz_get_str(szy, 10, input[2*i+1]));
//	}
//
//	if(op_mult_success = the_party->multShares(xy_pair_count, x_shares, y_shares, xy_shares))
//	{
//		for(size_t i = 0; i < xy_pair_count; ++i)
//		{
//			gf2e2mpz(xy_shares[i], output + i);
//			syslog(LOG_DEBUG, "spdz2_ext_processor_gf2n::protocol_mult: X-Y product %lu: X*Y=%s", i, mpz_get_str(szx, 10, output[i]));
//		}
//	}
//	else
//	{
//		syslog(LOG_ERR, "spdz2_ext_processor_gf2n::protocol_mult: protocol mult failure.");
//	}
//	return op_mult_success;
//}
//
//bool spdz2_ext_processor_gf2n::protocol_share(const int pid, const size_t count, const mpz_t * input, mpz_t * output)
//{
//	bool success = false;
//	std::vector<GF2E> values(count), shares(count);
//
//	for(size_t i = 0; i < count; ++i)
//	{
//		mpz2gf2e(input + i, values[i]);
//	}
//
//	if(success = the_party->makeShare(pid, values, shares))
//	{
//		for(size_t i = 0; i < count; ++i)
//		{
//			gf2e2mpz(shares[i], output + i);
//			char sz[128];
//			syslog(LOG_DEBUG, "spdz2_ext_processor_gf2n::protocol_share_immediates: share[%lu] = %s", i, mpz_get_str(sz, 10, output[i]));
//		}
//	}
//	else
//	{
//		syslog(LOG_ERR, "spdz2_ext_processor_gf2n::protocol_share_immediates: protocol share_immediates failure.");
//	}
//	return success;
//}
//
//bool spdz2_ext_processor_gf2n::protocol_random_value(mpz_t * value)
//{
//	return false;
//}
//
//bool spdz2_ext_processor_gf2n::protocol_value_inverse(const mpz_t * value, mpz_t * inverse)
//{
//	return false;
//}
//
//bool spdz2_ext_processor_gf2n::protocol_verify(int * error)
//{
//	*error = (the_party->verify())? 0: -1;
//}
//
//bool spdz2_ext_processor_gf2n::protocol_bits(const size_t count, mpz_t * bit_shares)
//{
//	bool success = false;
//	std::vector<GF2E> gf2_bit_shares(count);
//
//	if(success = the_party->bits(count, gf2_bit_shares))
//	{
//		for(size_t i = 0; i < count; ++i)
//		{
//			gf2e2mpz(gf2_bit_shares[i], bit_shares + i);
//		}
//	}
//	else
//	{
//		syslog(LOG_ERR, "spdz2_ext_processor_gf2n::protocol_bits: protocol bits failure.");
//	}
//	return success;
//}
//
//bool spdz2_ext_processor_gf2n::protocol_value_mult(const mpz_t * op1, const mpz_t * op2, mpz_t * product)
//{
//	return false;
//}
//
//std::string spdz2_ext_processor_gf2n::get_parties_file()
//{
//	return "Parties_gf2n.txt";
//}
//
//std::string spdz2_ext_processor_gf2n::get_syslog_name()
//{
//	char buffer[32];
//	snprintf(buffer, 32, "spdz2x_gf2n_n%d_%d_%d", gf2n_bits, m_party_id, m_thread_id);
//	return std::string(buffer);
//}
