#pragma once
#include <iomanip>
#include <iterator>
#include <sstream>
#include <vector>

namespace Gcxp {
class Message {
public:
    using Id = std::vector<unsigned char>;
    using Payload = std::vector<char>;
    enum class Type { invalid = 0, notice, request, response };

    Type type = Type::invalid;
    bool accepted = false;
    Id id;
    Payload payload;

    template <typename Type>
    static typename std::enable_if<std::is_arithmetic<Type>::value, Id>::type constructId(Type t) {
        Id id;
        id.resize(sizeof t);
        for (auto i = sizeof t; i; t >>= 8) {
            id[--i] = t & 0xFFu;
        }
        return id;
    }

    inline static std::string idToString(const Gcxp::Message::Id& id) {
        const std::string hex = "0123456789abcdef";
        if (id.empty()) return "<EMPTY>";

        std::string s;
        // reserving floor(id.size()*2.25+1) would be sufficient but id.size()*3 avoids floating-point math
        s.reserve(id.size() * 3);
        unsigned i = 0;
        for (unsigned ch : id) {
            s += hex[(ch >> 4) & 0x0Fu];
            s += hex[ch & 0x0Fu];
            if ((++i) % 4 == 0) s += ':';
        }
        if (s[s.size() - 1] == ':') s.pop_back();
        return s;
    }

    inline static std::string payloadToString(const Gcxp::Message::Payload& payload) {
        const std::string hex = "0123456789abcdef";
        if (payload.empty()) return "";

        std::string s;
        s.reserve(payload.size() * 2);
        for (unsigned ch : payload) {
            s += hex[(ch >> 4) & 0x0Fu];
            s += hex[ch & 0x0Fu];
        }
        return s;
    }

    bool operator==(const Message& other) const {
        if (type != other.type) return false;
        if (accepted != other.accepted) return false;
        if (id != other.id) return false;
        if (payload != other.payload) return false;
        return true;
    }
};

inline std::ostream& operator<<(std::ostream& out, Message::Type type) {
    switch (type) {
    case Message::Type::invalid:
        out << "Invalid";
        break;
    case Message::Type::notice:
        out << "Notice";
        break;
    case Message::Type::request:
        out << "Request";
        break;
    case Message::Type::response:
        out << "Response";
        break;
    default:
        out << "Unknown(" << static_cast<std::underlying_type<Message::Type>::type>(type) << ")";
        break;
    }
    return out;
}

inline std::ostream& operator<<(std::ostream& out, const Message& m) {
    switch (m.type) {
    case Message::Type::request:
        out << "REQ ";
        break;
    case Message::Type::response:
        out << "RSP ";
        out << " A:" << std::string(m.accepted ? "T" : "F");
        break;
    default:
        out << "INV ";
    }
    out << " I:" << Message::idToString(m.id);
    if (!m.payload.empty()) out << " P:" << Message::payloadToString(m.payload);
    return out;
}
} // namespace Gcxp
