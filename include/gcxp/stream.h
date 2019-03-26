#pragma once
#include <gcxp/codec.h>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/bind.hpp>
#include <boost/tuple/tuple.hpp>
#include <cassert>
#include <sstream>
#include <vector>

namespace Gcxp {

// Content Exchange Protocol Stream
using Buffer = std::vector<char>;

class Stream {
public:
    Stream(boost::asio::io_service& io_service, boost::asio::ssl::context& tls) : socket_(io_service, tls) {
    }

    Stream(boost::asio::ip::tcp::socket tcpSocket, boost::asio::ssl::context& tls) : socket_(tcpSocket.get_io_service(), tls) {
        socket_.lowest_layer() = std::move(tcpSocket);
    }

    ~Stream() noexcept = default;

    Stream(const Stream&) = delete;
    Stream& operator=(const Stream&) = delete;
    Stream(Stream&&) = delete;
    Stream& operator=(Stream&&) = delete;

    boost::asio::ssl::stream<boost::asio::ip::tcp::socket>& socket() noexcept {
        return socket_;
    }

    template <typename Handler>
    void asyncWrite(size_t version, Handler handler) {
        outboundFrame_.clear();
        outboundMessage_.clear();
        if (version != gcxpVersion) throw Exception("internal version mismatch");
        auto len = Codec::encodeVersion(outboundFrame_, version);
        assert(len == outboundFrame_.size());
        assert(len == Codec::preambleReadAmount);

        std::vector<boost::asio::const_buffer> buffers;
        buffers.push_back(boost::asio::buffer(outboundFrame_));
        boost::asio::async_write(socket_, buffers, handler);
    }

    template <typename Handler>
    void asyncWrite(const Message& m, Handler handler) {
        outboundFrame_.clear();
        outboundMessage_.clear();
        auto len = Codec::encodeMessage(outboundMessage_, m);
        assert(len == outboundMessage_.size());
        len = Codec::encodeFrame(outboundFrame_, len);
        assert(len == outboundFrame_.size());

        std::vector<boost::asio::const_buffer> buffers;
        buffers.push_back(boost::asio::buffer(outboundFrame_));
        buffers.push_back(boost::asio::buffer(outboundMessage_));
        boost::asio::async_write(socket_, buffers, handler);
    }

    template <typename Type, typename Handler>
    typename std::enable_if<std::is_unsigned<Type>::value, void>::type asyncRead(Type& t, Handler handler) {
        void (Stream::*f)(const boost::system::error_code&, Type&, boost::tuple<Handler>) = &Stream::handleRead<Type, Handler>;
        inboundFrame_.resize(Codec::preambleReadAmount);
        boost::asio::async_read(socket_, boost::asio::buffer(inboundFrame_),
            boost::bind(f, this, boost::asio::placeholders::error, boost::ref(t), boost::make_tuple(handler)));
    }

    template <typename Type, typename Handler>
    typename std::enable_if<std::is_unsigned<Type>::value, void>::type handleRead(
        const boost::system::error_code& e, Type& t, boost::tuple<Handler> handler) {
        if (e) return boost::get<0>(handler)(e);

        try {
            auto pos = std::begin(inboundFrame_);
            auto end = std::end(inboundFrame_);
            Codec::decodeVersion(pos, end, t, Codec::flags);
            if (pos != end) throw Exception("Improper preamble");

        } catch (...) {
            boost::system::error_code error(boost::asio::error::invalid_argument);
            return boost::get<0>(handler)(error);
        }

        boost::get<0>(handler)(e);
    }

    template <typename Type, typename Handler>
    typename std::enable_if<!std::is_unsigned<Type>::value, void>::type asyncRead(Type& t, Handler handler) {
        void (Stream::*f)(const boost::system::error_code&, Type&, boost::tuple<Handler>) = &Stream::handleRead<Type, Handler>;
        inboundFrame_.resize(Codec::frameReadAmount);
        boost::asio::async_read(socket_, boost::asio::buffer(inboundFrame_),
            boost::bind(f, this, boost::asio::placeholders::error, boost::ref(t), boost::make_tuple(handler)));
    }

    template <typename Type, typename Handler>
    typename std::enable_if<!std::is_unsigned<Type>::value, void>::type handleRead(
        const boost::system::error_code& e, Type& t, boost::tuple<Handler> handler) {
        if (e) return boost::get<0>(handler)(e);
        uint64_t offset = 0;
        try {
            inboundMessage_.resize(0);
            auto pos = std::begin(inboundFrame_);
            auto end = std::end(inboundFrame_);
            size_t msgLen;
            auto len = Codec::decodeFrame(pos, end, msgLen, Codec::flags);
            auto leftover = inboundFrame_.size() - len;
            assert(leftover == static_cast<decltype(leftover)>(std::distance(pos, end)));
            if (leftover > msgLen) throw "Undersized message";
            if (leftover) inboundMessage_.insert(std::begin(inboundMessage_), pos, end);
            inboundMessage_.resize(msgLen);
            if (leftover == msgLen) return handleReadMessage(e, t, handler);
            offset = leftover;

        } catch (...) {
            boost::system::error_code error(boost::asio::error::invalid_argument);
            return boost::get<0>(handler)(error);
        }

        void (Stream::*f)(const boost::system::error_code&, Message&, boost::tuple<Handler>) =
            &Stream::handleReadMessage<Type, Handler>;
        boost::asio::async_read(socket_, boost::asio::buffer(inboundMessage_.data() + offset, inboundMessage_.size() - offset),
            boost::bind(f, this, boost::asio::placeholders::error, boost::ref(t), handler));
    }

    template <typename Type, typename Handler>
    void handleReadMessage(const boost::system::error_code& e, Type& t, boost::tuple<Handler> handler) {
        if (e) return boost::get<0>(handler)(e);

        try {
            auto pos = std::begin(inboundMessage_);
            auto end = std::end(inboundMessage_);
            Codec::decodeMessage(pos, end, t, Codec::flags);
            if (pos != end) throw Exception("Improper framing");

        } catch (...) {
            boost::system::error_code error(boost::asio::error::invalid_argument);
            return boost::get<0>(handler)(error);
        }

        boost::get<0>(handler)(e);
    }

private:
    Buffer inboundFrame_;
    Buffer inboundMessage_;
    Buffer outboundFrame_;
    Buffer outboundMessage_;
    boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_;
};

} // namespace Gcxp
