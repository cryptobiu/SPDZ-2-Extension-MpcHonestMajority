
#ifndef SPDZEXT_H_
#define SPDZEXT_H_

#include <stdlib.h>

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
	int init(void ** handle, const int pid, const int num_of_parties, const char * field, const int offline_size = 0);

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
	 * @param[in] verify Whether the opened values are to be verified, 0 for false
	 * @return 0 on success, -1 otherwise
	 */
	int start_open(void * handle, const size_t share_count, const u_int64_t * shares, int verify);

	/**
	 * Complete an asynchronous opening of shared values
	 * Will fail if no open start was called before
	 * @param[in] handle An initialized session handle
	 * @param[out] open_count The number of the opened values
	 * @param[out] opens The opened values - the memory will be allocated by the function
	 * @return 0 on success, -1 otherwise
	 */
	int stop_open(void * handle, size_t * open_count, u_int64_t ** opens);

	/**
	 * Retrieve a triple of values
	 * @param[in] handle An initialized session handle
	 * @param[out] a The first value
	 * @param[out] b The second value
	 * @param[out] c The third value
	 * @return 0 on success, -1 otherwise
	 */
	int triple(void * handle, u_int64_t * a, u_int64_t * b, u_int64_t * c);

	/**
	 * Retrieve an input value from a party
	 * @param[in] handle An initialized session handle
	 * @param[in] input_of_pid The party identifier from which input is required
	 * @param[out] input_value The input value
	 * @return 0 on success, -1 otherwise
	 */
	int input(void * handle, const int input_of_pid, u_int64_t * input_value);

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
	 * @return 0 on success, -1 otherwise
	 */
    int start_input(void * handle, const int input_of_pid, const size_t num_of_inputs);

	/**
	 * Complete an asynchronous multiple input values operation
	 * Will fail if no input start was called before
	 * @param[in] handle An initialized session handle
	 * @param[out] input_count Number of the input values
	 * @param[out] inputs The input values - the memory will be allocated by the function
	 * @return 0 on success, -1 otherwise
	 */
    int stop_input(void * handle, size_t * input_count, u_int64_t ** inputs);

    /**
     * Start an asynchronous multiply operation
	 * Must be followed by a call to stop mult
	 * @param[in] handle An initialized session handle
	 * @param[in] share_count Number of the shared values
	 * @param[in] shares The shared values
	 * @param[in] verify Verification flag
	 * @return 0 on success, -1 otherwise
     */
    int start_mult(void * handle, const size_t share_count, const u_int64_t * shares, int verify);

	/**
	 * Complete an asynchronous multiply operation
	 * Will fail if no start mult was previously called
	 * @param[in] handle An initialized session handle
	 * @param[out] product_count Number of the product values
	 * @param[out] products The product results
	 * @return 0 on success, -1 otherwise
	 */
    int stop_mult(void * handle, size_t * product_count, u_int64_t ** products);

    /**
     * Addition of a share value with a scalar value
     * @param[in] handle An initialized session handle
     * @param[in/out] share The share value to which the scalar value is added
     * @param[in] scalar A scalar value to add to the share
	 * @return 0 on success, -1 otherwise
     */
    int mix_add(void * handle, u_int64_t * share, u_int64_t scalar);

    /**
     * Subtraction of a scalar value from a share
     * @param[in] handle An initialized session handle
     * @param[in/out] share The share value from which the scalar value is subtracted
     * @param[in] scalar A scalar value to subtract from the share
	 * @return 0 on success, -1 otherwise
     */
    int mix_sub_scalar(void * handle, u_int64_t * share, u_int64_t scalar);

    /**
     * Subtraction of a share value from a scalar
     * @param[in] handle An initialized session handle
     * @param[in/out] share The share value to subtract from the scalar
     * @param[in] scalar A scalar from which the share value is subtracted
	 * @return 0 on success, -1 otherwise
     */
    int mix_sub_share(void * handle, u_int64_t scalar, u_int64_t * share);

    /**
     * Start an asynchronous operation of sharing given values
	 * Must be followed by a stop call to share_immediates
	 * @param[in] handle An initialized session handle
	 * @param[in] input_of_pid The party identifier that will initiate the share (by simulating input)
	 * @param[in] value_count The number of the values to share
	 * @param[in] values An array of values to share
	 * @return 0 on success, -1 otherwise
     */
    int start_share_immediates(void * handle, const int input_of_pid, const size_t value_count, const u_int64_t * values);

	/**
	 * Complete an asynchronous value sharing operation
	 * Will fail if no start share_immediates was previously called
	 * @param[in] handle An initialized session handle
	 * @param[out] share_count Number of the shared values
	 * @param[out] shares The shared values
	 * @return 0 on success, -1 otherwise
	 */
    int stop_share_immediates(void * handle, size_t * share_count, u_int64_t ** shares);

    /**
     * Synchrnous load share from immediate
	 * @param[in] handle An initialized session handle
	 * @param[in] value A value to share
	 * @param[out] share A share to fill
	 * @return 0 on success, -1 otherwise
     */
    int share_immediate(void * handle, const u_int64_t value, u_int64_t * share);

    /**
     * Get a shared input bit value
     * @param[in] handle An initialized session handle
     * @param[out] share A share to fill
	 * @return 0 on success, -1 otherwise
     */
    int bit(void * handle, u_int64_t * share);

    /**
     * Get a shared value and its inverse
     * @param[in] handle An initialized session handle
     * @param[out] share_value A shared value to fill
     * @param[out] share_inverse A shared value-inverse to fill
	 * @return 0 on success, -1 otherwise
     */
    int inverse(void * handle, u_int64_t * share_value, u_int64_t * share_inverse);

	u_int64_t gfp_conversion(const u_int64_t value);
	u_int64_t gfp_add(u_int64_t, u_int64_t);
	u_int64_t gfp_sub(u_int64_t, u_int64_t);
	u_int64_t gfp_mult(u_int64_t, u_int64_t);

	u_int64_t gf2n_conversion(const u_int64_t value);
	u_int64_t gf2n_add(u_int64_t, u_int64_t);
	u_int64_t gf2n_sub(u_int64_t, u_int64_t);
	u_int64_t gf2n_mult(u_int64_t, u_int64_t);
}

#endif /* SPDZEXT_H_ */
