enable_testing()

add_executable(unit codec.cpp exception.cpp message.cpp unit.cpp ${CBOR_LITE_INCLUDE_DIR}/../unit/basic.cpp ${CBOR_LITE_INCLUDE_DIR}/../unit/canon.cpp ${CBOR_LITE_INCLUDE_DIR}/../unit/exception.cpp)
if(OPENSSL_FOUND)
    target_sources(unit PRIVATE stream.cpp)
else()
    message(WARNING "Skipping stream.cpp unit tests, no OpenSSL found")
endif()
target_compile_definitions(unit PRIVATE "BOOST_TEST_DYN_LINK=1")
target_link_libraries(unit PRIVATE ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

add_custom_target(unit-tests
    COMMENT Unit Tests
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/unit --log_level=warning
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_custom_target(junit
    COMMENT Unit Tests with JUNIT-format results output
    COMMAND mkdir -p ${PROJECT_BINARY_DIR}/test-reports
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/unit --log_level=warning -f JUNIT -k ${PROJECT_BINARY_DIR}/test-reports/unit.xml
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
