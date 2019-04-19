#pragma once
#include <gcxp/exception.h>
#include <gcxp/message.h>
#include <cbor-lite/codec.h>
#include <sstream>
#include <string>

namespace Gcxp {
using Version = std::uint_fast64_t;
constexpr Version gcxpVersion = 0;

template <typename Type>
void checkVersion(Type v) {
    if (v != gcxpVersion) throw Exception("GCXP version mismatch");
}

namespace Codec {
constexpr CborLite::Flags flags = CborLite::Flag::requireMinimalEncoding;

constexpr std::size_t preambleReadAmount = 4;

template <typename Buffer, typename Type>
typename std::enable_if<std::is_class<Buffer>::value, std::size_t>::type encodeVersion(Buffer& buffer, Type t) {
    auto len = CborLite::encodeTagAndValue(buffer, CborLite::Major::semantic, CborLite::Minor::selfDescribeCbor);
    return len + CborLite::encodeUnsigned(buffer, t);
}

template <typename InputIterator, typename Type>
typename std::enable_if<std::is_class<InputIterator>::value, std::size_t>::type decodeVersion(
    InputIterator& pos, InputIterator end, Type& t, CborLite::Flags flags = CborLite::Flag::none) {
    CborLite::Tag tag;
    CborLite::Tag value;
    auto len = CborLite::decodeTagAndValue(pos, end, tag, value, flags);
    if (tag != CborLite::Major::semantic || value != CborLite::Minor::selfDescribeCbor) {
        throw Exception("Expected self-describe CBOR");
    }
    return len + CborLite::decodeUnsigned(pos, end, t, flags);
}

constexpr auto frameReadAmount = 7;

template <typename Buffer, typename Type>
typename std::enable_if<std::is_class<Buffer>::value, std::size_t>::type encodeFrame(Buffer& buffer, Type t) {
    return CborLite::encodeEncodedBytesPrefix(buffer, t);
}

template <typename InputIterator, typename Type>
typename std::enable_if<std::is_class<InputIterator>::value, std::size_t>::type decodeFrame(
    InputIterator& pos, InputIterator end, Type& t, CborLite::Flags flags = CborLite::Flag::none) {
    return CborLite::decodeEncodedBytesPrefix(pos, end, t, flags);
}

template <typename Buffer>
static typename std::enable_if<std::is_class<Buffer>::value, std::size_t>::type encodeMessage(Buffer& buffer, const Message& m) {
    std::size_t len = 0;
    switch (m.type) {
    case Message::Type::request:
        len = CborLite::encodeArraySize(buffer, m.payload.empty() ? 2ul : 3ul);
        break;
    case Message::Type::response:
        len = CborLite::encodeArraySize(buffer, m.payload.empty() ? 3ul : 4ul);
        break;
    default:
        throw Exception("bad Type");
    }
    len += CborLite::encodeUnsigned(buffer, static_cast<unsigned long>(m.type));
    if (m.type == Message::Type::response) {
        len += CborLite::encodeBool(buffer, m.accepted);
    } else if (m.accepted) {
        throw Exception("accepted true but type is not response");
    }
    if (m.id.empty()) throw Exception("id is empty");
    len += CborLite::encodeBytes(buffer, m.id);
    if (!m.payload.empty()) len += CborLite::encodeText(buffer, m.payload);
    return len;
}

template <typename InputIterator>
typename std::enable_if<std::is_class<InputIterator>::value, std::size_t>::type decodeMessage(
    InputIterator& pos, InputIterator end, Message& m, CborLite::Flags flags = CborLite::Flag::none) {
    std::size_t nItems = 0u;
    std::size_t len = CborLite::decodeArraySize(pos, end, nItems, flags);
    if (nItems < 2) throw Exception("array size too small");
    if (nItems > 4) throw Exception("array size too large");
    unsigned long type;

    len += CborLite::decodeUnsigned(pos, end, type, flags);
    nItems--;
    m.type = static_cast<Message::Type>(type);
    switch (m.type) {
    case Message::Type::request:
    case Message::Type::response:
        break;
    default:
        throw Exception("bad message type");
    }

    switch (m.type) {
    case Message::Type::response:
        len += CborLite::decodeBool(pos, end, m.accepted, flags);
        nItems--;
        break;
    default:;
    }
    if (!nItems--) throw Exception("too few items");
    len += CborLite::decodeBytes(pos, end, m.id);
    if (m.id.size() < 1 || m.id.size() > 256) throw Exception("bad Id length");
    if (m.id.empty()) throw Exception("empty id");

    if (!nItems) return len;
    len += CborLite::decodeText(pos, end, m.payload, flags);
    if (m.payload.empty()) throw Exception("empty payload");
    if (--nItems) throw Exception("too many items");
    return len;
}
} // namespace Codec
} // namespace Gcxp
