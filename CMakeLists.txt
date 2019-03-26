cmake_minimum_required (VERSION 3.9.5 FATAL_ERROR)
project (guard)

if(POLICY CMP0076)
  cmake_policy(SET CMP0076 OLD)
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

find_package(Git REQUIRED)

find_path(CBOR_LITE_INCLUDE_DIR cbor-lite)
if ((NOT CBOR_LITE_INCLUDE_DIR) OR (NOT EXISTS ${CBOR_LITE_INCLUDE_DIR}))
    message(STATUS "using cbor-lite submodule")
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init -- external/cbor-lite
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    set(CBOR_LITE_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/external/cbor-lite/include CACHE PATH "cbor-lite include directory" FORCE)
    message(STATUS "CBOR_LITE_INCLUDE_DIR=${CBOR_LITE_INCLUDE_DIR}")
endif()
include_directories(${CBOR_LITE_INCLUDE_DIR})

add_compile_options(-O0)
add_compile_options(-Wall -Wextra -pedantic -Wold-style-cast)

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -ftrapv")

include_directories(${PROJECT_SOURCE_DIR}/include ${CMAKE_BINARY_DIR}/include)

set(Boost_ADDITIONAL_VERSIONS "1.62" "1.62.0" "1.62.1" "1.63" "1.63.1" "1.64" "1.64.1" "1.65" "1.65.0" "1.65.1" "1.66" "1.66.0")
set(Boost_USE_STATIC_LIB ON)
find_package(Boost 1.62 REQUIRED COMPONENTS unit_test_framework thread system)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
add_definitions(-DBOOST_LOG_DYN_LINK)
add_definitions(-DBOOST_NO_AUTO_PTR)
if (NOT (${CMAKE_SYSTEM_NAME} MATCHES "Linux"))
    add_definitions(-DBOOST_ASIO_HAS_STD_STRING_VIEW)
endif()
link_libraries(${Boost_THREAD_LIBRARY} ${Boost_SYSTEM_LIBRARY})
 
find_package(OpenSSL REQUIRED)
include_directories(SYSTEM ${OPENSSL_INCLUDE_DIR})
link_libraries(${OPENSSL_LIBRARIES})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads)
link_libraries(Threads::Threads)

add_library(gcxp INTERFACE)
target_include_directories(gcxp INTERFACE include/)

add_subdirectory(consumer)
add_subdirectory(producer)

add_custom_target(unit-test COMMAND ./unit/unit --log_level=warning)
add_custom_target(junit COMMAND mkdir -p test-results && ./unit/unit --logger=JUNIT,warning,test-results/results.xml)

add_subdirectory(unit)

add_custom_target(reformat
    COMMAND sh tools/reformat.sh
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

