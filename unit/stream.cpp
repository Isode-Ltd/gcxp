// This file is part of GCXP which is copyright Isode Limited
// and others and released under a MIT license. For details, see the
// COPYRIGHT.md file in the top-level folder of the GCXP software
// distribution.
#include <gcxp/stream.h>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(gcxp)
BOOST_AUTO_TEST_SUITE(stream)

BOOST_AUTO_TEST_CASE(server) {
    BOOST_CHECK_NO_THROW({
        boost::asio::io_service io_service;
        boost::asio::ssl::context tls(boost::asio::ssl::context::tlsv12_server);
        boost::asio::ip::tcp::socket tcpSocket{io_service};
        BOOST_CHECK_EQUAL(tcpSocket.is_open(), false);
        Gcxp::Stream stream(std::move(tcpSocket), tls);
        auto& s = stream.socket();
        auto& sd = s.lowest_layer();
        BOOST_CHECK_EQUAL(&sd.get_io_service(), &io_service);
    });
}

BOOST_AUTO_TEST_CASE(client) {
    BOOST_CHECK_NO_THROW({
        boost::asio::io_service io_service;
        boost::asio::ssl::context tls(boost::asio::ssl::context::tlsv12_client);
        Gcxp::Stream stream(io_service, tls);
        auto& s = stream.socket();
        auto& sd = s.lowest_layer();
        BOOST_CHECK_EQUAL(&sd.get_io_service(), &io_service);
    });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
