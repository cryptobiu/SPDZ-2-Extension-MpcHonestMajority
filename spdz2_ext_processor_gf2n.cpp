
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

GF2E spdz2_ext_processor_gf2n::uint2gf2e(const u_int64_t & value)
{
	return this->the_field->GetElement(value);
}

u_int64_t spdz2_ext_processor_gf2n::gf2e2uint(GF2E & value)
{
	u_int64_t t = 0;
	the_field->elementToBytes((u_int8_t*)&t, value);
	return t;
}

std::string spdz2_ext_processor_gf2n::trace(GF2E & value)
{
	std::stringstream ss;
	ss << value;
	return ss.str();
}

int spdz2_ext_processor_gf2n::mix_add(u_int64_t * share, u_int64_t scalar)
{
	syslog(LOG_INFO, "spdz2_ext_processor_gf2n::mix_add: (s)%lu + (c)%lu", *share, scalar);
	GF2E input, output, arg;
	input = uint2gf2e(*share);
	arg = uint2gf2e(scalar);
	if(Protocol<GF2E>::addShareAndScalar(input, arg, output))
	{
		*share = gf2e2uint(output);
		syslog(LOG_INFO, "spdz2_ext_processor_gf2n::mix_add: result = (s)%lu", *share);
		return 0;
	}
	syslog(LOG_ERR, "spdz2_ext_processor_gf2n::mix_add: protocol addShareAndScalar failure.");
	return -1;
}

int spdz2_ext_processor_gf2n::mix_sub_scalar(u_int64_t * share, u_int64_t scalar)
{
	syslog(LOG_INFO, "spdz2_ext_processor_gf2n::mix_sub_scalar: (s)%lu - (c)%lu", *share, scalar);
	GF2E input, output, arg;
	input = uint2gf2e(*share);
	arg = uint2gf2e(scalar);
	if(Protocol<GF2E>::shareSubScalar(input, arg, output))
	{
		*share = gf2e2uint(output);
		syslog(LOG_INFO, "spdz2_ext_processor_gf2n::mix_sub_scalar: result = (s)%lu", *share);
		return 0;
	}
	syslog(LOG_ERR, "spdz2_ext_processor_gf2n::mix_sub_scalar: protocol shareSubScalar failure.");
	return -1;
}

int spdz2_ext_processor_gf2n::mix_sub_share(u_int64_t scalar, u_int64_t * share)
{
	syslog(LOG_INFO, "spdz2_ext_processor_gf2n::mix_sub_share: (c)%lu - (s)%lu", scalar, *share);
	GF2E input, output, arg;
	input = uint2gf2e(*share);
	arg = uint2gf2e(scalar);
	if(Protocol<GF2E>::scalarSubShare(input, arg, output))
	{
		*share = gf2e2uint(output);
		syslog(LOG_INFO, "spdz2_ext_processor_gf2n::mix_sub_share: result = (s)%lu", *share);
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
	GF2E x = this->the_field->GetElement(25);
	u_int64_t nx = gf2e2uint(x);
	syslog(LOG_DEBUG, "spdz2_ext_processor_gf2n::init_protocol: there and back again 25 -> %lu", nx);
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
	std::vector<GF2E> ext_shares, ext_opens;
	syslog(LOG_INFO, "spdz2_ext_processor_gf2n::protocol_open: calling open for %u shares", (u_int32_t)shares.size());
	for(std::vector<u_int64_t>::const_iterator i = shares.begin(); i != shares.end(); ++i)
	{
		ext_shares.push_back(uint2gf2e(*i));
		syslog(LOG_DEBUG, "spdz2_ext_processor_gf2n::protocol_open() share value %lu", *i);
	}
	ext_opens.resize(ext_shares.size());
	shares.clear();
	opens.clear();

	if(op_open_success = the_party->openShare((int)ext_shares.size(), ext_shares, ext_opens))
	{
		do_verify_open = false;
		if(!do_verify_open || the_party->verify())
		{
			syslog(LOG_INFO, "spdz2_ext_processor_gf2n::protocol_open: verify open for %u opens", (u_int32_t)ext_opens.size());
			u_int64_t open_value;
			for(std::vector<GF2E>::iterator i = ext_opens.begin(); i != ext_opens.end(); ++i)
			{
				open_value = gf2e2uint(*i);
				syslog(LOG_DEBUG, "spdz2_ext_processor_gf2n::protocol_open() open value %lu", open_value);
				opens.push_back(open_value);
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
		ext_shares.clear();
		ext_opens.clear();
	}
	return op_open_success;
}

bool spdz2_ext_processor_gf2n::protocol_triple()
{
	bool op_triple_success = false;
	std::vector<GF2E> triple(3);
	if(op_triple_success = the_party->triples(1, triple))
	{
		*pa = gf2e2uint(triple[0]);
		*pb = gf2e2uint(triple[1]);
		*pc = gf2e2uint(triple[2]);
		syslog(LOG_DEBUG, "spdz2_ext_processor_gf2n::protocol_triple: share a = %lu; share b = %lu; share c = %lu;", *pa, *pb, *pc);
	}

	/*
	{//test the triple with open
		std::vector<GF2E> ext_shares(3), ext_opens(3);
		ext_shares[0] = uint2gf2e(*pa);
		ext_shares[1] = uint2gf2e(*pb);
		ext_shares[2] = uint2gf2e(*pc);

		if(the_party->openShare((int)ext_shares.size(), ext_shares, ext_opens))
		{
			ss.str("");
			ss << "spdz2_ext_processor_gf2n::protocol_triple: triple opened open_a=" << ext_opens[0] << "; open_b=" << ext_opens[1] << "; open_c=" << ext_opens[2] << ";";
			syslog(LOG_DEBUG, "%s", ss.str().c_str());

			u_int64_t a = gf2e2uint(ext_opens[0]), b = gf2e2uint(ext_opens[1]), c = gf2e2uint(ext_opens[2]);
			syslog(LOG_INFO, "spdz2_ext_processor_gf2n::protocol_triple: test open of triple success");
			syslog(LOG_INFO, "spdz2_ext_processor_gf2n::protocol_triple: open a = %lu, open b = %lu, open c = %lu", a, b, c);
		}
		else
		{
			syslog(LOG_ERR, "spdz2_ext_processor_gf2n::protocol_triple: test open of triple failure");
		}
	}*/

	return op_triple_success;
}

bool spdz2_ext_processor_gf2n::protocol_input()
{
	bool op_input_success = false;
	std::vector<GF2E> input_value(1);
	if(op_input_success = the_party->input(input_party_id, input_value))
	{
		*p_input_value = gf2e2uint(input_value[0]);
		syslog(LOG_INFO, "spdz2_ext_processor_gf2n::protocol_input: input value %lu", *p_input_value);

		/*
		{//test the input with open
			std::vector<GF2E> ext_shares(1), ext_opens(1);
			ext_shares[0] = uint2gf2e(*p_input_value);

			if(the_party->openShare((int)ext_shares.size(), ext_shares, ext_opens))
			{
				u_int64_t open_value = gf2e2uint(ext_opens[0]);
				syslog(LOG_INFO, "spdz2_ext_processor_gf2n::protocol_input: test open of input success");
				syslog(LOG_INFO, "spdz2_ext_processor_gf2n::protocol_input: open input = %lu", open_value);
			}
			else
			{
				syslog(LOG_ERR, "spdz2_ext_processor_gf2n::protocol_input: test open of input failure");
			}
		}*/
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
	input_values.clear();
	input_values.resize(num_of_inputs, 0);

	std::vector<GF2E> ext_inputs(num_of_inputs);
	if(op_input_asynch_success = the_party->input(intput_asynch_party_id, ext_inputs))
	{
		u_int64_t input_value;
		for(std::vector<GF2E>::iterator i = ext_inputs.begin(); i != ext_inputs.end(); ++i)
		{
			input_value = gf2e2uint(*i);
			input_values.push_back(input_value);
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
	size_t xy_pair_count =  mult_values.size()/2;
	std::vector<GF2E> x_shares(xy_pair_count), y_shares(xy_pair_count), xy_shares(xy_pair_count);

	for(size_t i = 0; i < xy_pair_count; ++i)
	{
		x_shares[i] = uint2gf2e(mult_values[2*i]);
		y_shares[i] = uint2gf2e(mult_values[2*i+1]);
		syslog(LOG_DEBUG, "spdz2_ext_processor_gf2n::protocol_mult: X-Y pair %lu: X=%lu Y=%lu", i, mult_values[2*i], mult_values[2*i+1]);
	}

	if(op_mult_success = the_party->multShares(xy_pair_count, x_shares, y_shares, xy_shares))
	{
		u_int64_t product_value;
		for(size_t i = 0; i < xy_pair_count; ++i)
		{
			product_value = gf2e2uint(xy_shares[i]);
			products.push_back(product_value);
			syslog(LOG_DEBUG, "spdz2_ext_processor_gf2n::protocol_mult: X-Y product %lu: X*Y=%lu", i, products[i]);
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
	size_t value_count =  immediates_values.size();
	std::vector<GF2E> shares(value_count);

	if(op_share_immediates_success = the_party->load_share_immediates(0, shares, immediates_values))
	{
		u_int64_t value;
		for(size_t i = 0; i < value_count; ++i)
		{
			value = gf2e2uint(shares[i]);
			immediates_shares.push_back(value);
			syslog(LOG_DEBUG, "spdz2_ext_processor_gf2n::protocol_share_immediates: share[%lu] = %lu", i, immediates_shares[i]);
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

	std::vector<GF2E> shares(1);
	if(op_share_immediate_success = the_party->load_share_immediates(0, shares, immediate_value))
	{
		*p_immediate_share = gf2e2uint(shares[0]);
		syslog(LOG_INFO, "spdz2_ext_processor_gf2n::protocol_share_immediate: immediate %lu / share value %lu", immediate_value[0], *p_immediate_share);

		/*
		{//test the input with open
			std::vector<GF2E> ext_shares(1), ext_opens(1);
			ext_shares[0] = uint2gf2e(*p_immediate_share);

			syslog(LOG_DEBUG, "spdz2_ext_processor_gf2n::protocol_share_immediate: shared immediate to test open %s", trace(ext_shares[0]).c_str());

			if(the_party->openShare((int)ext_shares.size(), ext_shares, ext_opens))
			{
				u_int64_t value = gf2e2uint(ext_opens[0]);
				syslog(LOG_INFO, "spdz2_ext_processor_gf2n::protocol_share_immediate: test open share_immediate = %lu", value);
			}
			else
			{
				syslog(LOG_ERR, "spdz2_ext_processor_gf2n::protocol_share_immediate: test open of share_immediate failure");
			}
		}*/
	}
	else
	{
		syslog(LOG_ERR, "spdz2_ext_processor_gf2n::protocol_share_immediate: protocol load_share_immediates failure.");
	}
	return op_share_immediate_success;
}

