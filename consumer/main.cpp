//
// consumer program: half of the producer/consumer pair of testing programs
//
#include <gcxp/message.h>
#include <gcxp/stream.h>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/lexical_cast.hpp>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <utility>

namespace Consumer {
std::string peername("producer.z");

class Connection {
public:
    Connection(boost::asio::ip::tcp::socket socket, boost::asio::ssl::context& tls) : stream_(std::move(socket), tls) {
    }

    void start() {
        stream_.socket().lowest_layer().set_option(boost::asio::ip::tcp::no_delay(true));
        stream_.socket().set_verify_mode(boost::asio::ssl::verify_peer);
        stream_.socket().set_verify_callback(boost::asio::ssl::rfc2818_verification(peername));

        stream_.socket().async_handshake(boost::asio::ssl::stream_base::server,
            boost::bind(&Connection::handleHandshake, this, boost::asio::placeholders::error));
    }

    void write(const Gcxp::Message& msg) {
        stream_.asyncWrite(msg, boost::bind(&Connection::handleWrite, this, boost::asio::placeholders::error));
    }

private:
    void handleHandshake(const boost::system::error_code& e) {
        if (e) {
            std::cerr << "Stream TLS handshake: " << e.message() << "\n";
            return stream_.socket().lowest_layer().close();
        }

        std::cerr << "Stream TLS handshake: completed\n";

        stream_.asyncWrite(Gcxp::gcxpVersion, boost::bind(&Connection::handleWrite, this, boost::asio::placeholders::error));
        stream_.asyncRead(preamble_, boost::bind(&Connection::handlePreamble, this, boost::asio::placeholders::error));
    }

    void handlePreamble(const boost::system::error_code& e) {
        if (e.value() == boost::asio::error::operation_aborted) {
            std::cerr << "Stream Preamble aborted... shutting down TLS...\n";
            stream_.socket().async_shutdown(boost::bind(&Connection::handleShutdown, this, boost::asio::placeholders::error));
            return;
        }

        if (e) {
            std::cerr << "Stream Preamble Read: " << e.message() << "\n";
            stream_.socket().async_shutdown(boost::bind(&Connection::handleShutdown, this, boost::asio::placeholders::error));
            return;
        }

        std::cout << "GCXP version=" << boost::lexical_cast<std::string>(preamble_) << "\n";
        stream_.asyncRead(msg_, boost::bind(&Connection::handleRead, this, boost::asio::placeholders::error));
    }

    void handleRead(const boost::system::error_code& e) {
        if (e.value() == boost::asio::error::operation_aborted) {
            std::cerr << "Stream (read) operation aborted... shutting down TLS...\n";
            stream_.socket().async_shutdown(boost::bind(&Connection::handleShutdown, this, boost::asio::placeholders::error));
            return;
        }

        if (e) {
            std::cerr << "Stream Read: " << e.message() << "\n";
            stream_.socket().async_shutdown(boost::bind(&Connection::handleShutdown, this, boost::asio::placeholders::error));
            return;
        }

        std::cout << "id=" << Gcxp::Message::idToString(msg_.id)
                  << " payload=" << std::string(msg_.payload.data(), msg_.payload.size()) << "\n";

        Gcxp::Message rsp;
        rsp.id = std::move(msg_.id);
        rsp.type = Gcxp::Message::Type::response;
        rsp.accepted = true;
        rsp.payload = std::move(msg_.payload);
        write(rsp);
        msg_ = Gcxp::Message();

        stream_.asyncRead(msg_, boost::bind(&Connection::handleRead, this, boost::asio::placeholders::error));
    }

    void handleWrite(const boost::system::error_code& e) {
        if (e) {
            std::cerr << "Stream Write: " << e.message() << "\n";
            return stream_.socket().lowest_layer().cancel();
        }
    }

    void handleShutdown(const boost::system::error_code& e) {
        if (e && e.value() != boost::asio::error::eof) {
            std::cerr << "Stream Shutdown: " << e.message() << "\n";
        }
        std::cerr << "Stream TLS shutdown completed.\n";
        stream_.socket().lowest_layer().close();
    }

    Gcxp::Stream stream_;
    Gcxp::Version preamble_;
    Gcxp::Message msg_;
};

class Server {
public:
    Server(boost::asio::io_service& io_service, boost::asio::ssl::context& tls,
        boost::asio::ip::tcp::resolver::iterator& endpoint_iterator)
        : acceptor_(io_service), socket_(io_service), tls_(std::move(tls)) {
        boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
        acceptor_.open(endpoint.protocol());
        acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        acceptor_.bind(endpoint);
        acceptor_.listen();
        accept();
    }

private:
    void accept() {
        acceptor_.async_accept(socket_, boost::bind(&Server::handleAccept, this, boost::asio::placeholders::error));
    }

    void handleAccept(const boost::system::error_code& e) {
        std::cerr << "Incoming connection from " << socket_.remote_endpoint() << "\n";

        if (e) {
            std::cerr << "Accept failed:" << e.message() << "\n";

        } else {
            connection_ = std::make_shared<Connection>(std::move(socket_), tls_);
            connection_->start();
        }

        accept();
    }

    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::ip::tcp::socket socket_;
    boost::asio::ssl::context tls_;

    std::shared_ptr<Connection> connection_;
};

} // namespace Consumer

int main(int argc, char* argv[]) {
    try {
        if (argc == 4) {
            if (std::string(argv[1]) == std::string("-g")) {
                Consumer::peername = "guard.z";
                argc--;
                argv++;
            }
        }
        if (argc != 3) {
            std::cerr << "Usage: consumer <address> <port>\n";
            return EXIT_FAILURE;
        }

        const auto address = argv[1];
        const auto port = argv[2];

        boost::asio::io_service io_service;
        boost::asio::ip::tcp::resolver resolver(io_service);
        auto endpoint_iterator = resolver.resolve({address, port});

        boost::asio::ssl::context tls(boost::asio::ssl::context::tlsv12_server);
        tls.set_options(boost::asio::ssl::context::default_workarounds | boost::asio::ssl::context::single_dh_use);

        tls.load_verify_file("./consumer/trust.pem");
        tls.set_password_callback([](size_t, boost::asio::ssl::context::password_purpose) { return "secret"; });
        tls.use_certificate_chain_file("./consumer/identity.pem");
        tls.use_private_key_file("./consumer/identity.pem", boost::asio::ssl::context::pem);
        tls.use_tmp_dh_file("consumer/dh2048.pem");

        Consumer::Server server(io_service, tls, endpoint_iterator);
        io_service.run();
        return EXIT_SUCCESS;

    } catch (Gcxp::Exception& e) {
        std::cerr << e.what() << "\n";

    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";

    } catch (...) {
        std::cerr << "Unknown exception\n";
    }

    return EXIT_FAILURE;
}
