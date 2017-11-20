
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

int start_open(const size_t share_count, const char ** shares, char *** secrets)
{
	syslog(LOG_NOTICE, "Player %d SPDZ-2 extension library start_open for %u shares", the_pid, (unsigned int)share_count);

	std::vector<ZpMersenneLongElement> ext_shares, ext_secrets;

	char * end_ptr = NULL;
	for(size_t i = 0; i < share_count; i++)
	{
		syslog(LOG_NOTICE, "Player %d SPDZ-2 extension library start_open textual share[%u] = [%s]", the_pid, (unsigned int)i, shares[i]);
		unsigned long share_value = strtol(shares[i], &end_ptr, 10);
		syslog(LOG_NOTICE, "Player %d SPDZ-2 extension library start_open binary share[%u] = [%lu]", the_pid, (unsigned int)i, share_value);
		ext_shares.push_back(ZpMersenneLongElement(share_value));
	}

	syslog(LOG_NOTICE, "Player %d SPDZ-2 extension library start_open prep %u shares", the_pid, (unsigned int)ext_shares.size());

	the_party->openShare((int)ext_shares.size(), ext_shares, ext_secrets);
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
