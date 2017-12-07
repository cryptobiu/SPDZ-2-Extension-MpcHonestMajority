
#include "spdzext.h"
#include <iostream>
#include <vector>

#include "spdz_ext_processor.h"
#include "ZpMersenneLongElement.h"

//-------------------------------------------------------------------------------------------//
int init(void ** handle, const int pid, const int num_of_parties, const char * field, const int offline_size)
{
	spdz_ext_processor_ifc * proc = new spdz_ext_processor_ifc;
	if(0 != proc->start(pid, num_of_parties, field, offline_size))
	{
		delete proc;
		return -1;
	}
	*handle = proc;
	return 0;
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
int offline(void * handle, const int offline_size)
{
	return ((spdz_ext_processor_ifc *)handle)->offline(offline_size);
}
//-------------------------------------------------------------------------------------------//
int start_open(void * handle, const size_t share_count, const u_int64_t * shares, int verify)
{
	return ((spdz_ext_processor_ifc *)handle)->start_open(share_count, shares, verify);
}
//-------------------------------------------------------------------------------------------//
int stop_open(void * handle, size_t * open_count, u_int64_t ** opens)
{
	return ((spdz_ext_processor_ifc *)handle)->stop_open(open_count, opens, 20);
}
//-------------------------------------------------------------------------------------------//
int triple(void * handle, u_int64_t * a, u_int64_t * b, u_int64_t * c)
{
	return ((spdz_ext_processor_ifc *)handle)->triple(a, b, c, 20);
}
//-------------------------------------------------------------------------------------------//
int input(void * handle, const int input_of_pid, u_int64_t * input_value)
{
	return ((spdz_ext_processor_ifc *)handle)->input(input_of_pid, input_value);
}
//-------------------------------------------------------------------------------------------//
int start_verify(void * handle, int * error)
{
	return ((spdz_ext_processor_ifc *)handle)->start_verify(error);
}
//-------------------------------------------------------------------------------------------//
int stop_verify(void * handle)
{
	return ((spdz_ext_processor_ifc *)handle)->stop_verify();
}
//-------------------------------------------------------------------------------------------//
int start_input(void * handle, const int input_of_pid, const size_t num_of_inputs)
{
	return ((spdz_ext_processor_ifc *)handle)->start_input(input_of_pid, num_of_inputs);
}
//-------------------------------------------------------------------------------------------//
int stop_input(void * handle, size_t * input_count, u_int64_t ** inputs)
{
	return ((spdz_ext_processor_ifc *)handle)->stop_input(input_count, inputs);
}
//-------------------------------------------------------------------------------------------//
int start_mult(void * handle, const size_t share_count, const u_int64_t * shares, int verify)
{
	return ((spdz_ext_processor_ifc *)handle)->start_mult(share_count, shares, verify);
}
//-------------------------------------------------------------------------------------------//
int stop_mult(void * handle, size_t * product_count, u_int64_t ** products)
{
	return ((spdz_ext_processor_ifc *)handle)->stop_mult(product_count, products);
}
//-------------------------------------------------------------------------------------------//
u_int64_t test_conversion(const u_int64_t value)
{
	ZpMersenneLongElement element(value);
	return element.elem;
}
//-------------------------------------------------------------------------------------------//
u_int64_t add(u_int64_t v1, u_int64_t v2)
{
	ZpMersenneLongElement e1(v1), e2(v2);
	return (e1 + e2).elem;
}
//-------------------------------------------------------------------------------------------//
u_int64_t sub(u_int64_t v1, u_int64_t v2)
{
	ZpMersenneLongElement e1(v1), e2(v2);
	return (e1 - e2).elem;
}
//-------------------------------------------------------------------------------------------//
u_int64_t mult(u_int64_t v1, u_int64_t v2)
{
	ZpMersenneLongElement e1(v1), e2(v2);
	return (e1 * e2).elem;
}
//-------------------------------------------------------------------------------------------//
