
#include "spdzext.h"
#include "spdz2_ext_processor_base.h"
#include "spdz2_ext_processor_mersenne61.h"
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
	else
	{
		syslog(LOG_ERR, "SPDZ-2 extension library init: invalid field type [%s]", field);
		return -1;
	}

	if(0 != proc->init(pid, num_of_parties, thread_id, field, open_count, mult_count, bits_count, 700))
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
int opens(void * handle, const size_t share_count, const mp_limb_t * shares, mp_limb_t * opens, int verify)
{
	return ((spdz2_ext_processor_base *)handle)->open(share_count, shares, opens, verify);
}
//-------------------------------------------------------------------------------------------//
int triple(void * handle, mp_limb_t * a, mp_limb_t * b, mp_limb_t * c)
{
	return ((spdz2_ext_processor_base *)handle)->triple(a, b, c);
}
//-------------------------------------------------------------------------------------------//
int verify(void * handle, int * error)
{
	return ((spdz2_ext_processor_base *)handle)->verify(error);
}
//-------------------------------------------------------------------------------------------//
int input(void * handle, const int input_of_pid, const size_t num_of_inputs, mpz_t * inputs)
{
	return ((spdz2_ext_processor_base *)handle)->input(input_of_pid, num_of_inputs, inputs);
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
int mix_mul(void * handle, mpz_t share, const mpz_t scalar)
{
	return ((spdz2_ext_processor_base *)handle)->mix_mul(share, scalar);
}
//-------------------------------------------------------------------------------------------//
int adds(void * handle, mpz_t share1, const mpz_t share2)
{
	return ((spdz2_ext_processor_base *)handle)->adds(share1, share2);
}
//-------------------------------------------------------------------------------------------//
int subs(void * handle, mpz_t share1, const mpz_t share2)
{
	syslog(LOG_ERR, "SPDZ-2 extension library subs in use.");
	return ((spdz2_ext_processor_base *)handle)->subs(share1, share2);
}
//-------------------------------------------------------------------------------------------//
int share_immediates(void * handle, const int party_id, const size_t value_count, const mpz_t * values, mpz_t * shares)
{
	return ((spdz2_ext_processor_base *)handle)->share_immediates(party_id, value_count, values, shares);
}
//-------------------------------------------------------------------------------------------//
int bit(void * handle, mp_limb_t * share)
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
