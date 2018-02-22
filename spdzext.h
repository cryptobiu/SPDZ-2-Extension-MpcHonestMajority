
#ifndef SPDZEXT_H_
#define SPDZEXT_H_

#include <stdlib.h>
#include <gmp.h>

extern "C"
{
	/**
	 * Initialize a session with the extension library for the client.
	 * @param[out] handle A session handle to be filled by the function
	 * @param[in] pid The party identifier
	 * @param[in] num_of_parties The total number of parties
	 * @param[in] field The prime field used in text format
	 * @param[in] offline_size The number of offline elements to create
	 * @return 0 on success, -1 otherwise
	 */
	int init(void ** handle, const int pid, const int num_of_parties, const int thread_id,
			const char * field, const int open_count, const int mult_count, const int bits_count);

	/**
	 * Terminates an initialized session with the extension library and frees all resources
	 * @param[in] handle An initialized session handle
	 * @return 0 on success, -1 otherwise
	 */
	int term(void * handle);

	/**
	 * Create additional offline elements
	 * @param[in] handle An initialized session handle
	 * @param[in] offline_size The number of offline elements to create
	 * @return 0 on success, -1 otherwise
	 */
	int offline(void * handle, const int offline_size);

	/**
	 * Start an asynchronous opening of shared values
	 * Must be followed by a call to stop open
	 * @param[in] handle An initialized session handle
	 * @param[in] share_count The number of shared values to be opened
	 * @param[in] shares An array of share values to be opened
	 * @param[out] opens An array of opened values
	 * @param[in] verify Whether the opened values are to be verified, 0 for false
	 * @return 0 on success, -1 otherwise
	 */
	int start_open(void * handle, const size_t share_count, const mpz_t * shares, mpz_t * opens, int verify);

	/**
	 * Complete an asynchronous opening of shared values
	 * Will fail if no open start was called before
	 * @param[in] handle An initialized session handle
	 * @return 0 on success, -1 otherwise
	 */
	int stop_open(void * handle);

	/**
	 * Retrieve a triple of values
	 * @param[in] handle An initialized session handle
	 * @param[out] a The first value
	 * @param[out] b The second value
	 * @param[out] c The third value
	 * @return 0 on success, -1 otherwise
	 */
	int triple(void * handle, mpz_t * a, mpz_t * b, mpz_t * c);

	/**
	 * Retrieve an input value from a party
	 * @param[in] handle An initialized session handle
	 * @param[in] input_of_pid The party identifier from which input is required
	 * @param[out] input_value The input value
	 * @return 0 on success, -1 otherwise
	 */
	int input(void * handle, const int input_of_pid, mpz_t * input_value);

	/**
	 * Start an asynchronous verification operation
	 * Must be followed by a call to stop verify
	 * @param[in] handle An initialized session handle
	 * @param[out] error Verification error code
	 * @return 0 on success, -1 otherwise
	 */
	int start_verify(void * handle, int * error);

	/**
	 * Complete an asynchronous verification operation
	 * Will fail if no verification start was called before
	 * @param[in] handle An initialized session handle
	 * @return 0 on success, -1 otherwise
	 */
	int stop_verify(void * handle);

	/**
	 * Start an asynchronous multiple input values operation
	 * Must be followed by a call to stop input
	 * @param[in] handle An initialized session handle
	 * @param[in] input_of_pid The party identifier from which input is required
	 * @param[in] num_of_inputs The number of required input values
	 * @param[out] inputs The input values
	 * @return 0 on success, -1 otherwise
	 */
    int start_input(void * handle, const int input_of_pid, const size_t num_of_inputs, mpz_t * inputs);

	/**
	 * Complete an asynchronous multiple input values operation
	 * Will fail if no input start was called before
	 * @param[in] handle An initialized session handle
	 * @return 0 on success, -1 otherwise
	 */
    int stop_input(void * handle);

    /**
     * Start an asynchronous multiply operation
	 * Must be followed by a call to stop mult
	 * @param[in] handle An initialized session handle
	 * @param[in] share_count Number of the shared values
	 * @param[in] shares The shared values
	 * @param[out] products The product results
	 * @param[in] verify Verification flag
	 * @return 0 on success, -1 otherwise
     */
    int start_mult(void * handle, const size_t share_count, const mpz_t * shares, mpz_t * products, int verify);

	/**
	 * Complete an asynchronous multiply operation
	 * Will fail if no start mult was previously called
	 * @param[in] handle An initialized session handle
	 * @return 0 on success, -1 otherwise
	 */
    int stop_mult(void * handle);

    /**
     * Addition of a share value with a scalar value
     * @param[in] handle An initialized session handle
     * @param[in/out] share The share value to which the scalar value is added
     * @param[in] scalar A scalar value to add to the share
	 * @return 0 on success, -1 otherwise
     */
    int mix_add(void * handle, mpz_t * share, const mpz_t * scalar);

    /**
     * Subtraction of a scalar value from a share
     * @param[in] handle An initialized session handle
     * @param[in/out] share The share value from which the scalar value is subtracted
     * @param[in] scalar A scalar value to subtract from the share
	 * @return 0 on success, -1 otherwise
     */
    int mix_sub_scalar(void * handle, mpz_t * share, const mpz_t * scalar);

    /**
     * Subtraction of a share value from a scalar
     * @param[in] handle An initialized session handle
     * @param[in/out] share The share value to subtract from the scalar
     * @param[in] scalar A scalar from which the share value is subtracted
	 * @return 0 on success, -1 otherwise
     */
    int mix_sub_share(void * handle, const mpz_t * scalar, mpz_t * share);

    /**
     * Start an asynchronous operation of sharing given values
	 * Must be followed by a stop call to share_immediates
	 * @param[in] handle An initialized session handle
	 * @param[in] value_count The number of the values to share
	 * @param[in] values An array of values to share
	 * @param[out] shares The shared values
	 * @return 0 on success, -1 otherwise
     */
    int start_share_immediates(void * handle, const size_t value_count, const mpz_t * values, mpz_t * shares);

	/**
	 * Complete an asynchronous value sharing operation
	 * Will fail if no start share_immediates was previously called
	 * @param[in] handle An initialized session handle
	 * @return 0 on success, -1 otherwise
	 */
    int stop_share_immediates(void * handle);

    /**
     * Synchrnous load share from immediate
	 * @param[in] handle An initialized session handle
	 * @param[in] value A value to share
	 * @param[out] share A share to fill
	 * @return 0 on success, -1 otherwise
     */
    int share_immediate(void * handle, const mpz_t * value, mpz_t * share);

    /**
     * Get a shared input bit value
     * @param[in] handle An initialized session handle
     * @param[out] share A share to fill
	 * @return 0 on success, -1 otherwise
     */
    int bit(void * handle, mpz_t * share);

    /**
     * Get a shared value and its inverse
     * @param[in] handle An initialized session handle
     * @param[out] share_value A shared value to fill
     * @param[out] share_inverse A shared value-inverse to fill
	 * @return 0 on success, -1 otherwise
     */
    int inverse(void * handle, mpz_t * share_value, mpz_t * share_inverse);

    /*
    mpz_t gfp_conversion(const mpz_t * value);
    mpz_t gfp_add(const mpz_t *, const mpz_t *);
    mpz_t gfp_sub(const mpz_t *, const mpz_t *);
    mpz_t gfp_mult(const mpz_t *, const mpz_t *);

    mpz_t gf2n_conversion(const mpz_t value);
    mpz_t gf2n_add(const mpz_t *, const mpz_t *);
    mpz_t gf2n_sub(const mpz_t *, const mpz_t *);
    mpz_t gf2n_mult(const mpz_t *, const mpz_t *);
    */
}

#endif /* SPDZEXT_H_ */
