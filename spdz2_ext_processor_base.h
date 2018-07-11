#pragma once

#include <pthread.h>
#include <semaphore.h>
#include <deque>
#include <vector>
#include <map>
#include <list>
#include <string>
#include <gmp.h>

//class Measurement;

class spdz2_ext_processor_base
{
protected:
	int m_pid, m_nparties;
	int m_thid;
	std::string m_field;
	int m_nopen, m_nmult, m_nbits;

public:
	spdz2_ext_processor_base();
	virtual ~spdz2_ext_processor_base();

	virtual int init(const int pid, const int num_of_parties, const int thread_id, const char * field,
			 const int open_count, const int mult_count, const int bits_count);

	virtual int term() = 0;

	virtual int offline(const int offline_size) = 0;
	virtual int input(const int input_of_pid, mpz_t * input_value) = 0;
	virtual int triple(mpz_t * a, mpz_t * b, mpz_t * c) = 0;
	virtual int share_immediates(const size_t value_count, const mpz_t * values, mpz_t * shares) = 0;
    virtual int bit(mpz_t * share) = 0;
    virtual int inverse(mpz_t * share_value, mpz_t * share_inverse) = 0;
    virtual int open(const size_t share_count, const mpz_t * share_values, mpz_t * opens, int verify) = 0;
	virtual int verify(int * error) = 0;
	virtual int input(const int input_of_pid, const size_t num_of_inputs, mpz_t * inputs) = 0;
    virtual int mult(const size_t share_count, const mpz_t * shares, mpz_t * products, int verify) = 0;
    virtual int mix_add(mpz_t * share, const mpz_t * scalar) = 0;
    virtual int mix_sub_scalar(mpz_t * share, const mpz_t * scalar) = 0;
    virtual int mix_sub_share(const mpz_t * scalar, mpz_t * share) = 0;

    virtual std::string get_parties_file();
};
