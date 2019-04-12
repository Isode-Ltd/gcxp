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
    std::vector<std::tuple<Gcxp::Message, std::string, int>> cases{
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
            auto prefix = std::get<2>(test);
            BOOST_CHECK_EQUAL(len, prefix);
            std::string got(buffer.begin(), buffer.end());
            BOOST_CHECK_EQUAL(got, std::get<1>(test).substr(0, prefix));
        });
        BOOST_CHECK_NO_THROW({
            std::size_t got = 0;
            auto pos = std::begin(std::get<1>(test));
            auto prefix = std::get<2>(test);
            auto len = Gcxp::Codec::decodeFrame(pos, pos + prefix, got);
            BOOST_CHECK_EQUAL(std::distance(std::begin(std::get<1>(test)), pos), prefix);
            BOOST_CHECK_EQUAL(len, prefix);
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

BOOST_AUTO_TEST_SUITE_END()
