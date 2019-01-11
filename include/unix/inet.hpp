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
#include <unix/inet_common.hpp>

namespace _unix
{

namespace inet
{

using namespace cpp;

class SockAddr {
public:
    SockAddr() : _len(0), _ss{} { }

    // Creates a SockAddr from C-struct. Copies values, since the pointer
    // may be temporary inside a 'struct addrinfo'.
    SockAddr(const struct sockaddr*, socklen_t);

    SockAddr(const sockaddr_storage & ss, socklen_t l) : _len(l), _ss{ss} { 
        if(!AddressFamilyCheck::is_value(family())){
            auto v = cpp::to_underlying(family());
            throw std::runtime_error("Unknown family code: " + std::to_string(v));
        }
    }
    const struct sockaddr* addr() const;

    socklen_t addrlen() const;

    AddressFamily family() const;

    // Port numbers can really be enumerated..
    uint16_t port() const;

    std::string address() const;
    std::string to_string(int level=0) const;

    std::string address_and_port() const { return address() + std::to_string(port()); }

    bool operator==(const SockAddr o){ return o.address() == address() && o.port() == port(); }
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
        const std::initializer_list<RecvFlag> & fl = {}
    )
    {
        return ::recv(_sock, reinterpret_cast<unsigned char*>(buf), buflen, cpp::to_int(fl));
    }

    std::pair<ssize_t, Maybe<SockAddr>> 
    recvfrom(uint8_t * buf, ssize_t buflen, const std::initializer_list<RecvFlag> & f = {})
    {
        // If you are using recvfrom, you obviously want these filled in, don't cha?
        struct sockaddr_storage ss = {};
        socklen_t len = sizeof(ss);

        auto * sa = reinterpret_cast<struct sockaddr*>(&ss);

        auto ret = ::recvfrom(_sock, buf, buflen, cpp::to_int(f), sa, &len);

        return std::make_pair(ret, SockAddr(ss, len));
    }

    Maybe<SockAddr> getsockname() const;
    Maybe<SockAddr> getpeername() const;

    ssize_t sendto(const uint8_t * buf, size_t len, const SockAddr & dest, const std::initializer_list<SendFlag> & fl = {})
    {
        return ::sendto(_sock, buf, len, cpp::to_int(fl), dest.addr(), dest.addrlen());
    }

    // needs to be connect()'ed first
    ssize_t send(const uint8_t *buf, size_t buflen, const std::initializer_list<SendFlag> & fl = {}){
        return ::send(_sock, reinterpret_cast<const void*>(buf), buflen, cpp::to_int(fl));
    }

    // needs to be connect()'ed first
    ssize_t send(const std::string & s, const std::initializer_list<SendFlag> & fl = {}){
        return send(reinterpret_cast<const uint8_t*>(s.data()), s.size(), fl);
    }


    int setsockopt(int level, int optname, const void* optval, socklen_t optlen) {
        // hmmm
        return ::setsockopt(_sock, level, optname, optval, optlen);
    }
    int getsockopt() ;

    bool setblocking(bool val);

    // be careful. EXTREMELY careful. This is just to avoid circular dependencies
    // with other classes, such as Epoll
    int __fd() const { return _sock; }
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

Maybe<Socket> client_socket_any(
    const std::string & raddr,
    const std::string & service = ""
);


std::ostream & operator<<(std::ostream &, const inet::AddrInfo &);
std::ostream & operator<<(std::ostream &, const inet::SockAddr &);


// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

} // end of ns inet

} // unix
