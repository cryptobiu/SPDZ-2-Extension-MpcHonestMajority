
#include "spdzext.h"
//#include <syslog.h>
#include <iostream>
#include <vector>

#include "ProtocolParty.h"

void load_mersenne_elements(const size_t ul_count, const unsigned long * uls, std::vector<ZpMersenneLongElement> & elements);
void store_mersenne_elements(const std::vector<ZpMersenneLongElement> & elements, size_t * ul_count, unsigned long ** uls);

//-------------------------------------------------------------------------------------------//
ProtocolParty<ZpMersenneLongElement> * the_party = NULL;
int the_pid = -1;

int init(const int pid, const char * /*field*/, const int offline_size)
{
	the_pid = pid;

	/* The field in this example is Mersenne61 */

	the_party = new ProtocolParty<ZpMersenneLongElement>(pid, offline_size);
	the_party->init();

	std::cout << "Player " << pid << " SPDZ-2 extension library initialized" << std::endl;
	return 0;
}
//-------------------------------------------------------------------------------------------//
int start_open(const size_t share_count, const unsigned long * shares, size_t * open_count, unsigned long ** opens)
{
	std::cout << "Player " << the_pid << " SPDZ-2 extension library start_open for " << share_count << " shares" << std::endl;

	if(0 < share_count)
	{
		/*
		for(size_t i = 0; i < share_count; i++)
		{
			cout << "start_open() share[" << i << "] = " << shares[i] << endl;
		}
		*/
		std::vector<ZpMersenneLongElement> ext_shares, ext_opens;
		load_mersenne_elements(share_count, shares, ext_shares);

		//std::cout << "Player " << the_pid << " SPDZ-2 extension library start_open prep " << ext_shares.size() << " shares" << std::endl;

		ext_opens.resize(ext_shares.size());
		the_party->openShare((int)ext_shares.size(), ext_shares, ext_opens);

		//std::cout << "Player " << the_pid << " SPDZ-2 extension library start_open returned " << ext_opens.size() << " opens" << std::endl;

		if(0 < ext_opens.size())
		{
			store_mersenne_elements(ext_opens, open_count, opens);
			/*
			for(size_t i = 0; i < share_count; i++)
			{
				cout << "start_open() open[" << (unsigned long)i << "] = " << (*opens)[i] << "/" << ext_opens[i] << endl;
			}
			*/
		}
	}
	return 0;
}
//-------------------------------------------------------------------------------------------//
int stop_open()
{
	if(the_party->verify())
	{
		std::cout << "Player " << the_pid << " SPDZ-2 extension library verify() succeeded" << std::endl;
		return 0;
	}
	else
	{
		std::cout << "Player " << the_pid << " SPDZ-2 extension library verify() failed" << std::endl;
		return -1;
	}
}
//-------------------------------------------------------------------------------------------//
int term(void *)
{
	if(NULL != the_party)
	{
		delete the_party;
		the_party = NULL;
	}

	std::cout << "Player " << the_pid << " SPDZ-2 extension library terminated" << std::endl;
	return 0;
}
//-------------------------------------------------------------------------------------------//
unsigned long test_conversion(const unsigned long value)
{
	ZpMersenneLongElement element(value);
	return element.elem;
}
//-------------------------------------------------------------------------------------------//
void load_mersenne_elements(const size_t ul_count, const unsigned long * uls, std::vector<ZpMersenneLongElement> & elements)
{
	elements.clear();
	for(size_t i = 0; i < ul_count; i++)
	{
		elements.push_back(ZpMersenneLongElement(uls[i]));
	}
}
//-------------------------------------------------------------------------------------------//
void store_mersenne_elements(const std::vector<ZpMersenneLongElement> & elements, size_t * ul_count, unsigned long ** uls)
{
	*uls = new unsigned long[*ul_count = elements.size()];
	for(size_t i = 0; i < *ul_count; i++)
	{
		(*uls)[i] = elements[i].elem;
	}

}
//-------------------------------------------------------------------------------------------//
