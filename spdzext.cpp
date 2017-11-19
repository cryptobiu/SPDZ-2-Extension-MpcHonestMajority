
#include "spdzext.h"
#include <syslog.h>
#include <map>

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

int start_open(const size_t share_count, const char ** shares)
{
	syslog(LOG_NOTICE, "Player %d SPDZ-2 extension library start_open for %u shares", the_pid, (unsigned int)share_count);
	for(size_t i = 0; i < share_count; i++)
	{
		syslog(LOG_NOTICE, "Player %d SPDZ-2 extension library start_open share[%u] = [%s]", the_pid, (unsigned int)i, shares[i]);
	}
	return 0;
}

int stop_open(void *)
{
	return 0;
}

int term(void *)
{
	delete the_party;
	the_party = NULL;

	syslog(LOG_NOTICE, "Player %d SPDZ-2 extension library terminated", the_pid);
	return 0;
}
