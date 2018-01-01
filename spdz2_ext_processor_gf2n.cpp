
#include "spdz2_ext_processor_gf2n.h"

#include <syslog.h>

spdz2_ext_processor_gf2n::spdz2_ext_processor_gf2n(const int bits)
 : spdz2_ext_processor_base()
, the_field(NULL), the_party(NULL), gf2n_bits(bits)
{
}

spdz2_ext_processor_gf2n::~spdz2_ext_processor_gf2n()
{
}

void spdz2_ext_processor_gf2n::mpz2gf2e(const mpz_t * mpz_value, GF2E & gf2e_value)
{
	u_int64_t v = mpz_get_ui(*mpz_value);
	gf2e_value = the_field->GetElement(v);
}

void spdz2_ext_processor_gf2n::gf2e2mpz(GF2E & gf2e_value, mpz_t * mpz_value)
{
	u_int64_t v = 0;
	the_field->elementToBytes((u_int8_t*)&v, gf2e_value);
	mpz_set_ui(*mpz_value, v);
}

std::string spdz2_ext_processor_gf2n::trace(GF2E & value)
{
	std::stringstream ss;
	ss << value;
	return ss.str();
}

int spdz2_ext_processor_gf2n::mix_add(mpz_t * share, const mpz_t * scalar)
{
	char szsh[128], szsc[128];
	syslog(LOG_INFO, "spdz2_ext_processor_gf2n::mix_add: (s)%s + (c)%s", mpz_get_str(szsh, 10, *share), mpz_get_str(szsc, 10, *scalar));
	GF2E input, output, arg;
	mpz2gf2e(share, input);
	mpz2gf2e(scalar, arg);
	if(Protocol<GF2E>::addShareAndScalar(input, arg, output))
	{
		gf2e2mpz(output, share);
		syslog(LOG_INFO, "spdz2_ext_processor_gf2n::mix_add: result = (s)%s", mpz_get_str(szsh, 10, *share));
		return 0;
	}
	syslog(LOG_ERR, "spdz2_ext_processor_gf2n::mix_add: protocol addShareAndScalar failure.");
	return -1;
}

int spdz2_ext_processor_gf2n::mix_sub_scalar(mpz_t * share, const mpz_t * scalar)
{
	char szsh[128], szsc[128];
	syslog(LOG_INFO, "spdz2_ext_processor_gf2n::mix_sub_scalar: (s)%s - (c)%s", mpz_get_str(szsh, 10, *share), mpz_get_str(szsc, 10, *scalar));
	GF2E input, output, arg;
	mpz2gf2e(share, input);
	mpz2gf2e(scalar, arg);
	if(Protocol<GF2E>::shareSubScalar(input, arg, output))
	{
		gf2e2mpz(output, share);
		syslog(LOG_INFO, "spdz2_ext_processor_gf2n::mix_sub_scalar: result = (s)%s", mpz_get_str(szsh, 10, *share));
		return 0;
	}
	syslog(LOG_ERR, "spdz2_ext_processor_gf2n::mix_sub_scalar: protocol shareSubScalar failure.");
	return -1;
}

int spdz2_ext_processor_gf2n::mix_sub_share(const mpz_t * scalar, mpz_t * share)
{
	char szsh[128], szsc[128];
	syslog(LOG_INFO, "spdz2_ext_processor_gf2n::mix_sub_share: (c)%s - (s)%s", mpz_get_str(szsc, 10, *scalar), mpz_get_str(szsh, 10, *share));
	GF2E input, output, arg;
	mpz2gf2e(share, input);
	mpz2gf2e(scalar, arg);
	if(Protocol<GF2E>::scalarSubShare(input, arg, output))
	{
		gf2e2mpz(output, share);
		syslog(LOG_INFO, "spdz2_ext_processor_gf2n::mix_sub_share: result = (s)%s", mpz_get_str(szsh, 10, *share));
		return 0;
	}
	syslog(LOG_ERR, "spdz2_ext_processor_gf2n::mix_sub_share: protocol shareSubScalar failure.");
	return -1;
}

int spdz2_ext_processor_gf2n::init_protocol()
{
	the_field = new TemplateField<GF2E>(gf2n_bits);
	the_party = new Protocol<GF2E>(num_of_parties, party_id, offline_size, offline_size, the_field, input_file, "Parties_gf2n.txt");
	if(!the_party->offline())
	{
		syslog(LOG_ERR, "spdz2_ext_processor_gf2n::init_protocol: protocol offline() failure.");
		return -1;
	}
	return 0;
}

void spdz2_ext_processor_gf2n::delete_protocol()
{
	delete the_party;
	the_party = NULL;
	delete the_field;
	the_field = NULL;
}

bool spdz2_ext_processor_gf2n::protocol_offline()
{
	return the_party->offline();
}

bool spdz2_ext_processor_gf2n::protocol_open()
{
	bool op_open_success = false;
	char sz[128];
	std::vector<GF2E> ext_shares(open_share_value_count), ext_opens(open_share_value_count);
	syslog(LOG_INFO, "spdz2_ext_processor_gf2n::protocol_open: calling open for %lu shares", open_share_value_count);
	for(size_t i = 0; i < open_share_value_count; ++i)
	{
		mpz2gf2e(to_open_share_values + i, ext_shares[i]);
		syslog(LOG_DEBUG, "spdz2_ext_processor_gf2n::protocol_open() share value[%lu] = %s", i, mpz_get_str(sz, 10, to_open_share_values[i]));
	}

	if(op_open_success = the_party->openShare((int)ext_shares.size(), ext_shares, ext_opens))
	{
		do_verify_open = false;
		if(!do_verify_open || the_party->verify())
		{
			syslog(LOG_INFO, "spdz2_ext_processor_gf2n::protocol_open: verify open for %u opens", (u_int32_t)ext_opens.size());
			for(size_t i = 0; i < open_share_value_count; ++i)
			{
				gf2e2mpz(ext_opens[i], opened_share_values + i);
				syslog(LOG_DEBUG, "spdz2_ext_processor_gf2n::protocol_open() open value[%lu] = %s", i, mpz_get_str(sz, 10, opened_share_values[i]));
			}
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
	return op_open_success;
}

bool spdz2_ext_processor_gf2n::protocol_triple()
{
	bool op_triple_success = false;
	char sza[128], szb[128], szc[128];
	std::vector<GF2E> triple(3);
	if(op_triple_success = the_party->triples(1, triple))
	{
		gf2e2mpz(triple[0], pa);
		gf2e2mpz(triple[1], pb);
		gf2e2mpz(triple[2], pc);

		syslog(LOG_DEBUG, "spdz2_ext_processor_gf2n::protocol_triple: share a = %s; share b = %s; share c = %s;",
				mpz_get_str(sza, 10, *pa), mpz_get_str(szb, 10, *pb), mpz_get_str(szc, 10, *pc));
	}

	/**/
	{//test the triple with open
		std::vector<GF2E> ext_shares(3), ext_opens(3);
		mpz2gf2e(pa, ext_shares[0]);
		mpz2gf2e(pb, ext_shares[1]);
		mpz2gf2e(pc, ext_shares[2]);

		if(the_party->openShare((int)ext_shares.size(), ext_shares, ext_opens))
		{
			mpz_t opa, opb, opc;
			mpz_init(opa);
			mpz_init(opb);
			mpz_init(opc);
			gf2e2mpz(ext_opens[0], &opa);
			gf2e2mpz(ext_opens[1], &opb);
			gf2e2mpz(ext_opens[2], &opc);

			syslog(LOG_DEBUG, "spdz2_ext_processor_gf2n::protocol_triple: open a = %s; open b = %s; open c = %s;",
					mpz_get_str(sza, 10, opa), mpz_get_str(szb, 10, opb), mpz_get_str(szc, 10, opc));
		}
		else
		{
			syslog(LOG_ERR, "spdz2_ext_processor_gf2n::protocol_triple: test open of triple failure");
		}
	}

	return op_triple_success;
}

bool spdz2_ext_processor_gf2n::protocol_input()
{
	bool op_input_success = false;
	char sz[128];
	std::vector<GF2E> input_value(1);
	if(op_input_success = the_party->input(input_party_id, input_value))
	{
		gf2e2mpz(input_value[0], p_input_value);
		syslog(LOG_INFO, "spdz2_ext_processor_gf2n::protocol_input: input value %s", mpz_get_str(sz, 10, *p_input_value));

		/**/
		{//test the input with open
			std::vector<GF2E> ext_shares(1), ext_opens(1);
			mpz2gf2e(p_input_value, ext_shares[0]);

			if(the_party->openShare((int)ext_shares.size(), ext_shares, ext_opens))
			{
				mpz_t opv;
				mpz_init(opv);
				gf2e2mpz(ext_opens[0], &opv);
				syslog(LOG_INFO, "spdz2_ext_processor_gf2n::protocol_input: test open input = %s", mpz_get_str(sz, 10, opv));
			}
			else
			{
				syslog(LOG_ERR, "spdz2_ext_processor_gf2n::protocol_input: test open of input failure");
			}
		}
	}
	else
	{
		syslog(LOG_ERR, "spdz2_ext_processor_gf2n::protocol_input: protocol input failure.");
	}
	return op_input_success;
}

bool spdz2_ext_processor_gf2n::protocol_input_asynch()
{
	bool op_input_asynch_success = false;

	std::vector<GF2E> ext_inputs(intput_asynch_count);
	if(op_input_asynch_success = the_party->input(intput_asynch_party_id, ext_inputs))
	{
		for(size_t i = 0; i < intput_asynch_count; ++i)
		{
			gf2e2mpz(ext_inputs[i], intput_asynch_values + i);
		}
	}
	else
	{
		syslog(LOG_ERR, "spdz2_ext_processor_gf2n::protocol_input_asynch: protocol input failure.");
	}
	return op_input_asynch_success;
}

bool spdz2_ext_processor_gf2n::protocol_mult()
{
	bool op_mult_success = false;
	char szx[128], szy[128];
	size_t xy_pair_count = mult_share_count/2;
	std::vector<GF2E> x_shares(xy_pair_count), y_shares(xy_pair_count), xy_shares(xy_pair_count);

	for(size_t i = 0; i < xy_pair_count; ++i)
	{
		mpz2gf2e(mult_shares + (2*i), x_shares[i]);
		mpz2gf2e(mult_shares + (2*i+1), y_shares[i]);
		syslog(LOG_DEBUG, "spdz2_ext_processor_gf2n::protocol_mult: X-Y pair %lu: X=%s Y=%s",
				i, mpz_get_str(szx, 10, mult_shares[2*i]), mpz_get_str(szy, 10, mult_shares[2*i+1]));
	}

	if(op_mult_success = the_party->multShares(xy_pair_count, x_shares, y_shares, xy_shares))
	{
		for(size_t i = 0; i < xy_pair_count; ++i)
		{
			gf2e2mpz(xy_shares[i], mult_products + i);
			syslog(LOG_DEBUG, "spdz2_ext_processor_gf2n::protocol_mult: X-Y product %lu: X*Y=%s", i, mpz_get_str(szx, 10, mult_products[i]));
		}
	}
	else
	{
		syslog(LOG_ERR, "spdz2_ext_processor_gf2n::protocol_mult: protocol mult failure.");
	}
	return op_mult_success;
}

bool spdz2_ext_processor_gf2n::protocol_share_immediates()
{
	bool op_share_immediates_success = false;
	std::vector<GF2E> shares(immediates_count);
	vector<std::string> str_immediates_values;
	load_share_immediates_strings(str_immediates_values);

	if(op_share_immediates_success = the_party->load_share_immediates(0, shares, str_immediates_values))
	{
		for(size_t i = 0; i < immediates_count; ++i)
		{
			gf2e2mpz(shares[i], immediates_shares + i);
			char sz[128];
			syslog(LOG_DEBUG, "spdz2_ext_processor_gf2n::protocol_share_immediates: share[%lu] = %s", i, mpz_get_str(sz, 10, immediates_shares[i]));
		}
	}
	else
	{
		syslog(LOG_ERR, "spdz2_ext_processor_gf2n::protocol_share_immediates: protocol share_immediates failure.");
	}
	return op_share_immediates_success;
}

bool spdz2_ext_processor_gf2n::protocol_share_immediate()
{
	bool op_share_immediate_success = false;
	char sz[128];
	std::vector<GF2E> shares(1);
	vector<std::string> str_immediates_value(1);
	str_immediates_value[0] = mpz_get_str(sz, 10, *immediate_value);

	if(op_share_immediate_success = the_party->load_share_immediates(0, shares, str_immediates_value))
	{
		gf2e2mpz(shares[0], immediate_share);
		syslog(LOG_INFO, "spdz2_ext_processor_gf2n::protocol_share_immediate: immediate %s / share value %s",
				str_immediates_value[0].c_str(), mpz_get_str(sz, 10, *immediate_share));

		/**/
		{//test the input with open
			std::vector<GF2E> ext_shares(1), ext_opens(1);
			mpz2gf2e(immediate_share, ext_shares[0]);

			if(the_party->openShare((int)ext_shares.size(), ext_shares, ext_opens))
			{
				u_int64_t value = 0;
				the_field->elementToBytes((u_int8_t*)&value, ext_opens[0]);
				syslog(LOG_INFO, "spdz2_ext_processor_gf2n::protocol_share_immediate: test open share_immediate = %lu", value);
			}
			else
			{
				syslog(LOG_ERR, "spdz2_ext_processor_gf2n::protocol_share_immediate: test open of share_immediate failure");
			}
		}
	}
	else
	{
		syslog(LOG_ERR, "spdz2_ext_processor_gf2n::protocol_share_immediate: protocol load_share_immediates failure.");
	}
	return op_share_immediate_success;
}


bool spdz2_ext_processor_gf2n::protocol_random_value(mpz_t * value) const
{
	return false;
}

bool spdz2_ext_processor_gf2n::protocol_value_inverse(const mpz_t * value, mpz_t * inverse) const
{
	return false;
}
