
#include "spdzext.h"
#include <iostream>
#include <vector>

#include "spdz_ext_processor.h"
#include "ProtocolParty.h"

//-------------------------------------------------------------------------------------------//
int init(void ** handle, const int pid, const char * field, const int offline_size)
{
	spdz_ext_processor_ifc * proc = new spdz_ext_processor_ifc;
	if(0 != proc->start(pid, field, offline_size))
	{
		delete proc;
		return -1;
	}
	*handle = proc;
	return 0;
}
//-------------------------------------------------------------------------------------------//
int start_open(void * handle, const size_t share_count, const unsigned long * shares)
{
	return ((spdz_ext_processor_ifc *)handle)->start_open(share_count, shares);
}
//-------------------------------------------------------------------------------------------//
int stop_open(void * handle, size_t * open_count, unsigned long ** opens)
{
	return ((spdz_ext_processor_ifc *)handle)->stop_open(open_count, opens, 20);
}
//-------------------------------------------------------------------------------------------//
int triple(void * handle, unsigned long * a, unsigned long * b, unsigned long * c)
{
	return ((spdz_ext_processor_ifc *)handle)->triple(a, b, c, 20);
}
//-------------------------------------------------------------------------------------------//
int term(void * handle)
{
	spdz_ext_processor_ifc * proc = ((spdz_ext_processor_ifc *)handle);
	proc->stop(20);
	delete proc;
	return 0;
}
//-------------------------------------------------------------------------------------------//
unsigned long test_conversion(const unsigned long value)
{
	ZpMersenneLongElement element(value);
	return element.elem;
}
//-------------------------------------------------------------------------------------------//
