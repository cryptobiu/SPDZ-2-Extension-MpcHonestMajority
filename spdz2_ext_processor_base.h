#pragma once

#include <pthread.h>
#include <semaphore.h>
#include <deque>
#include <vector>
#include <string>

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
protected:

	int party_id, offline_size, num_of_parties;
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

	//--open---------------------------------------------
	std::vector<u_int64_t> shares, opens;
	bool do_verify_open;
	//---------------------------------------------------

	//--triple-------------------------------------------
	u_int64_t * pa, * pb, * pc;
	//---------------------------------------------------

	//--input--------------------------------------------
	int input_party_id;
	u_int64_t * p_input_value;
	//---------------------------------------------------

	//--input_asynch-------------------------------------
	bool input_asynch_on;
	int intput_asynch_party_id;
	size_t num_of_inputs;
	std::vector<u_int64_t> input_values;
	//---------------------------------------------------

	//--mult---------------------------------------------
	bool mult_on;
	std::vector<u_int64_t> mult_values, products;
	//---------------------------------------------------

	//--share_immediates---------------------------------
	bool share_immediates_on;
	std::vector<u_int64_t> immediates_values, immediates_shares;
	//---------------------------------------------------

	//--share_immediate----------------------------------
	std::vector<u_int64_t> immediate_value;
	u_int64_t * p_immediate_share;
	//---------------------------------------------------

public:
	spdz2_ext_processor_base();
	~spdz2_ext_processor_base();

	int start(const int pid, const int num_of_parties, const char * field, const int offline_size = 0);
	int stop(const time_t timeout_sec = 2);

	int offline(const int offline_size, const time_t timeout_sec = 5);

	int start_open(const size_t share_count, const u_int64_t * share_values, int verify);
	int stop_open(size_t * open_count, u_int64_t ** open_values, const time_t timeout_sec = 5);

	int triple(u_int64_t * a, u_int64_t * b, u_int64_t * c, const time_t timeout_sec = 5);

	int input(const int input_of_pid, u_int64_t * input_value);

	int start_verify(int * error);
	int stop_verify(const time_t timeout_sec = 5);

    int start_input(const int input_of_pid, const size_t num_of_inputs);
    int stop_input(size_t * input_count, u_int64_t ** inputs);

    int start_mult(const size_t share_count, const u_int64_t * shares, int verify);
    int stop_mult(size_t * product_count, u_int64_t ** products);

    virtual int mix_add(u_int64_t * share, u_int64_t scalar) = 0;
    virtual int mix_sub_scalar(u_int64_t * share, u_int64_t scalar) = 0;
    virtual int mix_sub_share(u_int64_t scalar, u_int64_t * share) = 0;

    int start_share_immediates(const int input_of_pid, const size_t value_count, const u_int64_t * values);
    int stop_share_immediates(size_t * share_count, u_int64_t ** shares, const time_t timeout_sec = 5);

    int share_immediate(const u_int64_t value, u_int64_t * share, const time_t timeout_sec = 5);

    int input_bit(u_int64_t * share, const time_t timeout_sec = 5);

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
