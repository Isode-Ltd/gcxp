// Gcxp::Message misc test cases
#include <gcxp/message.h>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(gcxp)
BOOST_AUTO_TEST_SUITE(message)

BOOST_AUTO_TEST_CASE(basic) {
    Gcxp::Message::Id id;
    BOOST_CHECK_EQUAL(Gcxp::Message::idToString(id), "<EMPTY>");
    id.push_back(0);
    BOOST_CHECK_EQUAL(Gcxp::Message::idToString(id), "00");
    id.push_back(1);
    BOOST_CHECK_EQUAL(Gcxp::Message::idToString(id), "0001");
    id.push_back(2);
    BOOST_CHECK_EQUAL(Gcxp::Message::idToString(id), "000102");
    id.push_back(3);
    BOOST_CHECK_EQUAL(Gcxp::Message::idToString(id), "00010203");
    id.push_back(128 + 4);
    BOOST_CHECK_EQUAL(Gcxp::Message::idToString(id), "00010203:84");
    id.push_back(5);
    BOOST_CHECK_EQUAL(Gcxp::Message::idToString(id), "00010203:8405");
    id.push_back(6);
    BOOST_CHECK_EQUAL(Gcxp::Message::idToString(id), "00010203:840506");
    id.push_back(7);
    BOOST_CHECK_EQUAL(Gcxp::Message::idToString(id), "00010203:84050607");
    id.push_back(128 + 8);
    BOOST_CHECK_EQUAL(Gcxp::Message::idToString(id), "00010203:84050607:88");
    id.push_back('\x09');
    BOOST_CHECK_EQUAL(Gcxp::Message::idToString(id), "00010203:84050607:8809");
    id.push_back(10);
    BOOST_CHECK_EQUAL(Gcxp::Message::idToString(id), "00010203:84050607:88090a");
    id.push_back(11);
    BOOST_CHECK_EQUAL(Gcxp::Message::idToString(id), "00010203:84050607:88090a0b");
    id.push_back(12);
    BOOST_CHECK_EQUAL(Gcxp::Message::idToString(id), "00010203:84050607:88090a0b:0c");
    id.push_back(13);
    BOOST_CHECK_EQUAL(Gcxp::Message::idToString(id), "00010203:84050607:88090a0b:0c0d");
    id.push_back(14);
    BOOST_CHECK_EQUAL(Gcxp::Message::idToString(id), "00010203:84050607:88090a0b:0c0d0e");
    id.push_back(15);
    BOOST_CHECK_EQUAL(Gcxp::Message::idToString(id), "00010203:84050607:88090a0b:0c0d0e0f");
    id.push_back('\xff');
    BOOST_CHECK_EQUAL(Gcxp::Message::idToString(id), "00010203:84050607:88090a0b:0c0d0e0f:ff");

    std::ostringstream out;
    out << Gcxp::Message::Type::invalid;
    BOOST_CHECK_EQUAL(out.str(), "Invalid");
    out.str("");
    out << Gcxp::Message::Type::notice;
    BOOST_CHECK_EQUAL(out.str(), "Notice");
    out.str("");
    out << Gcxp::Message::Type::request;
    BOOST_CHECK_EQUAL(out.str(), "Request");
    out.str("");
    out << Gcxp::Message::Type::response;
    BOOST_CHECK_EQUAL(out.str(), "Response");
    out.str("");
    out << static_cast<Gcxp::Message::Type>(-1);
    BOOST_CHECK_EQUAL(out.str(), "Unknown(-1)");
}

BOOST_AUTO_TEST_CASE(initialization) {
    Gcxp::Message m;
    BOOST_CHECK_EQUAL(m.id.size(), 0u);
    BOOST_CHECK_EQUAL(m.type, Gcxp::Message::Type::invalid);
    BOOST_CHECK_EQUAL(m.accepted, false);
    BOOST_CHECK_EQUAL(m.payload.size(), 0u);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
