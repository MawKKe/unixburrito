#pragma once

#include <sys/epoll.h>
#include <unistd.h>
#include <chrono>

#include <cpp.hpp>
#include <unix/common.hpp>
#include <unix/inet.hpp>

namespace _unix {
namespace epoll {

enum class EpollCtrlOperation {
    Add     = EPOLL_CTL_ADD,
    Delete  = EPOLL_CTL_DEL,
    Modify  = EPOLL_CTL_MOD,
};

enum class EpollEventType : uint32_t{
    Input       = EPOLLIN,
    Output      = EPOLLOUT,
    Error       = EPOLLERR,
    Hangup      = EPOLLHUP,
    EdgeTrigger = EPOLLET,
    OneShot     = EPOLLONESHOT,
    WakeUp      = EPOLLWAKEUP
};

enum class EpollFlag {
    CloseOnExec = EPOLL_CLOEXEC,
};

using _EV = struct epoll_event;

class EpollEvent : public _EV
{
public:
    bool matches(const _unix::inet::Socket & s){
        return s.__fd() == this->data.fd;
    }
};

bool operator&(const EpollEvent & ev, EpollEventType et){
    return ev.events & cpp::to_underlying(et);
}
bool operator&(EpollEventType et, const EpollEvent & ev){
    return ev & et;
}

// WARNING: do not specify N = 0, epoll_wait will error.
template <size_t N>
using EventList = std::array<EpollEvent, N>;

class Epoll {
public:
    Epoll(const std::initializer_list<EpollFlag> & fl = {})
        : _efd(epoll_create1(cpp::to_int(fl)))
    {
        if(_efd < 0){
            auto m = _unix::errno_str(errno);
            throw std::runtime_error("epoll_create1(): " + m);
        }
        std::cerr << "DEBUG: epoll created: " << std::to_string(_efd) << std::endl;
    }

    ~Epoll(){
        if(_efd > 0){
            std::cerr << "DEBUG: epoll closing (" << std::to_string(_efd) << ")";
            ::close(_efd);
            _efd = -1;
        }
    }
    // RO3
    Epoll(const Epoll &)            = delete;
    Epoll& operator=(const Epoll &) = delete;
    // Epoll object can be moved around with move semantics
    Epoll(Epoll && o){ *this = std::move(o); }
    Epoll& operator=(Epoll && o) { _efd = o._efd; o._efd = -1; return *this;}

    // --------------------------------------
    int    add(const _unix::inet::Socket & s, const std::initializer_list<EpollEventType> & l){ return add(s.__fd(), l);    }
    int modify(const _unix::inet::Socket & s, const std::initializer_list<EpollEventType> & l){ return modify(s.__fd(), l); }
    int remove(const _unix::inet::Socket & s){ return remove(s.__fd()); }

    int add(int fd, const std::initializer_list<EpollEventType> & l){
        return ctl(fd, EpollCtrlOperation::Add, l);
    }
    int modify(int fd, const std::initializer_list<EpollEventType> & l){
        return ctl(fd, EpollCtrlOperation::Modify, l);
    }
    // would call this 'delete', but it is a reserved word...
    // Note that the DEL operation does not need any arguments
    int remove(int fd){
        return ctl(fd, EpollCtrlOperation::Delete, {});
    }

    using MilliSeconds = std::chrono::milliseconds;

    template <size_t N>
    int wait(EventList<N> & evl, const MilliSeconds & timeout){
        return ::epoll_wait(_efd, evl.data(), evl.size(), timeout.count());
    }

    template <size_t N>
    int wait_blocking(EventList<N> & evl){
        return ::epoll_wait(_efd, evl.data(), evl.size(), -1);
    }

private:
    int ctl(int fd, EpollCtrlOperation op, const std::initializer_list<EpollEventType> & l = {}){
        struct epoll_event ev = {};
        ev.events = cpp::to_int(l);
        ev.data.fd = fd;
        int ret = ::epoll_ctl(_efd, cpp::to_underlying(op), fd, &ev);
        if(ret < 0){
            throw std::runtime_error("ERROR: epoll_ctl(): " + _unix::errno_str(errno));
        }
        return ret;
    }

    int _efd;
};

} // ns epoll

} // ns _unix
