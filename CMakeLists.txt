cmake_minimum_required(VERSION 2.8)
project(spdzext)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -std=c++14 -maes -mpclmul -mbmi2 -fPIC")

INCLUDE_DIRECTORIES(	$ENV{HOME}/MPCHonestMajorityForSpdz
						$ENV{HOME} 
						$ENV{HOME}/libscapi/lib/OTExtensionBristol
						$ENV{HOME}/libscapi/install/include
						$ENV{HOME}/libscapi/include/infra
						$ENV{HOME}/ACP/comm_client
)

set(SOURCE_FILES
	spdzext.h
	spdzext.cpp
	spdzext_width_defs.h
	spdz2_ext_processor_base.h
	spdz2_ext_processor_base.cpp
	spdz2_ext_processor_mersenne127.h
	spdz2_ext_processor_mersenne127.cpp
	spdz2_ext_processor_mersenne61.h
	spdz2_ext_processor_mersenne61.cpp
)

add_library(spdz_ext_biu SHARED ${SOURCE_FILES})

link_directories(
	$ENV{HOME}/libscapi/install/lib
)

target_link_libraries(	spdz_ext_biu
						$ENV{HOME}/MPCHonestMajorityForSpdz/libMPCHonestMajorityForSpdz.a
						$ENV{HOME}/ntl-11.1.0/src/ntl.a
						$ENV{HOME}/libscapi/libscapi.a
						$ENV{HOME}/libscapi/install/lib/libssl.a
						$ENV{HOME}/libscapi/install/lib/libcrypto.a
						gmp
						gmpxx
						$ENV{HOME}/libscapi/install/lib/libboost_system.a
						$ENV{HOME}/libscapi/install/lib/libboost_thread.a
						log4cpp
						pthread
						dl
						z
)


