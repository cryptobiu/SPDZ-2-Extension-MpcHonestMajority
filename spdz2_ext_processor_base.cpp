
#include "spdz2_ext_processor_base.h"

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>

//#include "Measurement.hpp"

#include <log4cpp/Category.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/SimpleLayout.hh>
#include <log4cpp/RollingFileAppender.hh>
#include <log4cpp/SimpleLayout.hh>
#include <log4cpp/BasicLayout.hh>
#include <log4cpp/PatternLayout.hh>

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
		 	 	 	 	 	 	   const int open_count, const int mult_count, const int bits_count, int log_level)
{
	m_pid = pid;
	m_nparties = num_of_parties;
	m_thid = thread_id;
	if(0 != init_log(log_level))
	{
		syslog(LOG_ERR, "%s: init_log() failure.", __FUNCTION__);
		return -1;
	}
	return 0;
}

//***********************************************************************************************//
int spdz2_ext_processor_base::init_log(int log_level)
{
	static const char the_layout[] = "%d{%y-%m-%d %H:%M:%S.%l}| %-6p | %-15c | %m%n";

	std::string log_file = "/var/log/";
	log_file += get_log_file();
	m_logcat = get_log_category();

    log4cpp::Layout * log_layout = NULL;
    log4cpp::Appender * appender = new log4cpp::RollingFileAppender("rlf.appender", log_file.c_str(), 10*1024*1024, 10);

    bool pattern_layout = false;
    try
    {
        log_layout = new log4cpp::PatternLayout();
        ((log4cpp::PatternLayout *)log_layout)->setConversionPattern(the_layout);
        appender->setLayout(log_layout);
        pattern_layout = true;
    }
    catch(...)
    {
        pattern_layout = false;
    }

    if(!pattern_layout)
    {
        log_layout = new log4cpp::BasicLayout();
        appender->setLayout(log_layout);
    }

    log4cpp::Category::getInstance(m_logcat).addAppender(appender);
    log4cpp::Category::getInstance(m_logcat).setPriority((log4cpp::Priority::PriorityLevel)log_level);
    log4cpp::Category::getInstance(m_logcat).notice("log start");
    return 0;
}

//***********************************************************************************************//
int spdz2_ext_processor_base::load_inputs()
{
	std::list<std::string> party_input_specs;
	if(0 != load_party_input_specs(party_input_specs))
	{
		LC(m_logcat).error("%s: failed loading the party input specs", __FUNCTION__);
		return -1;
	}
	LC(m_logcat).debug("%s: party input specs loaded.", __FUNCTION__);

	for(std::list<std::string>::const_iterator i = party_input_specs.begin(); i != party_input_specs.end(); ++i)
	{
		if(0 != load_party_inputs(*i))
		{
			LC(m_logcat).error("%s: failed loading party input by spec [%s]", __FUNCTION__, i->c_str());
			return -1;
		}
	}

	return 0;
}

//***********************************************************************************************//
int spdz2_ext_processor_base::load_party_input_specs(std::list<std::string> & party_input_specs)
{
	static const char common_inputs_spec[] = "parties_inputs.txt";

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
		LC(m_logcat).error("%s: failed to open [%s].", __FUNCTION__, common_inputs_spec);
		return -1;
	}
	return (party_input_specs.empty())? -1: 0;
}

//***********************************************************************************************//
int spdz2_ext_processor_base::load_party_inputs(const std::string & party_input_spec)
{
	LC(m_logcat).debug("%s: party_input_spec = [%s].", __FUNCTION__, party_input_spec.c_str());
	std::string::size_type pos = party_input_spec.find(',');
	if(std::string::npos != pos)
	{
		int pid = (int)strtol(party_input_spec.substr(0, pos).c_str(), NULL, 10);
		size_t count = (size_t)strtol(party_input_spec.substr(pos+1).c_str(), NULL, 10);
		LC(m_logcat).debug("%s: party_input_spec: id=%d; count=%lu;", __FUNCTION__, pid, count);
		if(0 < count)
			return load_party_inputs(pid, count);
		else
			return 0;
	}
	else
	{
		LC(m_logcat).error("%s: invalid party input spec format [%s]", __FUNCTION__, party_input_spec.c_str());
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
		LC(m_logcat).error("%s: failed loading clear inputs.", __FUNCTION__);
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
			LC(m_logcat).error("%s: failed to map-insert shared inputs for pid=%d.", __FUNCTION__, pid);
		}
	}
	else
	{
		for(size_t i = 0; i < count; ++i)
			mpz_clear(party_inputs.shared_values[i]);
		delete party_inputs.shared_values;
		LC(m_logcat).error("%s: protocol_share() failed for pid=%d.", __FUNCTION__, pid);
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
			LC(m_logcat).error("%s: not enough inputs in file [%s]; required %lu; file has %lu;",
					__FUNCTION__, input_file.c_str(), count, clr_values_idx);
		}
	}
	return (NULL != (*clr_values))? 0: -1;
}

//***********************************************************************************************//

int spdz2_ext_processor_base::input(const int input_of_pid, mpz_t input_value)
{
	std::map< int , shared_input_t >::iterator i = m_shared_inputs.find(input_of_pid);

	if(m_shared_inputs.end() != i && 0 < i->second.share_count && i->second.share_index < i->second.share_count)
	{
		mpz_set(input_value, i->second.shared_values[i->second.share_index++]);
		return 0;
	}
	LC(m_logcat).error("%s: failed to get input for pid %d.", __FUNCTION__, input_of_pid);
	return -1;
}

//***********************************************************************************************//

int spdz2_ext_processor_base::input(const int input_of_pid, const size_t num_of_inputs, mpz_t * inputs)
{
	int result = -1;
	std::map< int , shared_input_t >::iterator i = m_shared_inputs.find(input_of_pid);
	if(m_shared_inputs.end() != i)
	{
		if((i->second.share_count - i->second.share_index) >= num_of_inputs)
		{
			for(size_t j = 0; j < num_of_inputs; ++j)
			{
				mpz_set(inputs[j], i->second.shared_values[i->second.share_index++]);
			}
			result = 0;
		}
		else
		{
			LC(m_logcat).error("%s: not enough input for pid %d; required %lu; available %lu;",
					__FUNCTION__, input_of_pid, num_of_inputs, (i->second.share_count - i->second.share_index));
		}
	}
	else
	{
		LC(m_logcat).error("%s: failed to get input for pid %d.", __FUNCTION__, input_of_pid);
	}
	return result;
}

//***********************************************************************************************//


