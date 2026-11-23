#pragma once
// This file is part of GCXP which is copyright Isode Limited
// and others and released under a MIT license. For details, see the
// COPYRIGHT.md file in the top-level folder of the GCXP software
// distribution.
#include <iomanip>
#include <iterator>
#include <sstream>
#include <vector>

namespace Gcxp {
class Message {
public:
    using Id = std::vector<unsigned char>;
    using Payload = std::vector<char>;
    enum class Type { invalid = 0, notice, request, response, healthcheck };
    enum class HealthcheckType : char { check = 'c', response = 'r', error = 'e' };

    Type type = Type::invalid;
    bool accepted = false;
    Id id;
    Payload payload;

    template <typename Type>
    static typename std::enable_if<std::is_arithmetic<Type>::value, Id>::type constructId(Type t) {
        Id id(sizeof t);
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

    inline void setHealthcheckType(HealthcheckType type) {
        payload.assign({static_cast<char>(type)});
    }

    inline HealthcheckType getHealthcheckType() const {
        if (payload.empty()) throw std::runtime_error("Payload is empty");
        return static_cast<HealthcheckType>(payload[0]);
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
        out << "REQ";
        if (m.accepted) out << " A:T";
        break;
    case Message::Type::response:
        out << "RSP A:" << std::string(m.accepted ? "T" : "F");
        break;
    case Message::Type::healthcheck:
        out << "HCQ ";
        if (!m.payload.empty()) {
            switch (m.getHealthcheckType()) {
            case Message::HealthcheckType::check:
                out << "CHECK";
                break;
            case Message::HealthcheckType::response:
                out << "RESP";
                break;
            case Message::HealthcheckType::error:
                out << "ERR";
                break;
            default:
                out << "UNKNOWN";
                break;
            }
        }
        break;
    default:
        out << "INV";
        if (m.accepted) out << " A:T";
    }
    out << " I:" << Message::idToString(m.id);
    if (!m.payload.empty()) out << " P:" << Message::payloadToString(m.payload);
    return out;
}
} // namespace Gcxp
