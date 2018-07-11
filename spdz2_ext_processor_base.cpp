
#include "spdz2_ext_processor_base.h"

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>

#include "Measurement.hpp"

static const char tsk0[] = "setup";
static const char tsk1[] = "offline";
static const char tsk2[] = "online";

//***********************************************************************************************//
spdz2_ext_processor_base::spdz2_ext_processor_base()
{
}

//***********************************************************************************************//
spdz2_ext_processor_base::~spdz2_ext_processor_base()
{
}

//***********************************************************************************************//
int spdz2_ext_processor_base::init(const int pid, const int num_of_parties, const int thread_id, const char * field,
		 	 	 	 	 	 	   const int open_count, const int mult_count, const int bits_count)
{
	m_pid = pid;
	m_nparties = num_of_parties;
	m_thid = thread_id;
	return 0;
}

//***********************************************************************************************//
