enable_testing()

add_executable(unit codec.cpp exception.cpp message.cpp unit.cpp ${CBOR_LITE_INCLUDE_DIR}/../unit/basic.cpp ${CBOR_LITE_INCLUDE_DIR}/../unit/canon.cpp ${CBOR_LITE_INCLUDE_DIR}/../unit/exception.cpp)
if(OPENSSL_FOUND)
    target_sources(unit PRIVATE stream.cpp)
endif()
target_compile_definitions(unit PRIVATE "BOOST_TEST_DYN_LINK=1")
target_link_libraries(unit PRIVATE ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

