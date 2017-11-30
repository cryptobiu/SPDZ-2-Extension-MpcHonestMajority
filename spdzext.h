
#ifndef SPDZEXT_H_
#define SPDZEXT_H_

#include <stdlib.h>

//#define SPDZEXT_VALTYPE	u_int32_t
#define SPDZEXT_VALTYPE	u_int64_t

extern "C"
{
	/**
	 * Initialize a session with the extension library for the client.
	 * @param[out] handle A session handle to be filled by the function
	 * @param[in] pid The party identifier
	 * @param[in] field The prime field used in text format
	 * @param[in] offline_size The number of offline elements to create
	 * @return 0 on success, -1 otherwise
	 */
	int init(void ** handle, const int pid, const char * field, const int offline_size = 0);

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
	int start_open(void * handle, const size_t share_count, const SPDZEXT_VALTYPE * shares, int verify);

	/**
	 * Complete an asynchronous opening of shared values
	 * Will fail if no open start was called before
	 * @param[in] handle An initialized session handle
	 * @param[out] open_count The number of the opened values
	 * @param[out] opens The opened values - the memory will be allocated by the function
	 * @return 0 on success, -1 otherwise
	 */
	int stop_open(void * handle, size_t * open_count, SPDZEXT_VALTYPE ** opens);

	/**
	 * Retrieve a triple of values
	 * @param[in] handle An initialized session handle
	 * @param[out] a The first value
	 * @param[out] b The second value
	 * @param[out] c The third value
	 * @return 0 on success, -1 otherwise
	 */
	int triple(void * handle, SPDZEXT_VALTYPE * a, SPDZEXT_VALTYPE * b, SPDZEXT_VALTYPE * c);

	/**
	 * Retrieve an input value from a party
	 * @param[in] handle An initialized session handle
	 * @param[in] input_of_pid The party identifier from which input is required
	 * @param[out] input_value The input value
	 * @return 0 on success, -1 otherwise
	 */
	int input(void * handle, const int input_of_pid, SPDZEXT_VALTYPE * input_value);

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
    int stop_input(void * handle, size_t * input_count, SPDZEXT_VALTYPE ** inputs);

    /**
     * Test the library value conversion from integer (32/64) to internal representation
     * The returned value should be equal to the provided value
     * @param value The test value to convert
     * @return The value restored from the converted value
     */
	SPDZEXT_VALTYPE test_conversion(const SPDZEXT_VALTYPE value);
}

#endif /* SPDZEXT_H_ */
