cmake_minimum_required (VERSION 3.9.5 FATAL_ERROR)
project (gcxp)

if(POLICY CMP0076)
    cmake_policy(SET CMP0076 OLD)
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "C++ compiler is ${CMAKE_CXX_COMPILER_ID}")

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(WARNING "Not a 64-bit system! YMMV")
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

find_package(Git REQUIRED)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(/permissive-)
else()
    add_compile_options(-O0)
    add_compile_options(-Wall -Wextra -pedantic -Wold-style-cast)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        add_compile_options(-Wno-implicit-fallthrough)
    endif()
endif()

find_path(CBOR_LITE_INCLUDE_DIR cbor-lite)
if((NOT CBOR_LITE_INCLUDE_DIR) OR (NOT EXISTS ${CBOR_LITE_INCLUDE_DIR}))
    message(STATUS "using cbor-lite submodule")
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init -- external/cbor-lite
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    set(CBOR_LITE_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/external/cbor-lite/include CACHE PATH "cbor-lite include directory" FORCE)
    message(STATUS "CBOR_LITE_INCLUDE_DIR=${CBOR_LITE_INCLUDE_DIR}")
    set(CBOR_LITE_UNIT_DIR ${CMAKE_SOURCE_DIR}/external/cbor-lite/unit CACHE PATH "cbor-lite unit test directory" FORCE)
    message(STATUS "CBOR_LITE_UNIT_DIR=${CBOR_LITE_UNIT_DIR}")
else()
    set(CBOR_LITE_UNIT_DIR ${CBOR_LITE_INCLUDE_DIR}/../unit CACHE PATH "cbor-lite unit test directory" FORCE)
endif()
include_directories(${CBOR_LITE_INCLUDE_DIR})

include_directories(${PROJECT_SOURCE_DIR}/include ${CMAKE_BINARY_DIR}/include)

add_library(gcxp INTERFACE)
target_include_directories(gcxp INTERFACE include/)

if(WIN32)
    return()
endif()

include_directories(SYSTEM ${OPENSSL_INCLUDE_DIR})
link_libraries(${OPENSSL_LIBRARIES})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads)
link_libraries(Threads::Threads)

set(Boost_NO_WARN_NEW_VERSIONS 1)
set(Boost_USE_STATIC_LIB ON)
find_package(Boost REQUIRED COMPONENTS thread system OPTIONAL_COMPONENTS unit_test_framework)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
add_definitions(-DBOOST_NO_AUTO_PTR)
add_definitions(-DBOOST_ASIO_HAS_STD_STRING_VIEW)

link_libraries(${Boost_THREAD_LIBRARY} ${Boost_SYSTEM_LIBRARY})

if(Boost_UNIT_TEST_FRAMEWORK_FOUND)
    add_subdirectory(unit)
endif()

add_subdirectory(consumer)
add_subdirectory(producer)

add_custom_target(reformat
    COMMAND sh tools/reformat.sh
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

