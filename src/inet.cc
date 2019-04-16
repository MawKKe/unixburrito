#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>

#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
#include <iomanip>

#include <unix/inet.hpp>
#include <unix/common.hpp>

#include <cpp.hpp>

namespace _unix
{

namespace inet
{

using namespace cpp;

// int_to_flags
int flags_to_int(const std::vector<AIFlag> & fv){
    int flags = 0;
    for(const auto & f : fv){
        flags |= to_underlying(f);
    }
    return flags;
}


const struct sockaddr* SockAddr::addr() const {
    return reinterpret_cast<const struct sockaddr*>(&_ss);
}
socklen_t SockAddr::addrlen() const {
    return _len;
}

AddressFamily SockAddr::family() const {
    return static_cast<AddressFamily>(_ss.ss_family);
}

// Port numbers can really be enumerated..
uint16_t SockAddr::port() const {
    auto fam = family();
    switch (fam){
        case AddressFamily::IPv4: {
            auto * p = reinterpret_cast<const struct sockaddr_in*>(&_ss);
            return ntohs(p->sin_port);
        }
        case AddressFamily::IPv6: {
            auto * p = reinterpret_cast<const struct sockaddr_in6*>(&_ss);
            return ntohs(p->sin6_port);
        }
        default: {
            // idk
            return 0;
        }
    }
}

std::string SockAddr::address() const {
    char buf[INET6_ADDRSTRLEN+1];
    auto fam = family();
    switch (fam){
        case AddressFamily::Any:
            return "<Any/Unknown address>";
            break;
        case AddressFamily::IPv4: {
            auto * p = reinterpret_cast<const struct sockaddr_in*>(&_ss);
            auto * n = inet_ntop(to_underlying(fam), &(p->sin_addr), buf, sizeof(buf));
            return (n == nullptr) ? "<invalid ipv4 address>" : std::string(buf);
            break;
        }
        case AddressFamily::IPv6: {
            auto * p = reinterpret_cast<const struct sockaddr_in6*>(&_ss);
            auto * n = inet_ntop(to_underlying(fam), &(p->sin6_addr), buf, sizeof(buf));
            return (n == nullptr) ? "<invalid ipv6 address>" : std::string(buf);
            break;
        }
        /*
        case AddressFamily::Unix: {
            break;
        }
        */
        default:
            throw std::runtime_error("Strange family code!?!?!");
            break;
    };
}

std::string SockAddr::to_string(int level) const {
    std::string prefix(level*3, ' ');
    std::stringstream ss;
    ss
    << prefix << "SockAddr {\n"
    << prefix << "  family:  " << inet::to_string(family()) << "\n"
    << prefix << "  port:    " << port() << "\n"
    << prefix << "  address: " << address() << "\n"
    << prefix << "}";
    return ss.str();
}


AddrInfo::AddrInfo(
    AddressFamily af,
    SocketType type,
    Protocol proto,
    const std::vector<AIFlag> & flags
) {
    reset();
    set_family(af);
    set_socktype(type);
    set_protocol(proto);
    for(auto & f : flags){
        set_flag(f);
    }
}
std::string AddrInfo::to_string(int level) const {
    std::string prefix(level*2, ' ');
    std::stringstream ss;
    ss  << prefix << "AddrInfo {\n"
        << prefix << "  family:    " << family_s()          << "\n"
        << prefix << "  socktype:  " << socket_type_s()     << "\n"
        << prefix << "  protocol:  " << protocol_s()        << "\n"
        << prefix << "  flags:     " << flags_s()           << "\n"
        << prefix << "  canonname: " << canonical_name().value_or("<null>") << "\n"
        << prefix << "  addrlen:   " << addr_len() << "\n"
        << prefix << "  sockaddr:  ";

    auto sa = sockaddr();
    if(sa){
        ss << "\n" << (*sa).to_string(level+1) << "\n";
    }
    else {
        ss << "<null>\n";
    }
    ss << prefix << "}";
    return ss.str();
}

struct addrinfo AddrInfo::to_hints() const {
    // When needing hints, only these four fields are required,
    // others are zero (will be filled by getaddrinfo)
    struct addrinfo h = {};
    h.ai_family   = to_underlying(family());
    h.ai_socktype = to_underlying(socket_type());
    h.ai_protocol = to_underlying(protocol());
    h.ai_flags    = flags_to_int(flags());
    return h;
}

AddressFamily AddrInfo::family()      const { return _af; }
SocketType    AddrInfo::socket_type() const { return _st; }
Protocol      AddrInfo::protocol()    const { return _pt; }

socklen_t     AddrInfo::addr_len()    const { return bool(_sa) ? (*_sa).addrlen() : 0; }
std::vector<AIFlag> AddrInfo::flags() const { return _flags; }

void AddrInfo::set_params(
    AddressFamily af,
    SocketType st,
    Protocol pt,
    const std::vector<AIFlag> & flags
){
    // To keep this object in consistent state, the "output" variables need
    // to be reset if any "input" variables are changed
    _reset_outvars();
    _af = af;
    _st = st;
    _pt = pt;
    _flags = flags;
}

void AddrInfo::set_family(AddressFamily af) {
    _af = af;
    _reset_outvars();
}
void AddrInfo::set_socktype(SocketType st){
    _st = st;
    _reset_outvars();
}
void AddrInfo::set_protocol(Protocol proto){
    _pt = proto;
    _reset_outvars();
}
void AddrInfo::set_flag(AIFlag f) {
    _flags.push_back(f);
    _reset_outvars();
}
void AddrInfo::set_flags(const std::vector<AIFlag> & fv){
    _flags = fv;
    _reset_outvars();
}

std::string AddrInfo::family_s() const {
    return inet::to_string(family());
}
std::string AddrInfo::socket_type_s() const {
    return inet::to_string(socket_type());
}
std::string AddrInfo::protocol_s() const {
    return inet::to_string(protocol());
}
std::string AddrInfo::flags_s() const {
    return inet::to_string(flags());
}
Maybe<std::string> AddrInfo::canonical_name() const {
    return _cn;
}
Maybe<SockAddr> AddrInfo::sockaddr() const {
    return _sa;
}
void AddrInfo::_reset_outvars() {
    _cn = Nothing();
    _sa = Nothing();
}

void AddrInfo::reset() {
    _af = AddressFamily::Any;
    _st = SocketType::Any;
    _pt = Protocol::Any;
    _reset_outvars();
}

// Convert (struct sockaddr)->ai_flags to a vector of our 'enum class AIFlag'
std::vector<AIFlag> int_to_flags(int flags);

AddrInfo AddrInfo::from_struct(const struct addrinfo * p){
    auto af = to_enum<AddressFamily>(p->ai_family);
    auto st = to_enum<SocketType>(p->ai_socktype);
    auto pt = to_enum<Protocol>(p->ai_protocol);

    if(!af){
        throw std::runtime_error("Invalid address family");
    }
    if(!st){
        throw std::runtime_error("Invalid socket type");
    }
    if(!pt){
        throw std::runtime_error("Invalid protocol");
    }
    AddrInfo a(*af, *st, *pt, int_to_flags(p->ai_flags));

    if(p->ai_addrlen > 0){
        a._sa = SockAddr::from_struct(p->ai_addr, p->ai_addrlen, false);
    }
    return a;
}

std::vector<AddrInfo> getAddrInfo(
        const std::string & host,
        const AddrInfo& ai,
        const std::string & service
){
    std::vector<AddrInfo> v;

    // this assumes empty host or service name is not valid in any context
    const char * h = (host    == "") ? nullptr : host.c_str();
    const char * s = (service == "") ? nullptr : service.c_str();

    struct addrinfo *res = nullptr, hints = ai.to_hints();

#ifdef _UNIXBURRITO_DEBUG
    std::cerr << "DEBUG: getaddrinfo with hints:\n" << ai << "\n";
#endif

    // Do lookup
    auto ret = getaddrinfo(h, s, &hints, &res);

    if(ret != 0){
        std::cerr << gai_strerror(ret) << std::endl;
        return v;
    }

    for(auto * p = res; p != nullptr; p = p->ai_next){
        v.push_back(AddrInfo::from_struct(p));
    }
    freeaddrinfo(res);

    return v;
}
std::vector<AddrInfo> getAddrInfo(const std::string & host, AddrInfo& hints, uint16_t service){
    std::string srv = std::to_string(service);
    return getAddrInfo(host, hints, srv);
}

std::vector<AIFlag> int_to_flags(int flags) {
    std::vector<AIFlag> fv;
    for(unsigned i = 0; i < (sizeof(flags) * 8); ++i){
        int mask =  (0x1 << i);
        if(flags & mask){
            auto f = to_enum<AIFlag>(mask);
            if(!f){
#ifdef _UNIXBURRITO_DEBUG
                std::cerr << "WARNING: skipping unrecognized AIFlag value: " << mask << std::endl;
#endif
                continue;
            }
            fv.push_back(*f);
            flags &= ~mask;
        }
    }
#ifdef _UNIXBURRITO_DEBUG
    if(flags){
        std::cerr
            << "WARNING -AddrIinfoHint::flags(): Uknown ai_flags remaining:"
            << flags << "\n";
    }
#endif

    std::sort(fv.begin(), fv.end());
    return fv;
}

// A lot of boilerplate, can we somehow merge these?
Socket::Socket(const AddrInfo & info) :
    Socket(info.family(), info.socket_type(), info.protocol())
{
}
Socket::Socket(AddressFamily af, SocketType st, Protocol pt) :
    _sock(::socket(to_underlying(af), to_underlying(st), to_underlying(pt)))
{
    if(_sock < 0){ throw std::runtime_error(errno_str(errno)); }
    //std::cerr << "Opened socket: " << _sock << std::endl;
}
Socket::~Socket() {
    if(_sock > 0){
        //std::cerr << "Closing socket: " << _sock << "\n";
        ::close(_sock);
    }
    _sock = -1;
}

int Socket::bind(const SockAddr & sa){
    return ::bind(_sock, sa.addr(), sa.addrlen());
}

int Socket::bind(const AddrInfo & ai){
    if(!ai.sockaddr()){
        throw std::runtime_error("Trying to bind() without a sockaddr!");
    }
    return bind(*ai.sockaddr());
}

int Socket::connect(const SockAddr & sa){
    int ret = ::connect(_sock, sa.addr(), sa.addrlen());
    if(ret < 0){
        perror("connect()");
        //std::cerr << "ERRROR: connect(): " << _unix::errno_str(errno) << std::endl;
    }
    return ret;
}

int Socket::connect(const AddrInfo & ai){
    if(!ai.sockaddr()){
        throw std::runtime_error("Trying to connect() without a sockaddr!");
    }
    return connect(*ai.sockaddr());
}


bool Socket::setblocking(bool blocking){
    int opts = fcntl(_sock, F_GETFL);
    if (opts < 0){
		std::cerr << "ERROR: fcntl(F_GETFL): " << _unix::errno_str(errno) << "\n";
		return false;
    }

    // Turn ON or OFF depending on the parameter value
    opts = blocking ? (opts & ~O_NONBLOCK) : (opts | O_NONBLOCK);

    if(fcntl(_sock, F_SETFL, opts) < 0){
        std::cerr << "ERROR: fcntl(F_SETFL): " << _unix::errno_str(errno) << "\n";
		return false;
    }

    return true;
}


// a.k.a "passive" socket
Maybe<Socket> server_socket_udp(
    const std::string & laddr,
    const std::string & service
)
{
    AddrInfo hints(AddressFamily::Any, SocketType::Datagram, Protocol::UDP);

    hints.set_flag(AIFlag::Passive);

    const auto aiv = getAddrInfo(laddr, hints, service);

    for(const auto & ai : aiv){
#ifdef _UNIXBURRITO_DEBUG
        std::cerr << "DEBUG: server_socket got ai:\n";
        std::cout << ai << std::endl;
#endif
        try {
            Socket s(ai);
            int ret = s.bind(ai);
            if(ret != 0){
#ifdef _UNIXBURRITO_DEBUG
                std::cerr << "ERROR bind(): " << _unix::errno_str(errno) << std::endl;
#endif
                continue;
            }
            return std::move(s);
        }
        catch (std::runtime_error & e){
#ifdef _UNIXBURRITO_DEBUG
            std::cerr << "server_socket_udp creation failed: " <<  e.what() << std::endl;
#endif
            continue;
        }
    }
#ifdef _UNIXBURRITO_DEBUG
    std::cerr
        << "ERROR: could not create socket for '"
        << laddr << ":" << service << "'" << std::endl;
#endif
    return Nothing();
}

int Socket::listen(int backlog){
    return ::listen(_sock, backlog);
}

Maybe<SockAddr> Socket::getsockname() const {
	struct sockaddr_storage ss;
	socklen_t len = sizeof(ss);
	auto ret = ::getsockname(_sock, reinterpret_cast<struct sockaddr*>(&ss), &len);
	if(ret < 0){
#ifdef _UNIXBURRITO_DEBUG
		std::cerr << "ERROR getsockname(): " << _unix::errno_str(errno) << std::endl;
#endif
		return Nothing();
	}
	return SockAddr::from_struct(ss, len, false);
}
Maybe<SockAddr> Socket::getpeername() const {
	struct sockaddr_storage ss;
	socklen_t len = sizeof(ss);
	int ret = ::getpeername(_sock, reinterpret_cast<struct sockaddr*>(&ss), &len);
	if(ret < 0){
		std::cerr << "ERROR getpeername(): " << _unix::errno_str(errno) << std::endl;
		return Nothing();
	}
	return SockAddr::from_struct(ss, len, false);
}

// a.k.a "active" socket
Maybe<Socket> client_socket_udp(
    const std::string & raddr,
    const std::string & service
)
{
    // Defaults, "Any" in all fields. The details will be filled depending on the other side
    AddrInfo hints(AddressFamily::Any, SocketType::Datagram, Protocol::UDP);

    const auto aiv = getAddrInfo(raddr, hints, service);

    for(const auto & ai : aiv){
#ifdef _UNIXBURRITO_DEBUG
        std::cerr << "----\n";
        std::cerr << "DEBUG: client_socket got ai:\n";
        std::cout << ai << std::endl;
#endif
        try {
            Socket s(ai);
            int ret = s.connect(ai);
            if(ret != 0){
#ifdef _UNIXBURRITO_DEBUG
                std::cerr << "ERROR connect(): " << _unix::errno_str(errno) << std::endl;
#endif
                continue;
            }
            return std::move(s);
        }
        catch (std::runtime_error & e){
#ifdef _UNIXBURRITO_DEBUG
            std::cerr << "client_socket_any creation failed: " <<  e.what() << std::endl;
#endif
            continue;
        }
    }
    std::cerr
        << "ERROR: could not create socket for '"
        << raddr << ":" << service << "'" << std::endl;

    return Nothing();
}


std::ostream & operator<<(std::ostream & os, const _unix::inet::AddrInfo & a){
    os << a.to_string();
	return os;
}
std::ostream & operator<<(std::ostream & os, const _unix::inet::SockAddr & a){
    os << a.to_string();
	return os;
}

Maybe<SockAddr> SockAddr::from_struct(const struct sockaddr_storage & ss, socklen_t len, bool verify){
    if(!verify){
        return SockAddr(ss, len);
    }
    if(len < 2){
        return Nothing();
    }
    auto fam = ss.ss_family;
    auto f = AddressFamilyCheck::to_enum(fam);
    if(!f && *f == AddressFamily::Any){
        return Nothing();
    }
    return SockAddr(ss, len);
}

Maybe<SockAddr> SockAddr::from_struct(const struct sockaddr * p, socklen_t len, bool verify){
    assert(p != nullptr);
    if(!verify){
        return SockAddr(p, len);
    }
    if(len < 2){
        return Nothing();
    }
    auto fam = p->sa_family;
    auto f = AddressFamilyCheck::to_enum(fam);
    if(!f || *f == AddressFamily::Any){
        return Nothing();
    }
    return SockAddr(p, len);
}


} // ns inet

} // ns unix
