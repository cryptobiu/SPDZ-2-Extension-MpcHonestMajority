#pragma once

#include <pthread.h>
#include <semaphore.h>
#include <deque>
#include <vector>
#include <list>
#include <string>
#include <gmp.h>

class spdz2_ext_processor_base
{
	pthread_t runner;
	bool run_flag;

	sem_t task;
	std::deque<int> task_q;
	pthread_mutex_t q_lock;

	void run();
	int push_task(const int op_code);
	int pop_task();

	//--offline------------------------------------------
	sem_t offline_done;
	void exec_offline();
	bool offline_success;
	//---------------------------------------------------

	//--open---------------------------------------------
	bool start_open_on;
	sem_t open_done;
	void exec_open();
	bool open_success;
	//---------------------------------------------------

	//--triple-------------------------------------------
	sem_t triple_done;
	void exec_triple();
	bool triple_success;
	//---------------------------------------------------

	//--input--------------------------------------------
	sem_t input_done;
	void exec_input();
	bool input_success;
	//---------------------------------------------------

	//--verify-------------------------------------------
	bool verification_on;
	int * verification_error;
	sem_t verify_done;
	void exec_verify();
	bool verify_success;
	//---------------------------------------------------

	//--input_asynch-------------------------------------
	sem_t input_asynch_done;
	void exec_input_asynch();
	bool input_asynch_success;
	//---------------------------------------------------

	//--mult---------------------------------------------
	sem_t mult_done;
	void exec_mult();
	bool mult_success;
	//---------------------------------------------------

	//--share_immediates---------------------------------
	sem_t share_immediates_done;
	void exec_share_immediates();
	bool share_immediates_success;
	//---------------------------------------------------

	//--share_immediate----------------------------------
	sem_t share_immediate_done;
	void exec_share_immediate();
	bool share_immediate_success;
	//---------------------------------------------------

	std::list<mpz_t> m_file_input;
	std::list<mpz_t>::iterator m_next_file_input;
	void load_file_input();
	void clear_file_input();
	int get_input_from_file(mpz_t * value);
	int get_input_from_user(mpz_t * value);
	int get_input(mpz_t * value, bool from_user = false) { return (from_user)? get_input_from_user(value): get_input_from_file(value); }

protected:

	int m_party_id, m_offline_size, num_of_parties;
	std::string input_file;

	virtual int init_protocol() = 0;
	virtual void delete_protocol() = 0;
	virtual bool protocol_offline() = 0;
	virtual bool protocol_open() = 0;
	virtual bool protocol_triple() = 0;
	virtual bool protocol_input() = 0;
	virtual bool protocol_input_asynch() = 0;
	virtual bool protocol_mult() = 0;
	virtual bool protocol_share_immediates() = 0;
	virtual bool protocol_share_immediate() = 0;
	virtual bool protocol_random_value(mpz_t * value) const = 0;
	virtual bool protocol_value_inverse(const mpz_t * value, mpz_t * inverse) const = 0;

	//--open---------------------------------------------
	const mpz_t * to_open_share_values;
	mpz_t * opened_share_values;
	size_t open_share_value_count;
	bool do_verify_open;
	//---------------------------------------------------

	//--triple-------------------------------------------
	mpz_t * pa, * pb, * pc;
	//---------------------------------------------------

	//--input--------------------------------------------
	int input_party_id;
	mpz_t * p_input_value;
	//---------------------------------------------------

	//--input_asynch-------------------------------------
	bool input_asynch_on;
	int intput_asynch_party_id;
	size_t intput_asynch_count;
	mpz_t * intput_asynch_values;
	//---------------------------------------------------

	//--mult---------------------------------------------
	bool mult_on;
	const mpz_t * mult_shares;
	size_t mult_share_count;
	mpz_t * mult_products;
	//---------------------------------------------------

	//--share_immediates---------------------------------
	bool share_immediates_on;
	const mpz_t * immediates_values;
	size_t immediates_count;
	mpz_t * immediates_shares;
	//---------------------------------------------------

	//--share_immediate----------------------------------
	const mpz_t * immediate_value;
	mpz_t * immediate_share;
	//---------------------------------------------------

	void load_share_immediates_strings(std::vector<std::string>&) const;
public:
	spdz2_ext_processor_base();
	~spdz2_ext_processor_base();

	int start(const int pid, const int num_of_parties, const char * field, const int offline_size = 0);
	int stop(const time_t timeout_sec = 2);

	int offline(const int offline_size, const time_t timeout_sec = 5);

	int start_open(const size_t share_count, const mpz_t * share_values, mpz_t * opens, int verify);
	int stop_open(const time_t timeout_sec = 5);

	int triple(mpz_t * a, mpz_t * b, mpz_t * c, const time_t timeout_sec = 5);

	int input(const int input_of_pid, mpz_t * input_value);

	int start_verify(int * error);
	int stop_verify(const time_t timeout_sec = 5);

    int start_input(const int input_of_pid, const size_t num_of_inputs, mpz_t * inputs);
    int stop_input();

    int start_mult(const size_t share_count, const mpz_t * shares, mpz_t * products, int verify);
    int stop_mult(const time_t timeout_sec = 5);

    virtual int mix_add(mpz_t * share, const mpz_t * scalar) = 0;
    virtual int mix_sub_scalar(mpz_t * share, const mpz_t * scalar) = 0;
    virtual int mix_sub_share(const mpz_t * scalar, mpz_t * share) = 0;

    int start_share_immediates(const size_t value_count, const mpz_t * values, mpz_t * shares);
    int stop_share_immediates(const time_t timeout_sec = 5);

    int share_immediate(const mpz_t * value, mpz_t * share, const time_t timeout_sec = 5);

    int bit(mpz_t * share, const time_t timeout_sec = 5);
    int inverse(mpz_t * share_value, mpz_t * share_inverse, const time_t timeout_sec = 5);

    friend void * spdz2_ext_processor_proc(void * arg);

	static const int op_code_open;
	static const int op_code_triple;
	static const int op_code_offline;
	static const int op_code_input;
	static const int op_code_verify;
	static const int op_code_input_asynch;
	static const int op_code_mult;
	static const int op_code_share_immediates;
	static const int op_code_share_immediate;
};
