#pragma once

#include "spdz2_ext_processor_base.h"

#include "Protocol.h"
#include "Mersenne127.h"
#include <gmp.h>

class spdz2_ext_processor_mersenne127 : public spdz2_ext_processor_base
{
public:
	spdz2_ext_processor_mersenne127();
	virtual ~spdz2_ext_processor_mersenne127();

	int init(const int pid, const int num_of_parties, const int thread_id, const char * field,
			 const int open_count, const int mult_count, const int bits_count);
	int term();

	int offline(const int offline_size);
	int input(const int input_of_pid, mpz_t * input_value);
	int triple(mpz_t * a, mpz_t * b, mpz_t * c);
	int share_immediates(const int share_of_pid, const size_t value_count, const mpz_t * values, mpz_t * shares);
    int bit(mpz_t * share);
    int inverse(mpz_t * share_value, mpz_t * share_inverse);
    int open(const size_t share_count, const mpz_t * share_values, mpz_t * opens, int verify);
	int verify(int * error);
	int input(const int input_of_pid, const size_t num_of_inputs, mpz_t * inputs);
    int mult(const size_t share_count, const mpz_t * shares, mpz_t * products, int verify);
    int mix_add(mpz_t * share, const mpz_t * scalar);
    int mix_sub_scalar(mpz_t * share, const mpz_t * scalar);
    int mix_sub_share(const mpz_t * scalar, mpz_t * share);
};

/*
class spdz2_ext_processor_mersenne127 : public spdz2_ext_processor_base
{
	TemplateField<Mersenne127> * the_field;
	Protocol<Mersenne127> * the_party;
	mutable gmp_randstate_t the_gmp_rstate;

	std::vector<Mersenne127> m_aux1, m_aux2, m_aux3;
protected:

	virtual int init_protocol(const int open_count, const int mult_count, const int bits_count);
	virtual int delete_protocol();
	virtual bool protocol_offline();
	virtual bool protocol_share(const int pid, const size_t count, const mpz_t * input, mpz_t * output);
	virtual bool protocol_triple(mpz_t * A, mpz_t * B, mpz_t * C);
	virtual bool protocol_random_value(mpz_t * value);
	virtual bool protocol_value_inverse(const mpz_t * value, mpz_t * inverse);
	virtual bool protocol_open(const size_t value_count, const mpz_t * shares, mpz_t * opens, bool verify);
	virtual bool protocol_verify(int * error);
	virtual bool protocol_mult(const size_t count, const mpz_t * input, mpz_t * output, bool verify);
	virtual bool protocol_bits(const size_t count, mpz_t * bit_shares);
	virtual bool protocol_value_mult(const mpz_t * op1, const mpz_t * op2, mpz_t * product);

	virtual std::string get_parties_file();
	virtual std::string get_syslog_name();

public:
	spdz2_ext_processor_mersenne127();
	virtual ~spdz2_ext_processor_mersenne127();

    virtual int mix_add(mpz_t * share, const mpz_t * scalar);
    virtual int mix_sub_scalar(mpz_t * share, const mpz_t * scalar);
    virtual int mix_sub_share(const mpz_t * scalar, mpz_t * share);
};
*/
