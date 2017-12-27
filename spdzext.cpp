
#include "spdzext.h"
#include "spdz2_ext_processor_base.h"
#include "spdz2_ext_processor_mersenne61.h"
#include "spdz2_ext_processor_gf2n.h"

#include <syslog.h>

//***********************************************************************************************//

//-------------------------------------------------------------------------------------------//
int init(void ** handle, const int pid, const int num_of_parties, const char * field, const int offline_size)
{
	spdz2_ext_processor_base * proc = NULL;
	if(strncmp(field, "gfp", 3) == 0)
	{
		proc = new spdz2_ext_processor_mersenne61;
	}
	else if(strncmp(field, "gf2n", 4) == 0)
	{
		long numbits = strtol(field + 4, NULL, 10);
		proc = new spdz2_ext_processor_gf2n(numbits);
	}
	else
	{
		syslog(LOG_ERR, "SPDZ-2 extension library init: invalid field type [%s]", field);
		return -1;
	}

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
	spdz2_ext_processor_base * proc = ((spdz2_ext_processor_base *)handle);
	proc->stop(20);
	delete proc;
	return 0;
}
//-------------------------------------------------------------------------------------------//
int offline(void * handle, const int offline_size)
{
	return ((spdz2_ext_processor_base *)handle)->offline(offline_size);
}
//-------------------------------------------------------------------------------------------//
int start_open(void * handle, const size_t share_count, const u_int64_t * shares, int verify)
{
	return ((spdz2_ext_processor_base *)handle)->start_open(share_count, shares, verify);
}
//-------------------------------------------------------------------------------------------//
int stop_open(void * handle, size_t * open_count, u_int64_t ** opens)
{
	return ((spdz2_ext_processor_base *)handle)->stop_open(open_count, opens, 20);
}
//-------------------------------------------------------------------------------------------//
int triple(void * handle, u_int64_t * a, u_int64_t * b, u_int64_t * c)
{
	return ((spdz2_ext_processor_base *)handle)->triple(a, b, c, 20);
}
//-------------------------------------------------------------------------------------------//
int input(void * handle, const int input_of_pid, u_int64_t * input_value)
{
	return ((spdz2_ext_processor_base *)handle)->input(input_of_pid, input_value);
}
//-------------------------------------------------------------------------------------------//
int start_verify(void * handle, int * error)
{
	return ((spdz2_ext_processor_base *)handle)->start_verify(error);
}
//-------------------------------------------------------------------------------------------//
int stop_verify(void * handle)
{
	return ((spdz2_ext_processor_base *)handle)->stop_verify();
}
//-------------------------------------------------------------------------------------------//
int start_input(void * handle, const int input_of_pid, const size_t num_of_inputs)
{
	return ((spdz2_ext_processor_base *)handle)->start_input(input_of_pid, num_of_inputs);
}
//-------------------------------------------------------------------------------------------//
int stop_input(void * handle, size_t * input_count, u_int64_t ** inputs)
{
	return ((spdz2_ext_processor_base *)handle)->stop_input(input_count, inputs);
}
//-------------------------------------------------------------------------------------------//
int start_mult(void * handle, const size_t share_count, const u_int64_t * shares, int verify)
{
	return ((spdz2_ext_processor_base *)handle)->start_mult(share_count, shares, verify);
}
//-------------------------------------------------------------------------------------------//
int stop_mult(void * handle, size_t * product_count, u_int64_t ** products)
{
	return ((spdz2_ext_processor_base *)handle)->stop_mult(product_count, products);
}
//-------------------------------------------------------------------------------------------//
int mix_add(void * handle, u_int64_t * share, u_int64_t scalar)
{
	return ((spdz2_ext_processor_base *)handle)->mix_add(share, scalar);
}
//-------------------------------------------------------------------------------------------//
int mix_sub_scalar(void * handle, u_int64_t * share, u_int64_t scalar)
{
	return ((spdz2_ext_processor_base *)handle)->mix_sub_scalar(share, scalar);
}
//-------------------------------------------------------------------------------------------//
int mix_sub_share(void * handle, u_int64_t scalar, u_int64_t * share)
{
	return ((spdz2_ext_processor_base *)handle)->mix_sub_share(scalar, share);
}
//-------------------------------------------------------------------------------------------//
int start_share_immediates(void * handle, const int input_of_pid, const size_t value_count, const u_int64_t * values)
{
	return ((spdz2_ext_processor_base *)handle)->start_share_immediates(input_of_pid, value_count, values);
}
//-------------------------------------------------------------------------------------------//
int stop_share_immediates(void * handle, size_t * share_count, u_int64_t ** shares)
{
	return ((spdz2_ext_processor_base *)handle)->stop_share_immediates(share_count, shares);
}
//-------------------------------------------------------------------------------------------//
int share_immediate(void * handle, const u_int64_t value, u_int64_t * share)
{
	return ((spdz2_ext_processor_base *)handle)->share_immediate(value, share);
}
//-------------------------------------------------------------------------------------------//
int bit(void * handle, u_int64_t * share)
{
	return ((spdz2_ext_processor_base *)handle)->bit(share);
}
//-------------------------------------------------------------------------------------------//
int inverse(void * handle, u_int64_t * share_value, u_int64_t * share_inverse)
{
	return ((spdz2_ext_processor_base *)handle)->inverse(share_value, share_inverse);
}
//-------------------------------------------------------------------------------------------//
u_int64_t gfp_conversion(const u_int64_t value)
{
	ZpMersenneLongElement element(value);
	return element.elem;
}
//-------------------------------------------------------------------------------------------//
u_int64_t gfp_add(u_int64_t v1, u_int64_t v2)
{
	ZpMersenneLongElement e1(v1), e2(v2);
	return (e1 + e2).elem;
}
//-------------------------------------------------------------------------------------------//
u_int64_t gfp_sub(u_int64_t v1, u_int64_t v2)
{
	ZpMersenneLongElement e1(v1), e2(v2);
	return (e1 - e2).elem;
}
//-------------------------------------------------------------------------------------------//
u_int64_t gfp_mult(u_int64_t v1, u_int64_t v2)
{
	ZpMersenneLongElement e1(v1), e2(v2);
	return (e1 * e2).elem;
}
//-------------------------------------------------------------------------------------------//
u_int64_t gf2n_conversion(const u_int64_t value)
{
	GF2E element = to_GF2E(value);
	u_int64_t result;
	conv(element, result);
	return result;
}
//-------------------------------------------------------------------------------------------//
u_int64_t gf2n_add(u_int64_t v1_, u_int64_t v2_)
{
	u_int64_t result;
	GF2E v1 = to_GF2E(v1_), v2 = to_GF2E(v2_), sum;
	add(sum, v1, v2);
	conv(sum, result);
	return result;
}
//-------------------------------------------------------------------------------------------//
u_int64_t gf2n_sub(u_int64_t v1_, u_int64_t v2_)
{
	u_int64_t result;
	GF2E v1 = to_GF2E(v1_), v2 = to_GF2E(v2_), diff;
	sub(diff, v1, v2);
	conv(diff, result);
	return result;
}
//-------------------------------------------------------------------------------------------//
u_int64_t gf2n_mult(u_int64_t v1_, u_int64_t v2_)
{
	u_int64_t result;
	GF2E v1 = to_GF2E(v1_), v2 = to_GF2E(v2_), product;
	mul(product, v1, v2);
	conv(product, result);
	return result;
}
//-------------------------------------------------------------------------------------------//
