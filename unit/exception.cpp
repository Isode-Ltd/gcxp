// Gcxp::Exception test cases
#include <gcxp/exception.h>
#include <boost/test/unit_test.hpp>
#include <cbor-lite/codec.h>

BOOST_AUTO_TEST_SUITE(gcxp)
BOOST_AUTO_TEST_SUITE(exception)

BOOST_AUTO_TEST_CASE(basic) {
    BOOST_CHECK_EXCEPTION(throw Gcxp::Exception(), Gcxp::Exception,
        [](Gcxp::Exception const& e) { return e.what() == std::string("GCXP Exception"); });

    BOOST_CHECK_EXCEPTION(throw Gcxp::Exception("cstring"), Gcxp::Exception,
        [](Gcxp::Exception const& e) { return e.what() == std::string("GCXP Exception: cstring"); });

    BOOST_CHECK_EXCEPTION(throw Gcxp::Exception(std::string("string")), Gcxp::Exception,
        [](Gcxp::Exception const& e) { return e.what() == std::string("GCXP Exception: string"); });

    Gcxp::Exception ex("another string");
    BOOST_CHECK_EXCEPTION(throw Gcxp::Exception(ex), Gcxp::Exception,
        [](Gcxp::Exception const& caught) { return caught.what() == std::string("GCXP Exception: another string"); });
    BOOST_CHECK_EQUAL(ex.what(), std::string("GCXP Exception: another string"));
    BOOST_CHECK_EXCEPTION(throw Gcxp::Exception(std::move(ex)), Gcxp::Exception,
        [](Gcxp::Exception const& caught) { return caught.what() == std::string("GCXP Exception: another string"); });
    BOOST_CHECK_EQUAL(ex.what(), std::string("")); // note the odd state, Gcxp::Exception not expected to be reused after move
}

BOOST_AUTO_TEST_CASE(inheritance) {
    // Gcxp::Exception doesn't (publicly) inherit from std::exception
    BOOST_CHECK_THROW(try { throw Gcxp::Exception(); } catch (const std::exception&){}, Gcxp::Exception);
    // nor from CborLite::Exception
    BOOST_CHECK_THROW(try { throw Gcxp::Exception(); } catch (const CborLite::Exception&){}, Gcxp::Exception);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
