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
	/* Operational Section */
	pthread_t m_runner;
	bool m_run_flag;

	sem_t m_task;
	std::deque<int> m_task_q;
	pthread_mutex_t m_q_lock;

	void run();
	int push_task(const int op_code);
	int pop_task();

	static const int sm_op_code_offline_synch;
	static const int sm_op_code_input_synch;
	static const int sm_op_code_triple_synch;
	static const int sm_op_code_share_immediate_synch;
	static const int sm_op_code_bit_synch;
	static const int sm_op_code_inverse_synch;
	static const int sm_op_code_open_asynch;
	static const int sm_op_code_verify_asynch;
	static const int sm_op_code_input_asynch;
	static const int sm_op_code_mult_asynch;
	static const int sm_op_code_share_immediates_asynch;

	std::list<std::string> m_file_input;
	std::list<std::string>::iterator m_next_file_input;
	void load_file_input();
	void clear_file_input();
	int get_input_from_file(mpz_t * value);
	int get_input_from_user(mpz_t * value);
	int get_input(mpz_t * value, bool from_user = false) { return (from_user)? get_input_from_user(value): get_input_from_file(value); }

	/* Services Section */
	/*
		offline_synch
		input_synch
		triple_synch
		share_immediate_synch
		bit_synch
		inverse_synch

		open_asynch
		verify_asynch
		input_asynch
		mult_asynch
		share_immediates_asynch
	 */

	void exec_offline_synch();
	bool m_offline_synch_success;
	sem_t m_offline_synch_done;

	void exec_input_synch();
	bool m_input_synch_success;
	sem_t m_input_synch_done;
	mpz_t m_input_synch_input;
	mpz_t * m_input_synch_output;
	int m_input_synch_pid;

	void exec_triple_synch();
	bool m_triple_synch_success;
	sem_t m_triple_synch_done;
	mpz_t * m_A, * m_B, * m_C;

	void exec_share_immediate_synch();
	bool m_share_immediate_synch_success;
	sem_t m_share_immediate_synch_done;
	const mpz_t * m_share_immediate_synch_value;
	mpz_t * m_share_immediate_synch_share;

	void exec_bit_synch();
	bool m_bit_synch_success;
	sem_t m_bit_synch_done;
	mpz_t * m_bit_synch_share;

	void exec_inverse_synch();
	bool m_inverse_synch_success;
	sem_t m_inverse_synch_done;
	mpz_t m_inverse_synch_value, m_inverse_synch_inverse, * m_inverse_synch_value_share, * m_inverse_synch_inverse_share;

	void exec_open_asynch();
	bool m_open_asynch_on;
	bool m_open_asynch_success;
	sem_t m_open_asynch_done;
	size_t m_open_asynch_count;
	const mpz_t * m_open_asynch_input;
	mpz_t * m_open_asynch_output;
	bool m_open_asynch_verify;

	void exec_verify_asynch();
	bool m_verify_asynch_on;
	bool m_verify_asynch_success;
	sem_t m_verify_asynch_done;
	int * m_verify_asynch_error;

	void exec_input_asynch();
	bool m_input_asynch_on;
	bool m_input_asynch_success;
	sem_t m_input_asynch_done;
	int m_input_asynch_pid;
	size_t m_input_asynch_count;
	mpz_t * m_input_asynch_input;
	mpz_t * m_input_asynch_output;

	void exec_mult_asynch();
	bool m_mult_asynch_on;
	bool m_mult_asynch_success;
	sem_t m_mult_asynch_done;
	size_t m_mult_asynch_count;
	const mpz_t * m_mult_asynch_input;
	mpz_t * m_mult_asynch_output;
	bool m_mult_asynch_verify;

	void exec_share_immediates_asynch();
	bool m_share_immediates_asynch_on;
	bool m_share_immediates_asynch_success;
	sem_t m_share_immediates_asynch_done;
	size_t m_share_immediates_asynch_count;
	const mpz_t * m_share_immediates_asynch_input;
	mpz_t * m_share_immediates_asynch_output;

protected:
	int m_party_id, m_num_of_parties;
	std::string input_file;

	virtual int init_protocol(const int open_count, const int mult_count, const int bits_count) = 0;
	virtual int delete_protocol() = 0;
	virtual bool protocol_offline() = 0;
	virtual bool protocol_share(const int pid, const size_t count, const mpz_t * input, mpz_t * output) = 0;
	virtual bool protocol_triple(mpz_t * A, mpz_t * B, mpz_t * C) = 0;
	virtual bool protocol_random_value(mpz_t * value) = 0;
	virtual bool protocol_value_inverse(const mpz_t * value, mpz_t * inverse) = 0;
	virtual bool protocol_open(const size_t value_count, const mpz_t * shares, mpz_t * opens, bool verify) = 0;
	virtual bool protocol_verify(int * error) = 0;
	virtual bool protocol_mult(const size_t count, const mpz_t * input, mpz_t * output, bool verify) = 0;
	virtual bool protocol_bits(const size_t count, mpz_t * bit_shares) = 0;

public:
	spdz2_ext_processor_base();
	virtual ~spdz2_ext_processor_base();

	int start(const int pid, const int num_of_parties, const char * field,
			  const int open_count, const int mult_count, const int bits_count);
	int stop(const time_t timeout_sec = 2);

	int offline(const int offline_size, const time_t timeout_sec = 5);

	int input(const int input_of_pid, mpz_t * input_value, const time_t timeout_sec = 5);

	int triple(mpz_t * a, mpz_t * b, mpz_t * c, const time_t timeout_sec = 5);

    int share_immediate(const mpz_t * value, mpz_t * share, const time_t timeout_sec = 5);

    int bit(mpz_t * share, const time_t timeout_sec = 5);

    int inverse(mpz_t * share_value, mpz_t * share_inverse, const time_t timeout_sec = 5);

	int start_open(const size_t share_count, const mpz_t * share_values, mpz_t * opens, int verify);
	int stop_open(const time_t timeout_sec = 5);

	int start_verify(int * error);
	int stop_verify(const time_t timeout_sec = 5);

    int start_input(const int input_of_pid, const size_t num_of_inputs, mpz_t * inputs);
    int stop_input(const time_t timeout_sec = 5);

    int start_mult(const size_t share_count, const mpz_t * shares, mpz_t * products, int verify);
    int stop_mult(const time_t timeout_sec = 5);

    int start_share_immediates(const size_t value_count, const mpz_t * values, mpz_t * shares);
    int stop_share_immediates(const time_t timeout_sec = 5);

    virtual int mix_add(mpz_t * share, const mpz_t * scalar) = 0;
    virtual int mix_sub_scalar(mpz_t * share, const mpz_t * scalar) = 0;
    virtual int mix_sub_share(const mpz_t * scalar, mpz_t * share) = 0;

    friend void * spdz2_ext_processor_proc(void * arg);
};
