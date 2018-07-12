
#include "spdzext.h"
#include "spdz2_ext_processor_base.h"
#include "spdz2_ext_processor_mersenne61.h"
#include "spdz2_ext_processor_gf2n.h"
#include "spdz2_ext_processor_mersenne127.h"

#include <syslog.h>

//***********************************************************************************************//

//-------------------------------------------------------------------------------------------//
int init(void ** handle, const int pid, const int num_of_parties, const int thread_id,
		 const char * field, const int open_count, const int mult_count, const int bits_count)
{
	spdz2_ext_processor_base * proc = NULL;
	if(strncmp(field, "gfp", 3) == 0)
	{
		long numbits = strtol(field + 3, NULL, 10);
		if(61 == numbits)
		{
			proc = new spdz2_ext_processor_mersenne61;
		}
		else if(127 == numbits)
		{
			proc = new spdz2_ext_processor_mersenne127;
		}
		else
		{
			syslog(LOG_ERR, "SPDZ-2 extension library init: invalid GFP number of bits [%ld]", numbits);
			return -1;
		}
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

	if(0 != proc->init(pid, num_of_parties, thread_id, field, open_count, mult_count, bits_count))
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
	proc->term();
	delete proc;
	return 0;
}
//-------------------------------------------------------------------------------------------//
int offline(void * handle, const int offline_size)
{
	return ((spdz2_ext_processor_base *)handle)->offline(offline_size);
}
//-------------------------------------------------------------------------------------------//
int opens(void * handle, const size_t share_count, const mpz_t * shares, mpz_t * opens, int verify)
{
	return ((spdz2_ext_processor_base *)handle)->open(share_count, shares, opens, verify);
}
//-------------------------------------------------------------------------------------------//
int triple(void * handle, mpz_t a, mpz_t b, mpz_t c)
{
	return ((spdz2_ext_processor_base *)handle)->triple(a, b, c);
}
//-------------------------------------------------------------------------------------------//
int verify(void * handle, int * error)
{
	return ((spdz2_ext_processor_base *)handle)->verify(error);
}
//-------------------------------------------------------------------------------------------//
int input(void * handle, const int input_of_pid, mpz_t input_value)
{
	return ((spdz2_ext_processor_base *)handle)->input(input_of_pid, input_value);
}
//-------------------------------------------------------------------------------------------//
int mult(void * handle, const size_t share_count, const mpz_t * shares, mpz_t * products, int verify)
{
	return ((spdz2_ext_processor_base *)handle)->mult(share_count, shares, products, verify);
}
//-------------------------------------------------------------------------------------------//
int mix_add(void * handle, mpz_t share, const mpz_t scalar)
{
	return ((spdz2_ext_processor_base *)handle)->mix_add(share, scalar);
}
//-------------------------------------------------------------------------------------------//
int mix_sub_scalar(void * handle, mpz_t share, const mpz_t scalar)
{
	return ((spdz2_ext_processor_base *)handle)->mix_sub_scalar(share, scalar);
}
//-------------------------------------------------------------------------------------------//
int mix_sub_share(void * handle, const mpz_t scalar, mpz_t share)
{
	return ((spdz2_ext_processor_base *)handle)->mix_sub_share(scalar, share);
}
//-------------------------------------------------------------------------------------------//
int share_immediates(void * handle, const int party_id, const size_t value_count, const mpz_t * values, mpz_t * shares)
{
	return ((spdz2_ext_processor_base *)handle)->share_immediates(party_id, value_count, values, shares);
}
//-------------------------------------------------------------------------------------------//
int bit(void * handle, mpz_t share)
{
	return ((spdz2_ext_processor_base *)handle)->bit(share);
}
//-------------------------------------------------------------------------------------------//
int inverse(void * handle, mpz_t share_value, mpz_t share_inverse)
{
	return ((spdz2_ext_processor_base *)handle)->inverse(share_value, share_inverse);
}
//-------------------------------------------------------------------------------------------//
/*
mpz_t gfp_conversion(const mpz_t value)
{
	ZpMersenneLongElement element(value);
	return element.elem;
}
//-------------------------------------------------------------------------------------------//
mpz_t gfp_add(mpz_t v1, mpz_t v2)
{
	ZpMersenneLongElement e1(v1), e2(v2);
	return (e1 + e2).elem;
}
//-------------------------------------------------------------------------------------------//
mpz_t gfp_sub(mpz_t v1, mpz_t v2)
{
	ZpMersenneLongElement e1(v1), e2(v2);
	return (e1 - e2).elem;
}
//-------------------------------------------------------------------------------------------//
mpz_t gfp_mult(mpz_t v1, mpz_t v2)
{
	ZpMersenneLongElement e1(v1), e2(v2);
	return (e1 * e2).elem;
}
//-------------------------------------------------------------------------------------------//
mpz_t gf2n_conversion(const mpz_t value)
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
*/
