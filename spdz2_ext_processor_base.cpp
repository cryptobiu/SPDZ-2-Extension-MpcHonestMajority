
#include "spdz2_ext_processor_base.h"

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>

#include <string>
#include <sstream>
#include <iomanip>

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

	std::string log_file = "/var/log/spdz/";
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
	static const char spaces[] = " \t\n\v\f\r";
	static const char common_inputs_spec[] = "parties_inputs.txt";

	party_input_specs.clear();
	FILE * pf = fopen(common_inputs_spec, "r");
	if(NULL != pf)
	{
		char sz[128];
		while(NULL != fgets(sz, 128, pf))
		{
			std::string str = sz;
			if(str.find('#') != std::string::npos)
				continue;
			for(std::string::size_type n = str.find_first_of(spaces); n != std::string::npos; n = str.find_first_of(spaces, n))
				str.erase(n, 1);
			if(str.empty())
				continue;
			party_input_specs.push_back(str);
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
	mp_limb_t * clr_values = new mp_limb_t[count * 2];
	memset(clr_values, 0, count * 2 * sizeof(mp_limb_t));
	if(0 == load_clr_party_inputs(clr_values, count))
		result = load_peer_party_inputs(m_pid, count, clr_values);
	else
		LC(m_logcat).error("%s: failed loading clear inputs.", __FUNCTION__);
	delete clr_values;
	return result;
}

//***********************************************************************************************//
int spdz2_ext_processor_base::load_peer_party_inputs(const int pid, const size_t count, const mp_limb_t * clr_values)
{
	int result = -1;
	shared_input_t party_inputs;
	party_inputs.share_count = count;
	party_inputs.share_index = 0;
	party_inputs.shared_values = new mp_limb_t[party_inputs.share_count * 4];
	memset(party_inputs.shared_values, 0, party_inputs.share_count * 4 * sizeof(mp_limb_t));

	if(0 == closes(pid, count, (m_pid == pid)? clr_values: NULL, party_inputs.shared_values))
	{
		if(m_shared_inputs.insert(std::pair<int, shared_input_t>(pid, party_inputs)).second)
			result = 0;
		else
		{
			delete party_inputs.shared_values;
			LC(m_logcat).error("%s: failed to map-insert shared inputs for pid=%d.", __FUNCTION__, pid);
		}
	}
	else
	{
		delete party_inputs.shared_values;
		LC(m_logcat).error("%s: share_immediates() failed for pid=%d.", __FUNCTION__, pid);
	}
	return result;
}

//***********************************************************************************************//
int spdz2_ext_processor_base::load_clr_party_inputs(mp_limb_t * clr_values, const size_t count)
{
	int result = -1;
	char sz[128];
	snprintf(sz, 128, "party_%d_input.txt", m_pid);
	std::string input_file = sz;

	FILE * pf = fopen(input_file.c_str(), "r");
	if(NULL != pf)
	{
		size_t clr_values_idx = 0;
		mpz_t t;
		mpz_init(t);
		while(NULL != fgets(sz, 128, pf))
		{
			LC(m_logcat).debug("%s: party input [%s] line: [%s];", __FUNCTION__, input_file.c_str(), sz);
			mpz_set_str(t, sz, 10);
			mpz_export(clr_values + 2*clr_values_idx, NULL, -1, 8, 0, 0, t);
			if(++clr_values_idx >= count)
				break;
		}
		mpz_clear(t);
		fclose(pf);

		if(count == clr_values_idx)
			result = 0;
		else
			LC(m_logcat).error("%s: not enough inputs in file [%s]; required %lu; file has %lu;",
							   __FUNCTION__, input_file.c_str(), count, clr_values_idx);
	}
	return result;
}

//***********************************************************************************************//

int spdz2_ext_processor_base::input(const int input_of_pid, mp_limb_t * input_value)
{
	std::map< int , shared_input_t >::iterator i = m_shared_inputs.find(input_of_pid);
	if(m_shared_inputs.end() != i && 0 < i->second.share_count && i->second.share_index < i->second.share_count)
	{
		memcpy(input_value, i->second.shared_values + (4 * i->second.share_index++), 4 * sizeof(mp_limb_t));
		return 0;
	}
	LC(m_logcat).error("%s: failed to get input for pid %d.", __FUNCTION__, input_of_pid);
	return -1;
}

//***********************************************************************************************//

int spdz2_ext_processor_base::input(const int input_of_pid, const size_t num_of_inputs, mp_limb_t * inputs)
{
	LC(m_logcat).debug("%s: %lu input of party %d requested.", __FUNCTION__, num_of_inputs, input_of_pid);

	int result = -1;
	std::map< int , shared_input_t >::iterator i = m_shared_inputs.find(input_of_pid);
	if(m_shared_inputs.end() != i)
	{
		if(i->second.share_count >= (i->second.share_index + num_of_inputs))
		{
			memcpy(inputs, i->second.shared_values + (4 * i->second.share_index), num_of_inputs * 4 * sizeof(mp_limb_t));
			i->second.share_index += num_of_inputs;
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

int spdz2_ext_processor_base::inverse(mp_limb_t * x, mp_limb_t * y)
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
	memset(x, 0, 4*sizeof(mp_limb_t));
	memset(y, 0, 4*sizeof(mp_limb_t));

	mp_limb_t r[4], u[4];
	memset(r, 0, 4*sizeof(mp_limb_t));
	memset(u, 0, 4*sizeof(mp_limb_t));
	if(0 != triple(x, r, u))
	{
		LC(m_logcat).error("%s: protocol triple() failed", __FUNCTION__);
		return -1;
	}

	mp_limb_t open_u[2];
	memset(open_u, 0, 2*sizeof(mp_limb_t));
	if(0 != open(1, u, open_u, 1))
	{
		LC(m_logcat).error("%s: protocol open() failed", __FUNCTION__);
		return -1;
	}

	mp_limb_t v[2];
	memset(v, 0, 2*sizeof(mp_limb_t));
	if(0 != inverse_value(open_u, v))
	{
		LC(m_logcat).error("%s: inverse_value() failed", __FUNCTION__);
		return -1;
	}

	if(0 != mix_mul(r, v, y))
	{
		LC(m_logcat).error("%s: protocol mix_mul() failed", __FUNCTION__);
		return -1;
	}

	return 0;
}

//***********************************************************************************************//

int spdz2_ext_processor_base::inverse_value(const mp_limb_t * value, mp_limb_t * inverse)
{
	mpz_t gcd, x, y, P, _value, _inverse;

	mpz_init(gcd);
	mpz_init(x);
	mpz_init(y);
	mpz_init(P);
	mpz_init(_value);
	mpz_init(_inverse);

	mpz_import(_value, 2, -1, 8, 0, 0, value);
	get_P(P);
	mpz_gcdext(gcd, x, y, _value, P);
	mpz_mod(_inverse, x, P);
	mpz_export(inverse, NULL, -1, 8, 0, 0, _inverse);

	mpz_clear(gcd);
	mpz_clear(x);
	mpz_clear(y);
	mpz_clear(P);
	mpz_clear(_value);
	mpz_clear(_inverse);

	return 0;
}

//***********************************************************************************************//


