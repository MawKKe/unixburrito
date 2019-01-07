#pragma once

#include <cpp.hpp>

#include <string>
#include <sstream>

using namespace cpp;

namespace _unix {

namespace inet {

// Top level enumeration classes for various network interfaces.
// The C-apis use enumerated variables, but their types are just defined int's, which
// are not type safe in any way. These enum *classes* will perhaps prevent stupid accidential
// mistakes from creeping into the code.

// There are a LOT of address families, but here are the most common
enum class AddressFamily : uint32_t { 
    Any     = AF_UNSPEC, 
    IPv4    = AF_INET, 
    IPv6    = AF_INET6, 
    /* Unix = AF_UNIX */ 
};
using AddressFamilyCheck = cpp::EnumCheck<AddressFamily,
      AddressFamily::Any,
      AddressFamily::IPv4,
      AddressFamily::IPv6>;

enum class SocketType : uint32_t {
    Any         = 0, 
    Datagram    = SOCK_DGRAM,
    Stream      = SOCK_STREAM,
    Raw         = SOCK_RAW,
};
using SocketTypeCheck = cpp::EnumCheck<SocketType,
      SocketType::Any,
      SocketType::Datagram,
      SocketType::Stream,
      SocketType::Raw>;

enum class Protocol : uint32_t {
    Any = 0,
    UDP = IPPROTO_UDP,
    TCP = IPPROTO_TCP
};
using ProtocolCheck = cpp::EnumCheck<Protocol,
      Protocol::Any,
      Protocol::UDP,
      Protocol::TCP>;

enum class AIFlag : uint32_t {
    // Even though these are enumerated, they are not mutually exclusive (like the 
    // other enums are)
    // TODO: probably non-exhaustive list. Check you OS's include files for more flags...
    Passive     = AI_PASSIVE,
    CanonName   = AI_CANONNAME,
    NumericHost = AI_NUMERICHOST,
    NumericServ = AI_NUMERICSERV,
    V4Mapped    = AI_V4MAPPED,
    All         = AI_ALL,
    AddrConfig  = AI_ADDRCONFIG,
};
using AIFlagCheck = cpp::EnumCheck<AIFlag,
	AIFlag::Passive,
	AIFlag::CanonName,
	AIFlag::NumericHost,
	AIFlag::NumericServ,
	AIFlag::V4Mapped,
	AIFlag::All,
	AIFlag::AddrConfig>;

enum class RecvFlag : uint32_t {
    DontWait = MSG_DONTWAIT,
    // TODO: augment me plz
};
using RecvFlagCheck = cpp::EnumCheck<RecvFlag, RecvFlag::DontWait>;

enum class SendFlag : uint32_t {
    Confirm     = MSG_CONFIRM,
    DontWait    = MSG_DONTWAIT,
    DontRoute   = MSG_DONTROUTE,
    EndOfRecord = MSG_EOR,
    More        = MSG_MORE,
    NoSignal    = MSG_NOSIGNAL,
    OutOfBounds = MSG_OOB,
};
using SendFlagCheck = cpp::EnumCheck<SendFlag,
      SendFlag::Confirm,
      SendFlag::DontWait,
      SendFlag::DontRoute,
      SendFlag::EndOfRecord,
      SendFlag::More,
      SendFlag::NoSignal,
      SendFlag::OutOfBounds>;

inline auto to_integral(AddressFamily af)   { return _to_integral<AddressFamilyCheck>(af);  }
inline auto to_integral(SocketType st)      { return _to_integral<SocketTypeCheck>(st);     }
inline auto to_integral(Protocol pt)        { return _to_integral<ProtocolCheck>(pt);       }
inline auto to_integral(AIFlag fl)          { return _to_integral<AIFlagCheck>(fl);         }
inline auto to_integral(RecvFlag rfl)       { return _to_integral<RecvFlagCheck>(rfl);      }
inline auto to_integral(SendFlag sfl)       { return _to_integral<SendFlagCheck>(sfl);      }

template <typename T>
inline auto to_enum(int);

template <>
inline auto to_enum<AddressFamily>(int v)   { return _to_enum<AddressFamilyCheck, AddressFamily>(v); }
template <>
inline auto to_enum<SocketType>(int v)      { return _to_enum<SocketTypeCheck, SocketType>(v);       }
template <>
inline auto to_enum<Protocol>(int v)        { return _to_enum<ProtocolCheck, Protocol>(v);           }
template <>
inline auto to_enum<AIFlag>(int v)          { return _to_enum<AIFlagCheck, AIFlag>(v);               }
template <>
inline auto to_enum<RecvFlag>(int v)        { return _to_enum<RecvFlagCheck, RecvFlag>(v);           }
template <>
inline auto to_enum<SendFlag>(int v)        { return _to_enum<SendFlagCheck, SendFlag>(v);           }


static inline Maybe<std::string> enum_name(AddressFamily af){
    using s = std::string;
    switch(af){
        case AddressFamily::Any:  return s("Protocol::Any");
        case AddressFamily::IPv4: return s("AddressFamily::IPv4");
        case AddressFamily::IPv6: return s("AddressFamily::IPv6");
    }
    return Nothing();
}
static inline Maybe<std::string> enum_name(SocketType t){
    using s = std::string;
    switch(t){
        case SocketType::Any:       return s("SocketType::Any");
        case SocketType::Datagram:  return s("SocketType::Datagram");
        case SocketType::Stream:    return s("SocketType::Stream");
        case SocketType::Raw:       return s("SocketType::Raw");
    }
    return Nothing();
}
static inline Maybe<std::string> enum_name(Protocol pt){
    using s = std::string;
    switch(pt){
        case Protocol::Any: return s("Protocol::Any");
        case Protocol::UDP: return s("Protocol::UDP");
        case Protocol::TCP: return s("Protocol::TCP");
    }
    return Nothing();
}
static inline const Maybe<std::string> enum_name(AIFlag f){
    using s = std::string;
    switch(f){
        case AIFlag::Passive:       return s("AIFlag::Passive");
        case AIFlag::CanonName:     return s("AIFlag::CanonName");
        case AIFlag::NumericHost:   return s("AIFlag::NumericHost");
        case AIFlag::NumericServ:   return s("AIFlag::NumericServ");
        case AIFlag::V4Mapped:      return s("AIFlag::V4Mapped");
        case AIFlag::All:           return s("AIFlag::All");
        case AIFlag::AddrConfig:    return s("AIFlag::AddrConfig");
        break;
    }
    return Nothing();
}
static inline Maybe<std::string> enum_name(RecvFlag f){
    using s = std::string;
    switch(f){
        case RecvFlag::DontWait: return s("RecvFlag::DontWait");
        // TODO: augment me, plz
    }
    return Nothing();
}

static inline Maybe<std::string> enum_name(SendFlag f){
    using s = std::string;
    switch(f){
        case SendFlag::Confirm:     return s("SendFlag::Confirm");
        case SendFlag::DontWait:    return s("SendFlag::DontWait");
        case SendFlag::DontRoute:   return s("SendFlag::DontRoute");
        case SendFlag::EndOfRecord: return s("SendFlag::EndOfRecord");
        case SendFlag::More:        return s("SendFlag::More");
        case SendFlag::NoSignal:    return s("SendFlag::NoSignal");
        case SendFlag::OutOfBounds: return s("SendFlag::OutOfBounds");
        break;
    }
    return Nothing();
}

inline std::string to_string(AddressFamily v)  { return enum_name(v).value_or("<Unknown AddressFamily: " + std::to_string(cpp::to_underlying(v)) + ">"); }
inline std::string to_string(SocketType v)     { return enum_name(v).value_or("<Unknown SocketType: "    + std::to_string(cpp::to_underlying(v)) + ">"); }
inline std::string to_string(Protocol v)       { return enum_name(v).value_or("<Unknown Protocol: "      + std::to_string(cpp::to_underlying(v)) + ">"); }
inline std::string to_string(RecvFlag v)       { return enum_name(v).value_or("<Unknown RecvFlag: "      + std::to_string(cpp::to_underlying(v)) + ">"); }
inline std::string to_string(SendFlag v)       { return enum_name(v).value_or("<Unknown SendFlag: "      + std::to_string(cpp::to_underlying(v)) + ">"); }
inline std::string to_string(const std::vector<AIFlag> & vf){
    std::stringstream ss;
    ss << "[";
    for(size_t i = 0; i < vf.size(); ++i){
        if(i > 0){ ss << ", "; }
        ss << enum_name(vf[i]).value_or("Unknown AIFlag: " + std::to_string(cpp::to_underlying(vf[i])));
    }
    ss << "]";
    return ss.str();
}

} // ns inet

} // ns unix
