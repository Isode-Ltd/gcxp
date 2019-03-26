// Gcxp::Stream test cases
#include <gcxp/stream.h>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(gcxp)
BOOST_AUTO_TEST_SUITE(stream)

BOOST_AUTO_TEST_CASE(server) {
    boost::asio::io_service io_service;
    boost::asio::ssl::context tls(boost::asio::ssl::context::tlsv12_server);
    boost::asio::ip::tcp::socket tcpSocket{io_service};
    BOOST_CHECK_EQUAL(tcpSocket.is_open(), false);
    Gcxp::Stream stream(std::move(tcpSocket), tls);
    auto& s = stream.socket();
    auto& sd = s.lowest_layer();
    BOOST_CHECK_EQUAL(&sd.get_io_service(), &io_service);
}

BOOST_AUTO_TEST_CASE(client) {
    boost::asio::io_service io_service;
    boost::asio::ssl::context tls(boost::asio::ssl::context::tlsv12_client);
    Gcxp::Stream stream(io_service, tls);
    auto& s = stream.socket();
    auto& sd = s.lowest_layer();
    BOOST_CHECK_EQUAL(&sd.get_io_service(), &io_service);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
