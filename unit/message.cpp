// Gcxp::Message misc test cases
#include <gcxp/message.h>
#include <boost/test/unit_test.hpp>
#include <cstdint>

BOOST_AUTO_TEST_SUITE(gcxp)
BOOST_AUTO_TEST_SUITE(message)

BOOST_AUTO_TEST_CASE(idToString) {
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
}

BOOST_AUTO_TEST_CASE(payloadToString) {
    {
        const std::string content;
        const Gcxp::Message::Payload payload;
        BOOST_CHECK_EQUAL(Gcxp::Message::payloadToString(payload), "");
    }
    {
        const std::string content = "<content/>";
        const Gcxp::Message::Payload payload(std::begin(content), std::end(content));
        const auto got = Gcxp::Message::payloadToString(payload);
        BOOST_CHECK_EQUAL(got.length(), content.length() * 2);
        BOOST_CHECK_EQUAL(got, "3c636f6e74656e742f3e");
    }
    {
        const std::string content("\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f", 16);
        const Gcxp::Message::Payload payload(std::begin(content), std::end(content));
        const auto got = Gcxp::Message::payloadToString(payload);
        BOOST_CHECK_EQUAL(got.length(), content.length() * 2);
        BOOST_CHECK_EQUAL(got, "000102030405060708090a0b0c0d0e0f");
    }
    {
        const std::string content("\xf0\xf1\xf2\xf3\xf4\xf5\xf6\xf7\xf8\xf9\xfa\xfb\xfc\xfd\xfe\xff", 16);
        const Gcxp::Message::Payload payload(std::begin(content), std::end(content));
        const auto got = Gcxp::Message::payloadToString(payload);
        BOOST_CHECK_EQUAL(got.length(), content.length() * 2);
        BOOST_CHECK_EQUAL(got, "f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff");
    }
    {
        const std::string content("\x00\x10\x20\x30\x40\x50\x60\x70\x80\x90\xa0\xb0\xc0\xd0\xe0\xf0", 16);
        const Gcxp::Message::Payload payload(std::begin(content), std::end(content));
        const auto got = Gcxp::Message::payloadToString(payload);
        BOOST_CHECK_EQUAL(got.length(), content.length() * 2);
        BOOST_CHECK_EQUAL(got, "00102030405060708090a0b0c0d0e0f0");
    }
}

BOOST_AUTO_TEST_CASE(type) {
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

BOOST_AUTO_TEST_CASE(initList) {
    Gcxp::Message m{Gcxp::Message::Type::notice, false, {}, {}};
    BOOST_CHECK_EQUAL(m.type, Gcxp::Message::Type::notice);
    BOOST_CHECK_EQUAL(m.accepted, false);
    BOOST_CHECK(m.id.empty());
    BOOST_CHECK(m.payload.empty());
    std::ostringstream out;
    out << m;
    BOOST_CHECK_EQUAL(out.str(), "INV  I:<EMPTY>");
}

BOOST_AUTO_TEST_CASE(initListWithEquals) {
    Gcxp::Message m = {Gcxp::Message::Type::notice, false, {}, {}};
    BOOST_CHECK_EQUAL(m.type, Gcxp::Message::Type::notice);
    BOOST_CHECK_EQUAL(m.accepted, false);
    BOOST_CHECK(m.id.empty());
    BOOST_CHECK(m.payload.empty());
    std::ostringstream out;
    out << m;
    BOOST_CHECK_EQUAL(out.str(), "INV  I:<EMPTY>");
}

BOOST_AUTO_TEST_CASE(operatorOut) {
    BOOST_CHECK_NO_THROW({
        Gcxp::Message m;
        std::ostringstream out;
        out << m;
        BOOST_CHECK_EQUAL(out.str(), "INV  I:<EMPTY>");
    });
    BOOST_CHECK_NO_THROW({
        Gcxp::Message m;
        m.type = Gcxp::Message::Type::notice;
        m.id = Gcxp::Message::constructId(std::int16_t(-1));
        m.accepted = true;
        const std::string content = "<doc/>";
        m.payload.assign(std::begin(content), std::end(content));
        std::ostringstream out;
        out << m;
        BOOST_CHECK_EQUAL(out.str(), "INV  I:ffff P:3c646f632f3e");
    });
    BOOST_CHECK_NO_THROW({
        Gcxp::Message m;
        m.type = Gcxp::Message::Type::request;
        m.id = Gcxp::Message::constructId(std::uint16_t(1));
        const std::string content = "<doc/>";
        m.payload.assign(std::begin(content), std::end(content));
        std::ostringstream out;
        out << m;
        BOOST_CHECK_EQUAL(out.str(), "REQ  I:0001 P:3c646f632f3e");
    });
    BOOST_CHECK_NO_THROW({
        Gcxp::Message m;
        m.type = Gcxp::Message::Type::response;
        m.id = Gcxp::Message::constructId(std::uint32_t(2));
        m.accepted = true;
        std::ostringstream out;
        out << m;
        BOOST_CHECK_EQUAL(out.str(), "RSP  A:T I:00000002");
    });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
