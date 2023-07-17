// This file is part of GCXP which is copyright Isode Limited
// and others and released under a MIT license. For details, see the
// COPYRIGHT.md file in the top-level folder of the GCXP software
// distribution.

//
// producer program: half of the producer/consumer pair of testing programs
//
#include <gcxp/message.h>
#include <gcxp/stream.h>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/lexical_cast.hpp>
#include <algorithm>
#include <deque>
#include <iostream>
#include <thread>

namespace Producer {
std::string peername("consumer.z");

class Connection {
public:
    Connection(boost::asio::io_service& io_service, boost::asio::ssl::context& tls)
        : stream_(io_service, tls), status_(Status::Inactive) {
    }

    enum class Status { Inactive, Connecting, Handshaking, Active, Terminating, Terminated };
    Status status() {
        return status_;
    }

    void connect(boost::asio::ip::tcp::resolver::results_type& endpoints) {
        status_ = Status::Connecting;
        boost::asio::async_connect(stream_.socket().lowest_layer(), endpoints,
            [this](const boost::system::error_code& e, const boost::asio::ip::tcp::endpoint&) {
                if (e) {
                    std::cerr << "Stream connect: " << e.message() << "\n";
                    status(Status::Terminated);
                    return;
                }
                status(Status::Handshaking);
                stream_.socket().lowest_layer().set_option(boost::asio::ip::tcp::no_delay(true));
                stream_.socket().async_handshake(boost::asio::ssl::stream_base::client, [this](const boost::system::error_code& e) {
                    if (e) {
                        std::cerr << "Stream TLS handshake: " << e.message() << "\n";
                        stream_.socket().lowest_layer().close();
                        status(Status::Terminated);
                        return;
                    }
                    status(Status::Active);
                    std::cerr << "Stream TLS handshake: completed\n";
                    stream_.asyncWrite(
                        Gcxp::gcxpVersion, [this](const boost::system::error_code& e, std::size_t) { handleWrite(e); });
                    stream_.asyncRead(preamble_, [this](const boost::system::error_code& e) {
                        if (e || preamble_ != Gcxp::gcxpVersion) {
                            status(Status::Terminating);
                            std::cerr << "Stream Preamble: " << (e ? e.message() : "version mismatch") << "\n";
                            stream_.socket().async_shutdown([this](const boost::system::error_code& e) { handleShutdown(e); });
                            return;
                        }
                        std::cerr << "Stream starting: GCXP version: " << boost::lexical_cast<std::string>(preamble_) << "\n";
                        stream_.asyncRead(msg_, [this](const boost::system::error_code& e) { handleRead(e); });
                    });
                });
            });
    }

    void stop() {
        stream_.socket().lowest_layer().cancel();
    }

    void write(const Gcxp::Message& req) {
        stream_.asyncWrite(req, [this](const boost::system::error_code& e, std::size_t) { handleWrite(e); });
    }

private:
    void handleRead(const boost::system::error_code& e) {
        if (e) {
            status(Status::Terminating);
            std::cerr << "Stream Read: " << e.message() << "\n";
            stream_.socket().async_shutdown([this](const boost::system::error_code& e) { handleShutdown(e); });
            return;
        }

        if (msg_.type != Gcxp::Message::Type::response) {
            std::cout << "Type: " << msg_.type << "\n";
        } else {
            std::cout << "Accepted: " << std::boolalpha << msg_.accepted << std::noboolalpha << "\n";
        }
        if (msg_.payload.size()) {
            std::cout << "Payload: " << std::string(msg_.payload.data(), msg_.payload.size()) << "\n";
        } else {
            std::cout << "No payload.\n";
        }
        msg_ = Gcxp::Message();
        stream_.asyncRead(msg_, [this](const boost::system::error_code& e) { handleRead(e); });
    }

    void handleWrite(const boost::system::error_code& e) {
        if (e) {
            status(Status::Terminating);
            std::cerr << "Stream Write: " << e.message() << "\n";
            return stream_.socket().lowest_layer().cancel();
        }
    }

    void handleShutdown(const boost::system::error_code& e) {
        if (e && e.value() != boost::asio::error::eof) {
            std::cerr << "Stream TLS shutdown: " << e.message() << "\n";
        }
        status_ = Status::Terminated;
        std::cerr << "Stream TLS shutdown completed.\n";
        stream_.socket().lowest_layer().close();
    }

    void status(Status status) {
        status_ = status;
    }

    Gcxp::Stream stream_;
    Gcxp::Version preamble_;
    Gcxp::Message msg_;
    Status status_;
};
} // namespace Producer

int main(int argc, char* argv[]) {
    try {
        if (argc == 4) {
            if (std::string(argv[1]) == std::string("-g")) {
                Producer::peername = "guard.z";
                argc--;
                argv++;
            }
        }
        if (argc != 3) {
            std::cerr << "Usage: [-g] producer <address> <port>\n";
            return EXIT_FAILURE;
        }
        const auto address = argv[1];
        const auto port = argv[2];

        boost::asio::io_service io_service;
        boost::asio::ip::tcp::resolver resolver(io_service);
        auto endpoints = resolver.resolve({address, port});

        boost::asio::ssl::context tls(boost::asio::ssl::context::tls_client);
        auto native_handle = tls.native_handle();
        SSL_CTX_set_min_proto_version(native_handle, TLS1_3_VERSION);
        {
            auto param = X509_VERIFY_PARAM_new();
            X509_VERIFY_PARAM_set_flags(param, X509_V_FLAG_PARTIAL_CHAIN);
            SSL_CTX_set1_param(native_handle, param);
            X509_VERIFY_PARAM_free(param);
        }
        if (SSL_CTX_config(native_handle, "gcxp") == 0) {
            std::cerr << "Unable to load GCXP TLS configuration\n";
            return EXIT_FAILURE;
        }
        tls.set_verify_mode(boost::asio::ssl::verify_peer);
        tls.set_verify_callback(boost::asio::ssl::host_name_verification(Producer::peername));

        Producer::Connection c(io_service, tls);
        c.connect(endpoints);

        // note: access to c from thread t and the main thread should be synchronized.
        std::thread t([&io_service]() { io_service.run(); });

        for (auto i = 1; c.status() < Producer::Connection::Status::Active; i *= 2) {
            std::cerr << "Awaiting connection...\n";
            sleep(i);
        };

        if (c.status() != Producer::Connection::Status::Active) {
            std::cerr << "Could not connect!\n";

        } else {
            std::cerr << "Connected!\n";
            std::vector<char> line(4096);
            for (auto i = 1; std::cin.getline(line.data(), line.size()); ++i) {
                if (c.status() != Producer::Connection::Status::Active) {
                    std::cerr << "No longer connected!\n";
                    break;
                }

                auto len = std::cin.gcount();
                if (!len--) continue; // -- to eat then nul termination

                Gcxp::Message msg;
                msg.id = Gcxp::Message::constructId(i);
                msg.type = Gcxp::Message::Type::request;
                msg.payload.assign(line.begin(), line.begin() + len);
                c.write(msg);
            }

            if (c.status() == Producer::Connection::Status::Active) {
                std::cerr << "Closing connection...\n";
                c.stop();
            }
        }

        t.join();
        return EXIT_SUCCESS;

    } catch (const Gcxp::Exception& e) {
        std::cerr << e.what() << "\n";
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    } catch (...) {
        std::cerr << "Unknown exception\n";
    }
    return EXIT_FAILURE;
}
