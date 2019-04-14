// GCXP Codec Test Cases
#include <gcxp/codec.h>
#include <gcxp/message.h>
#include <boost/test/unit_test.hpp>
#include <tuple>

BOOST_AUTO_TEST_SUITE(gcxp)

BOOST_AUTO_TEST_CASE(preamble) {
    BOOST_CHECK_NO_THROW(Gcxp::checkVersion(Gcxp::gcxpVersion));

    std::vector<char> buffer;
    BOOST_CHECK_NO_THROW({
        auto len = Gcxp::Codec::encodeVersion(buffer, Gcxp::gcxpVersion);
        BOOST_CHECK_EQUAL(len, Gcxp::Codec::preambleReadAmount);
        std::string got(std::begin(buffer), std::end(buffer));
        BOOST_CHECK_EQUAL(got.length(), Gcxp::Codec::preambleReadAmount);
        BOOST_CHECK_EQUAL(got, std::string("\xd9\xd9\xf7\x00", 4));
    });
    BOOST_CHECK_NO_THROW({
        auto version = ~Gcxp::gcxpVersion;
        auto pos = std::begin(buffer);
        auto len = Gcxp::Codec::decodeVersion(pos, std::end(buffer), version);
        BOOST_CHECK_EQUAL(len, Gcxp::Codec::preambleReadAmount);
        BOOST_CHECK_NO_THROW(Gcxp::checkVersion(version));
    });
}

BOOST_AUTO_TEST_CASE(codec) {
    const std::vector<const std::tuple<const Gcxp::Message, const std::string, int>> cases{
        {{Gcxp::Message::Type::request, false, {'@', '@', '@', '@'}, {' '}},
            "\xd8\x18\x49"
            "\x83"
            "\x02"
            "\x44\x40\x40\x40\x40"
            "\x61\x20",
            3},
        {{Gcxp::Message::Type::response, true, {'@', '@', '@', '@'}, {}},
            "\xd8\x18\x48"
            "\x83"
            "\x03"
            "\xf5"
            "\x44\x40\x40\x40\x40",
            3},
        {{Gcxp::Message::Type::response, false, {'@', '@', '@', '@'}, {}},
            "\xd8\x18\x48"
            "\x83"
            "\x03"
            "\xf4"
            "\x44\x40\x40\x40\x40",
            3},
    };

    for (const auto& test : cases) {
        std::string expect(std::get<1>(test), std::get<2>(test));
        BOOST_CHECK_NO_THROW({
            std::vector<char> buffer;
            auto len = Gcxp::Codec::encodeMessage(buffer, std::get<0>(test));
            BOOST_CHECK_EQUAL(len, expect.size());
            std::string got(std::begin(buffer), std::end(buffer));
            BOOST_CHECK_EQUAL(got, expect);
        });
        BOOST_CHECK_NO_THROW({
            Gcxp::Message msg;
            auto pos = std::begin(expect);
            auto len = Gcxp::Codec::decodeMessage(pos, std::end(expect), msg);
            BOOST_CHECK(pos == std::end(expect));
            BOOST_CHECK_EQUAL(len, expect.size());
            BOOST_CHECK_EQUAL(msg, std::get<0>(test));
        });
        BOOST_CHECK_NO_THROW({
            std::vector<char> buffer;
            auto len = Gcxp::Codec::encodeFrame(buffer, expect.size());
            auto prefixLen = std::get<2>(test);
            BOOST_CHECK_EQUAL(len, prefixLen);
            std::string got(buffer.begin(), buffer.end());
            BOOST_CHECK_EQUAL(got, std::get<1>(test).substr(0, prefixLen));
        });
        BOOST_CHECK_NO_THROW({
            std::size_t got = 0;
            auto pos = std::begin(std::get<1>(test));
            auto prefixLen = std::get<2>(test);
            auto len = Gcxp::Codec::decodeFrame(pos, pos + prefixLen, got);
            BOOST_CHECK_EQUAL(std::distance(std::begin(std::get<1>(test)), pos), prefixLen);
            BOOST_CHECK_EQUAL(len, prefixLen);
            BOOST_CHECK_EQUAL(got, expect.size());
        });
        BOOST_CHECK_NO_THROW({
            std::vector<char> buffer;
            auto len = CborLite::encodeEncodedBytes(buffer, expect);
            BOOST_CHECK_EQUAL(len, std::get<1>(test).size());
            std::string got(buffer.begin(), buffer.end());
            BOOST_CHECK_EQUAL(got, std::get<1>(test));
        });
        BOOST_CHECK_NO_THROW({
            std::string got;
            auto pos = std::begin(std::get<1>(test));
            auto len = CborLite::decodeEncodedBytes(pos, std::end(std::get<1>(test)), got);
            BOOST_CHECK(pos == std::end(std::get<1>(test)));
            BOOST_CHECK_EQUAL(len, std::get<1>(test).size());
            BOOST_CHECK_EQUAL(got, expect);
        });
    }
}

BOOST_AUTO_TEST_CASE(invalidMessage) {
    const std::vector<const std::pair<const Gcxp::Message, const std::string>> cases{
        {{Gcxp::Message::Type::invalid, false, {'@', '@', '@', '@'}, {' '}}, "GCXP Exception: bad Type"},
        {{Gcxp::Message::Type::notice, false, {'@', '@', '@', '@'}, {' '}}, "GCXP Exception: bad Type"},
        {{Gcxp::Message::Type::request, true, {'@', '@', '@', '@'}, {' '}},
            "GCXP Exception: accepted true but type is not response"},
        {{Gcxp::Message::Type::response, false, {}, {' '}}, "GCXP Exception: id is empty"},
    };

    for (const auto& test : cases) {
        BOOST_CHECK_EXCEPTION(
            {
                std::vector<char> buffer;
                (void)Gcxp::Codec::encodeMessage(buffer, test.first);
            },
            Gcxp::Exception,
            [&](Gcxp::Exception const& e) {
                BOOST_CHECK_EQUAL(e.what(), test.second);
                return e.what() == test.second;
            });
    }
}

BOOST_AUTO_TEST_CASE(invalidMessageEncoding) {
    const std::vector<const std::pair<const std::string, const std::string>> cases{
        {"\x81\x01", "GCXP Exception: array size too small"},
        {"\x85\x02", "GCXP Exception: array size too large"},
        {"\x83", "CBOR Exception: not enough input"},
        {"\x83\x80", "CBOR Exception: Not a Unsigned"},
        {"\x83\x01", "GCXP Exception: bad message type"},
        {"\x83\x04", "GCXP Exception: bad message type"},
        {"\x83\x02", "CBOR Exception: not enough input"},
        {"\x83\x03", "CBOR Exception: not enough input"},
        {"\x83\x02\xf4", "CBOR Exception: Not Bytes"},
        {"\x83\x02\xf5", "CBOR Exception: Not Bytes"},
        {"\x83\x02\x40", "GCXP Exception: bad Id length"},
        {"\x83\x02\x41", "CBOR Exception: not enough input"},
        {"\x83\x02\x42@", "CBOR Exception: not enough input"},
        {"\x83\x02\x41@\x61", "CBOR Exception: not enough input"},
        {"\x83\x02\x41@\x62 ", "CBOR Exception: not enough input"},
        {"\x83\x02\x41@\x41 ", "CBOR Exception: Not text"},
        {"\x83\x02\x41@\x60", "GCXP Exception: empty payload"},
        {"\x83\x03\x40", "CBOR Exception: Not a Boolean"},
        {"\x83\x03\xf3", "CBOR Exception: Not a Boolean"},
        {"\x83\x03\xf6", "CBOR Exception: Not a Boolean"},
        {"\x83\x03\xf4\x40", "GCXP Exception: bad Id length"},
        {"\x83\x03\xf4\x41", "CBOR Exception: not enough input"},
        {"\x83\x03\xf5\x42@", "CBOR Exception: not enough input"},
        {"\x84\x03\xf4\x41@\x61", "CBOR Exception: not enough input"},
        {"\x84\x03\xf4\x41@\x62 ", "CBOR Exception: not enough input"},
        {"\x84\x03\xf4\x41@\x41 ", "CBOR Exception: Not text"},
        {"\x84\x03\xf4\x41@\x60", "GCXP Exception: empty payload"},
    };

    for (const auto& test : cases) {
        BOOST_CHECK_EXCEPTION(
            {
                Gcxp::Message msg;
                auto pos = std::begin(test.first);
                (void)Gcxp::Codec::decodeMessage(pos, std::end(test.first), msg);
            },
            std::exception,
            [&](std::exception const& e) {
                BOOST_CHECK_EQUAL(e.what(), test.second);
                return e.what() == test.second;
            });
    }
}

BOOST_AUTO_TEST_CASE(invalidFrameEncoding) {
    const std::vector<const std::pair<const std::string, const std::string>> cases{
        {"\x49\x83\x02\x44\x40\x40\x40\x40\x61\x20", "CBOR Exception: Not a CBOR Encoded Data"},
        {"\xd8\x19", "CBOR Exception: Not a CBOR Encoded Data"},
        {"\xd8\x18\x80", "CBOR Exception: Not Bytes"},
        {"\xd8", "CBOR Exception: not enough input"},
        {"\xd8\x18", "CBOR Exception: not enough input"},
        {"\xd8\x18\x58", "CBOR Exception: not enough input"},
    };

    for (const auto& test : cases) {
        BOOST_CHECK_EXCEPTION(
            {
                std::size_t got = 0;
                auto pos = std::begin(test.first);
                (void)Gcxp::Codec::decodeFrame(pos, std::end(test.first), got);
            },
            std::exception,
            [&](std::exception const& e) {
                BOOST_CHECK_EQUAL(e.what(), test.second);
                return e.what() == test.second;
            });
    }
}

BOOST_AUTO_TEST_SUITE_END()
