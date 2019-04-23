enable_testing()

file(GLOB CBOR_UNIT_SOURCES ${CBOR_LITE_UNIT_DIR}/*.cpp)
list(REMOVE_ITEM CBOR_UNIT_SOURCES "${CBOR_LITE_UNIT_DIR}/fp.cpp")
list(REMOVE_ITEM CBOR_UNIT_SOURCES "${CBOR_LITE_UNIT_DIR}/main.cpp")
add_executable(unit main.cpp codec.cpp exception.cpp message.cpp ${CBOR_UNIT_SOURCES})
target_compile_definitions(unit PRIVATE "BOOST_TEST_DYN_LINK=1")
target_link_libraries(unit PRIVATE Boost::unit_test_framework)
if(OPENSSL_FOUND)
    target_sources(unit PRIVATE stream.cpp)
    target_link_libraries(unit PRIVATE Boost::thread Boost::system)
endif()

add_custom_target(unit-tests
    COMMENT Unit Tests
    DEPENDS unit
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/unit --log_level=warning
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_custom_target(junit
    COMMENT Unit Tests with JUNIT-format results output
    DEPENDS unit
    COMMAND mkdir -p ${PROJECT_BINARY_DIR}/test-reports
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/unit --log_level=warning -f JUNIT -k ${PROJECT_BINARY_DIR}/test-reports/unit.xml
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
