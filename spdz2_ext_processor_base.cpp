
#include "spdz2_ext_processor_base.h"

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>

#include "Measurement.hpp"

static const char tsk0[] = "setup";
static const char tsk1[] = "offline";
static const char tsk2[] = "online";

//***********************************************************************************************//
spdz2_ext_processor_base::spdz2_ext_processor_base()
{
}

//***********************************************************************************************//
spdz2_ext_processor_base::~spdz2_ext_processor_base()
{
}

//***********************************************************************************************//
int spdz2_ext_processor_base::init(const int pid, const int num_of_parties, const int thread_id, const char * field,
		 	 	 	 	 	 	   const int open_count, const int mult_count, const int bits_count)
{
	m_pid = pid;
	m_nparties = num_of_parties;
	m_thid = thread_id;
	return 0;
}

//***********************************************************************************************//
int spdz2_ext_processor_base::load_inputs()
{
	std::list<std::string> party_input_specs;
	if(0 != load_party_input_specs(party_input_specs))
	{
		syslog(LOG_ERR, "spdz2_ext_processor_base::load_inputs: failed loading the party input specs");
		return -1;
	}

	for(std::list<std::string>::const_iterator i = party_input_specs.begin(); i != party_input_specs.end(); ++i)
	{
		if(0 != load_party_inputs(*i))
		{
			syslog(LOG_ERR, "spdz2_ext_processor_base::load_inputs: failed loading party input by spec [%s]", i->c_str());
			return -1;
		}
	}

	return 0;
}

//***********************************************************************************************//
int spdz2_ext_processor_base::load_party_input_specs(std::list<std::string> & party_input_specs)
{
	static const char common_inputs_spec[] = "Parties_Inputs.txt";

	party_input_specs.clear();
	FILE * pf = fopen(common_inputs_spec, "r");
	if(NULL != pf)
	{
		char sz[128];
		while(NULL != fgets(sz, 128, pf))
		{
			if (NULL == strstr(sz, "#"))
				party_input_specs.push_back(sz);
		}
		fclose(pf);
	}
	else
	{
		syslog(LOG_ERR, "spdz2_ext_processor_base::load_party_input_specs: failed to open [%s].", common_inputs_spec);
		return -1;
	}
	return (party_input_specs.empty())? -1: 0;
}

//***********************************************************************************************//
int spdz2_ext_processor_base::load_party_inputs(const std::string & party_input_spec)
{
	std::string::size_type pos = party_input_spec.find(',');
	if(std::string::npos != pos)
	{
		int pid = (int)strtol(party_input_spec.substr(0, pos).c_str(), NULL, 10);
		size_t count = (size_t)strtol(party_input_spec.substr(pos+1).c_str(), NULL, 10);
		if(0 < count)
			return load_party_inputs(pid, count);
		else
			return 0;
	}
	else
	{
		syslog(LOG_ERR, "spdz2_ext_processor_base::load_party_inputs: invalid party input spec format [%s]", party_input_spec.c_str());
		return -1;
	}
}

//***********************************************************************************************//
int spdz2_ext_processor_base::load_party_inputs(const int pid, const size_t count)
{
	return (pid == this->m_pid)? load_self_party_inputs(count): load_peer_party_inputs(pid, count);
}

//***********************************************************************************************//
int spdz2_ext_processor_base::load_self_party_inputs(const size_t count)
{
	int result = -1;
	mpz_t * clr_values = NULL;
	if(0 == load_clr_party_inputs(&clr_values, count) && NULL != clr_values)
	{
		result = load_peer_party_inputs(m_pid, count, clr_values);

		for(size_t i = 0; i < count; ++i)
			mpz_clear(clr_values[i]);
		delete clr_values;
	}
	else
		syslog(LOG_ERR, "spdz2_ext_processor_base::load_self_party_inputs: failed loading clear inputs.");
	return result;
}

//***********************************************************************************************//
int spdz2_ext_processor_base::load_peer_party_inputs(const int pid, const size_t count, const mpz_t * clr_values)
{
	int result = -1;
	shared_input_t party_inputs;
	party_inputs.share_count = count;
	party_inputs.share_index = 0;
	party_inputs.shared_values = new mpz_t[party_inputs.share_count];
	for(size_t i = 0; i < party_inputs.share_count; ++i)
		mpz_init(party_inputs.shared_values[i]);

	//if(protocol_share(pid, count, (NULL != clr_values)? clr_values: party_inputs.shared_values, party_inputs.shared_values))

	if(0 == share_immediates(pid, count, (m_pid == pid)? clr_values: NULL, party_inputs.shared_values))
	{
		if(m_shared_inputs.insert(std::pair<int, shared_input_t>(pid, party_inputs)).second)
			result = 0;
		else
		{
			for(size_t i = 0; i < count; ++i)
				mpz_clear(party_inputs.shared_values[i]);
			delete party_inputs.shared_values;
			syslog(LOG_ERR, "spdz2_ext_processor_base::load_peer_party_inputs: failed to map-insert shared inputs for pid=%d.", pid);
		}
	}
	else
	{
		for(size_t i = 0; i < count; ++i)
			mpz_clear(party_inputs.shared_values[i]);
		delete party_inputs.shared_values;
		syslog(LOG_ERR, "spdz2_ext_processor_base::load_peer_party_inputs: protocol_share() failed for pid=%d.", pid);
	}
	return result;
}

//***********************************************************************************************//
int spdz2_ext_processor_base::load_clr_party_inputs(mpz_t ** clr_values, const size_t count)
{
	char sz[128];
	snprintf(sz, 128, "party_%d_input.txt", m_pid);
	std::string input_file = sz;

	FILE * pf = fopen(input_file.c_str(), "r");
	if(NULL != pf)
	{
		*clr_values = new mpz_t[count];
		size_t clr_values_idx = 0;

		while(NULL != fgets(sz, 128, pf))
		{
			mpz_init((*clr_values)[clr_values_idx]);
			mpz_set_str((*clr_values)[clr_values_idx++], sz, 10);
			if(clr_values_idx >= count)
				break;
		}
		fclose(pf);

		if(count > clr_values_idx)
		{
			for(size_t i = 0; i < clr_values_idx; i++)
				mpz_clear((*clr_values)[i]);
			delete (*clr_values);
			*clr_values = NULL;
			syslog(LOG_ERR, "spdz2_ext_processor_base::load_clr_party_inputs: not enough inputs in file [%s]; required %lu; file has %lu;",
					input_file.c_str(), count, clr_values_idx);
		}
	}
	return (NULL != (*clr_values))? 0: -1;
}

//***********************************************************************************************//
