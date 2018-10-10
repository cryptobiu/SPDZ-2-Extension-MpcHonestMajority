
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
int input(void * handle, const int input_of_pid, const size_t num_of_inputs, mp_limb_t * inputs)
{
	return ((spdz2_ext_processor_base *)handle)->input(input_of_pid, num_of_inputs, inputs);
}
//-------------------------------------------------------------------------------------------//
int mult(void * handle, const size_t share_count, const mp_limb_t * shares, mp_limb_t * products, int verify)
{
	return ((spdz2_ext_processor_base *)handle)->mult(share_count, shares, products, verify);
}
//-------------------------------------------------------------------------------------------//
int mix_add(void * handle, const mp_limb_t * share, const mp_limb_t * scalar, mp_limb_t * sum)
{
	return ((spdz2_ext_processor_base *)handle)->mix_add(share, scalar, sum);
}
//-------------------------------------------------------------------------------------------//
int mix_sub_scalar(void * handle, const mp_limb_t * share, const mp_limb_t * scalar, mp_limb_t * diff)
{
	return ((spdz2_ext_processor_base *)handle)->mix_sub_scalar(share, scalar, diff);
}
//-------------------------------------------------------------------------------------------//
int mix_sub_share(void * handle, const mp_limb_t * scalar, const mp_limb_t * share, mp_limb_t * diff)
{
	return ((spdz2_ext_processor_base *)handle)->mix_sub_share(scalar, share, diff);
}
//-------------------------------------------------------------------------------------------//
int mix_mul(void * handle, const mp_limb_t * share, const mp_limb_t * scalar, mp_limb_t * product)
{
	return ((spdz2_ext_processor_base *)handle)->mix_mul(share, scalar, product);
}
//-------------------------------------------------------------------------------------------//
int adds(void * handle, const mp_limb_t * share1, const mp_limb_t * share2, mp_limb_t * sum)
{
	return ((spdz2_ext_processor_base *)handle)->adds(share1, share2, sum);
}
//-------------------------------------------------------------------------------------------//
int subs(void * handle, const mp_limb_t * share1, const mp_limb_t * share2, mp_limb_t * diff)
{
	return ((spdz2_ext_processor_base *)handle)->subs(share1, share2, diff);
}
//-------------------------------------------------------------------------------------------//
int share_immediates(void * handle, const int party_id, const size_t value_count, const mp_limb_t * values, mp_limb_t * shares)
{
	return ((spdz2_ext_processor_base *)handle)->share_immediates(party_id, value_count, values, shares);
}
//-------------------------------------------------------------------------------------------//
int bit(void * handle, mp_limb_t * share)
{
	return ((spdz2_ext_processor_base *)handle)->bit(share);
}
//-------------------------------------------------------------------------------------------//
int inverse(void * handle, mp_limb_t * share_value, mp_limb_t * share_inverse)
{
	return ((spdz2_ext_processor_base *)handle)->inverse(share_value, share_inverse);
}
//-------------------------------------------------------------------------------------------//
