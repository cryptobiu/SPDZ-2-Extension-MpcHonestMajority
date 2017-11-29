cmake_minimum_required(VERSION 2.8)
project(spdzext)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -std=c++14 -maes -mpclmul -mbmi2 -fPIC")

INCLUDE_DIRECTORIES(	$ENV{HOME}/ReplicatedForSpdz
			$ENV{HOME} 
        		$ENV{HOME}/libscapi/lib/OTExtensionBristol
			$ENV{HOME}/libscapi/install/include
)

set(SOURCE_FILES
	spdzext.h
	spdzext.cpp
	spdz_ext_processor.h
	spdz_ext_processor.cpp
)

add_library(spdz_ext_biu SHARED ${SOURCE_FILES})

link_directories( $ENV{HOME}/libscapi/install/lib )

target_link_libraries(	spdz_ext_biu
			$ENV{HOME}/ReplicatedForSpdz/libReplicatedForSpdz.a
			$ENV{HOME}/libscapi/libscapi.a
			$ENV{HOME}/libscapi/install/lib/libssl.a
			$ENV{HOME}/libscapi/install/lib/libcrypto.a
			gmp
			gmpxx
			$ENV{HOME}/libscapi/install/lib/libboost_system.a
			$ENV{HOME}/libscapi/install/lib/libboost_thread.a
			pthread
			dl
			z
)

