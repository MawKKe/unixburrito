#pragma once

#include <string>
#include <vector>
#include <type_traits>
#include <map>


#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <iostream>
#include <algorithm>

#include <cpp.hpp>

#include <unix/common.hpp>

namespace _unix
{

namespace inet
{

using namespace cpp;



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

enum class SocketType : uint32_t {
    Any         = 0, 
    Datagram    = SOCK_DGRAM,
    Stream      = SOCK_STREAM,
    Raw         = SOCK_RAW,
};

enum class Protocol : uint32_t {
    Any = 0,
    UDP = IPPROTO_UDP,
    TCP = IPPROTO_TCP
};

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


// TODO: augment me plz
enum class RecvFlag : uint32_t {
    DontWait = MSG_DONTWAIT,
};
const std::map<int, RecvFlag> recv_flag_map = {
    {MSG_DONTWAIT, RecvFlag::DontWait},
};
const std::map<RecvFlag, std::string> recv_flag_names = {
    {RecvFlag::DontWait, "RecvFlag::DontWait"},
};

// How to augment:
//  1) add the label into the enum classes, use the proper 'int' value
//  2) add mapping from integer value to enum (***Map variables, below)
//  3) add mapping from

// These exist mainly because the C++ enum classes do not allow checking for existence
// (like "if (IPv4 in AddressFamily::values) "). Some solutions for existence checking
// can be developed by treating (casting) the enum values as int, and then checking wether
// the value is between lower and upper bounds. This, however, does not work when the enum values
// are non-contiguous.
const std::map<int, AddressFamily> address_family_map = {
    {AF_UNSPEC, AddressFamily::Any  },
    {AF_INET,   AddressFamily::IPv4 },
    {AF_INET6,  AddressFamily::IPv6 },
    //{AF_UNIX,   AddressFamily::Unix },
    // TODO: are these all possible?
};
const std::map<int, SocketType> socket_type_map = {
    {0,            SocketType::Any      },
    {SOCK_DGRAM,   SocketType::Datagram },
    {SOCK_STREAM,  SocketType::Stream   },
    {SOCK_RAW,     SocketType::Raw      },
    // TODO: are these all possible?
};
const std::map<int, Protocol> protocol_map = {
    {0,             Protocol::Any},
    {IPPROTO_UDP,   Protocol::UDP},
    {IPPROTO_TCP,   Protocol::TCP},
    // TODO: add more protocols
};
const std::map<int, AIFlag> flag_map {
    {AI_PASSIVE,        AIFlag::Passive       },
    {AI_CANONNAME,      AIFlag::CanonName     },
    {AI_NUMERICHOST,    AIFlag::NumericHost   },
    {AI_NUMERICSERV,    AIFlag::NumericServ   },
    {AI_V4MAPPED,       AIFlag::V4Mapped      },
    {AI_ALL,            AIFlag::All           },
    {AI_ADDRCONFIG,     AIFlag::AddrConfig    },
};

// TODO: probably non-exhaustive lists

const std::map<AddressFamily, std::string> address_family_names = {
    {AddressFamily::Any,  "AddressFamily::Any"},
    {AddressFamily::IPv4, "AddressFamily::IPv4"},
    {AddressFamily::IPv6, "AddressFamily::IPv6"},
    //{AddressFamily::Unix, "AddressFamily::Unix"},
};
const std::map<SocketType, std::string> socket_type_names = {
    {SocketType::Any,      "SocketType::Any"},
    {SocketType::Datagram, "SocketType::Datagram"},
    {SocketType::Stream,   "SocketType::Stream"}
};
const std::map<Protocol, std::string> protocol_names = {
    {Protocol::Any,  "Protocol::Any"},
    {Protocol::UDP,  "Protocol::UDP"},
    {Protocol::TCP,  "Protocol::TCP"}
};
const std::map<AIFlag, std::string> ai_flag_names = {
    {AIFlag::Passive,     "AIFlag::Passive"     },
    {AIFlag::CanonName,   "AIFlag::CanonName"   },
    {AIFlag::NumericHost, "AIFlag::NumericHost" },
    {AIFlag::NumericServ, "AIFlag::Numericserv" },
    {AIFlag::V4Mapped,    "AIFlag::V4Mapped"    },
    {AIFlag::AddrConfig,  "AIFlag::AddrConfig"  },
};

std::string to_string(AddressFamily af);
std::string to_string(SocketType st);
std::string to_string(Protocol pt);
std::string to_string(AIFlag f);
std::string to_string(RecvFlag f);
std::string to_string(const std::vector<AIFlag> & vf);

class SockAddr {
public:
    SockAddr() : _len(0), _ss{} { }
    SockAddr(const struct sockaddr*, socklen_t);
    SockAddr(const sockaddr_storage & ss, socklen_t l) : _len(l), _ss{ss} { 
        _ss = ss;
    }

    const struct sockaddr* addr() const;

    socklen_t addrlen() const;

    AddressFamily family() const;

    // Port numbers can really be enumerated..
    uint16_t port() const;

    std::string address() const;
    std::string to_string(int level=0) const;


private:
    socklen_t _len;
    struct sockaddr_storage _ss;
};

class AddrInfo {
public:

	AddrInfo(
        AddressFamily af = AddressFamily::Any,
        SocketType type  = SocketType::Any,
        Protocol proto   = Protocol::Any,
        const std::vector<AIFlag> & flags = {}
    ); 

    void set_params(
        AddressFamily af, 
        SocketType st, 
        Protocol pt,
        const std::vector<AIFlag> & flags
    );

    // You should only pass values defined in the enum classes. 
    // If you do something silly like set_family(static_cast<AddressFamily>(1234)), then
    // please do not complain that you code went bonkers
    void set_family(AddressFamily af);
    void set_socktype(SocketType st);
    void set_protocol(Protocol proto);
	void set_flag(AIFlag f);
    void set_flags(const std::vector<AIFlag> & fv);

    // Since an object of this class should not be possible to construct
    // without bypassing the type system, we assume the struct values are valid,
    // and the respective maps have values for all enumerated types.
    //
    // If not... well, it's your own fucking fault.
    AddressFamily family()      const;
    SocketType    socket_type() const;
    Protocol      protocol()    const;

    // If the AddfInfo does not have an associated sockaddr, this will return 0.
    socklen_t     addr_len() const;

    std::vector<AIFlag> flags() const;

    std::string family_s()      const;
    std::string socket_type_s() const;
    std::string protocol_s()    const;
    std::string flags_s()       const;
    Maybe<std::string> canonical_name() const;
    Maybe<SockAddr> sockaddr()  const;


    std::string to_string(int level=0) const; 

    // Select all fields to default values
    void reset(); 

    // If you finds yourself needing this function, you are probably doing
    // something wrong. Normally you should never need this, but let's provide 
    // it just in case
	struct addrinfo to_hints() const;

    static AddrInfo from_struct(const struct addrinfo * p);

private:
    void _reset_outvars();

    AddressFamily       _af;
    SocketType          _st;
    Protocol            _pt;
    std::vector<AIFlag> _flags;
    Maybe<std::string>  _cn;
    Maybe<SockAddr>     _sa;
};


class Socket {
public:
    // All sockets are created with information from getAddrInfo().
    //
    // You should NOT be able to make sockets manually from an integer (socket 
    // handle / descriptor), so don't even think about writing a Socket::Socket(int fd) !
    Socket(const AddrInfo & info);

    // RO3: Rule of Three (or Five) - prevent socket class copying
    Socket(const Socket & )             = delete;
    Socket & operator=(const Socket &)  = delete;

    // Socket can be moved around with move semantics
    Socket(Socket && o){ *this = std::move(o); }
    Socket & operator=(Socket && o) { _sock = o._sock; o._sock = -1; return *this;}

    Socket(AddressFamily af, SocketType st, Protocol pt);
    ~Socket();

    int bind(const AddrInfo & ai);
    int bind(const SockAddr & ai);

    int listen(int backlog);

    int connect(const AddrInfo &);
    int connect(const SockAddr &);

    // Type safe version of the ordinary recv and recvfrom
    // Declared in header so the flag-folding can be optimized out at compile time
    ssize_t recv(
        uint8_t * buf, 
        ssize_t buflen, 
        const std::initializer_list<RecvFlag> & f
    )
    {
        return ::recv(_sock, reinterpret_cast<unsigned char*>(buf), buflen, cpp::to_int(f));
    }

    std::pair<ssize_t, Maybe<SockAddr>> 
    recvfrom( uint8_t * buf, ssize_t buflen, const std::initializer_list<RecvFlag> & f)
    {
        // If you are using recvfrom, you obviously want these filled in, don't cha?
        struct sockaddr_storage ss = {};
        socklen_t len = sizeof(ss);

        auto ret = ::recvfrom(_sock, (unsigned char*)buf, buflen, cpp::to_int(f),
               reinterpret_cast<struct sockaddr*>(&ss), &len
        );

        return std::make_pair(ret, SockAddr(ss, len));
    }

    Maybe<SockAddr> getsockname() const {
        struct sockaddr_storage ss;
        socklen_t len = sizeof(ss);
        auto ret = ::getsockname(_sock, reinterpret_cast<struct sockaddr*>(&ss), &len);
        if(ret < 0){
            std::cerr << "ERROR getsockname(): " << _unix::errno_str(errno) << std::endl;
            return Nothing();
        }
        return SockAddr(ss, len);
    }

    int setsockopt(int level, int optname, const void* optval, socklen_t optlen) {
        // hmmm
        return ::setsockopt(_sock, level, optname, optval, optlen);
    }
    int getsockopt() ;

    // be careful
    int fd() { return _sock; }
private:
    int _sock;
};

std::vector<AddrInfo> getAddrInfo(
        const std::string & host, 
        const AddrInfo& ai, 
        const std::string & service
);
std::vector<AddrInfo> getAddrInfo(
        const std::string & host, 
        AddrInfo& hints, 
        uint16_t service
);

Maybe<Socket> server_socket_udp(
    const std::string & laddr,
    const std::string & service = ""
);

Maybe<Socket> client_socket(
    const std::string & raddr,
    const std::string & service = ""
);


std::ostream & operator<<(std::ostream &, const inet::AddrInfo &);
std::ostream & operator<<(std::ostream &, const inet::SockAddr &);


// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

} // end of ns inet

} // unix
