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
	std::string m_logcat;

	typedef struct
	{
		mp_limb_t * shared_values;
		size_t share_count, share_index;
	}shared_input_t;

	std::map< int , shared_input_t > m_shared_inputs;

	int load_inputs();
	int delete_inputs();
	int load_party_input_specs(std::list<std::string> & party_input_specs);
	int load_party_inputs(const std::string & party_input_spec);
	int load_party_inputs(const int pid, const size_t count);

	int load_self_party_inputs(const size_t count);
	int load_peer_party_inputs(const int pid, const size_t count, const mp_limb_t * clr_values = NULL);
	int load_clr_party_inputs(mp_limb_t * clr_values, const size_t count);

	int init_log(int log_level);

public:
	spdz2_ext_processor_base();
	virtual ~spdz2_ext_processor_base();

	virtual int init(const int pid, const int num_of_parties, const int thread_id, const char * field,
			 const int open_count, const int mult_count, const int bits_count, int log_level);

	virtual int term() = 0;

	virtual int input(const int input_of_pid, mp_limb_t * input_value);
	virtual int input(const int input_of_pid, const size_t num_of_inputs, mp_limb_t * inputs);
    virtual int inverse(mp_limb_t * share_value, mp_limb_t * share_inverse);
	virtual int inverse_value(const mp_limb_t * value, mp_limb_t * inverse);

	virtual int get_P(mpz_t P) = 0;
	virtual int offline(const int offline_size) = 0;
	virtual int triple(mp_limb_t * a, mp_limb_t * b, mp_limb_t * c) = 0;
	virtual int closes(const int share_of_pid, const size_t value_count, const mp_limb_t * values, mp_limb_t * shares) = 0;
    virtual int bit(mp_limb_t * share) = 0;
    virtual int open(const size_t share_count, const mp_limb_t * share_values, mp_limb_t * opens, int verify) = 0;
	virtual int verify(int * error) = 0;
    virtual int mult(const size_t share_count, const mp_limb_t * xshares, const mp_limb_t * yshares, mp_limb_t * products, int verify) = 0;
    virtual int mix_add(const mp_limb_t * share, const mp_limb_t * scalar, mp_limb_t * sum) = 0;
    virtual int mix_sub_scalar(const mp_limb_t * share, const mp_limb_t * scalar, mp_limb_t * diff) = 0;
    virtual int mix_sub_share(const mp_limb_t * scalar, const mp_limb_t * share, mp_limb_t * diff) = 0;
    virtual int mix_mul(const mp_limb_t * share, const mp_limb_t * scalar, mp_limb_t * product) = 0;
    virtual int adds(const mp_limb_t * share1, const mp_limb_t * share2, mp_limb_t * sum) = 0;
    virtual int subs(const mp_limb_t * share1, const mp_limb_t * share2, mp_limb_t * diff) = 0;

    virtual std::string get_parties_file() = 0;
	virtual std::string get_log_file() = 0;
	virtual std::string get_log_category() = 0;
};

#define LC(x) log4cpp::Category::getInstance(x)
