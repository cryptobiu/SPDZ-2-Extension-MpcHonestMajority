
#include "spdzext.h"
#include <syslog.h>
#include <vector>

#include "ProtocolParty.h"

ProtocolParty<ZpMersenneLongElement> * the_party = NULL;
int the_pid = -1;

int init(const int pid, const char * /*field*/, const int offline_size)
{
	the_pid = pid;

	/* The field in this example is Mersenne61 */

	the_party = new ProtocolParty<ZpMersenneLongElement>(pid, offline_size);
	the_party->init();

	syslog(LOG_NOTICE, "Player %d SPDZ-2 extension library initialized", pid);
	return 0;
}

int start_open(const size_t share_count, const char ** shares, size_t * open_count, char *** opens)
{
	syslog(LOG_NOTICE, "Player %d SPDZ-2 extension library start_open for %u shares", the_pid, (unsigned int)share_count);

	std::vector<ZpMersenneLongElement> ext_shares, ext_opens;

	char * end_ptr = NULL;
	for(size_t i = 0; i < share_count; i++)
	{
		syslog(LOG_NOTICE, "Player %d SPDZ-2 extension library start_open textual share[%u] = [%s]", the_pid, (unsigned int)i, shares[i]);
		unsigned long share_value = strtol(shares[i], &end_ptr, 10);
		syslog(LOG_NOTICE, "Player %d SPDZ-2 extension library start_open binary share[%u] = [%lu]", the_pid, (unsigned int)i, share_value);
		ext_shares.push_back(ZpMersenneLongElement(share_value));
	}

	syslog(LOG_NOTICE, "Player %d SPDZ-2 extension library start_open prep %u shares", the_pid, (unsigned int)ext_shares.size());

	the_party->openShare((int)ext_shares.size(), ext_shares, ext_opens);

	syslog(LOG_NOTICE, "Player %d SPDZ-2 extension library start_open returned %u opens", the_pid, (unsigned int)ext_opens.size());

	*open_count = ext_opens.size();
	if(0 < *open_count)
	{
		*opens = new char*[*open_count];
		memset(*opens, 0, *open_count * sizeof(char*));

		*open_count = 0;
		for(vector<ZpMersenneLongElement>::const_iterator i = ext_opens.begin(); i != ext_opens.end(); ++i)
		{
			std::stringstream ss;
			ss << *i;
			(*opens)[(*open_count)++] = strdup(ss.str().c_str());
		}
	}

	return 0;
}

int stop_open(const size_t /*share_count*/, const char ** /*shares*/, const size_t /*open_count*/, const char ** /*opens*/)
{
	//the current implementation does not use any of the params provided.
	if(the_party->verify())
	{
		syslog(LOG_NOTICE, "Player %d SPDZ-2 extension library verify() succeeded", the_pid);
		return 0;
	}
	else
	{
		syslog(LOG_NOTICE, "Player %d SPDZ-2 extension library verify() failed", the_pid);
		return -1;
	}
}

int term(void *)
{
	delete the_party;
	the_party = NULL;

	syslog(LOG_NOTICE, "Player %d SPDZ-2 extension library terminated", the_pid);
	return 0;
}
