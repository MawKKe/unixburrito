#include <unix/signals.hpp>

#include <string>
#include <sstream>
#include <iostream>
#include <cstring>

#include <cpp.hpp>

namespace _unix {

	namespace signals {

    SigAction::SigAction() : _act{} { }

    const struct sigaction * SigAction::action() const {
        return &_act;
    }
    // The handler is in effect a tagged union (even if the sigaction struct itself does not
    // present them as union). The SA_SIGINFO flag specifies which type of handler is used.
    // It's best to infer which type of function is used from the pointer type, and ONLY THEN
    // set/unset the SA_SIGINFO in the sa_flags.
    //
    // (Although I noticed the signal handling seems to work just fine even if you forget to set
    // SA_INFO flag...???)
    void SigAction::set_handler(HandlerType1 h){
        // HURR DURR imma switch the order of assignments and wonder for SEVERAL HOURS why the
        // fuck my signal handler is not called! HERP DERP DURR
        // (hint: sa_sigaction and sa_handler are in an union, most likely)
        _act.sa_sigaction = nullptr;
        _act.sa_handler = h;
        _set_siginfo(false);
    }
    void SigAction::set_handler(HandlerType2 h){
        _act.sa_handler = nullptr;
        _act.sa_sigaction = h;
        _set_siginfo(true);
    }

    void SigAction::_clear() {
        if(::sigemptyset(&_act.sa_mask) != 0){
            throw std::runtime_error("sigemptyset() failed: " + _unix::errno_str(errno));
        }
    }
    void SigAction::_fill() {
        if(::sigfillset(&_act.sa_mask) != 0){
            throw std::runtime_error("sigfillset() failed: " + _unix::errno_str(errno));
        }
    }
    void SigAction::mask_remove(Signal signum) {
        if(::sigdelset(&_act.sa_mask, cpp::to_underlying(signum)) != 0){
            throw std::runtime_error("sigaddset() failed: " + _unix::errno_str(errno));
        }
    }
    void SigAction::mask_add(Signal signum) {
        if(::sigaddset(&_act.sa_mask, cpp::to_underlying(signum)) != 0){
            throw std::runtime_error("sigaddset() failed: " + _unix::errno_str(errno));
        }
    }
    bool SigAction::mask_is_set(Signal signum) const {
        auto ret = ::sigismember(&_act.sa_mask, cpp::to_underlying(signum));
        if(ret < 0){
            throw std::runtime_error("sigaddset() failed: " + _unix::errno_str(errno));
        }
        // manuals say that "true" == 1, "false == 0", but what if the value is
        // something else?
        return bool(ret);
    }
    void SigAction::set_flags(const std::initializer_list<SigActionFlag> & fl){
        for(const auto & e : fl){
            if(e == SigActionFlag::IncludeSigInfo){
                std::cerr
                    << "WARNING: SigAction::set_flags(): "
                    << "ignoring flag SigActionFlag::IncludeSigInfo (SA_SIGINFO)" << std::endl;
                continue;
            }
            _act.sa_flags |= cpp::to_underlying(e);
        }
    }

    void SigAction::_set_siginfo(bool on){
        _act.sa_flags = on ? (_act.sa_flags | SA_SIGINFO) : (_act.sa_flags & ~SA_SIGINFO);
    }

    std::string SigAction::_handler_name() const {
        if(!(_act.sa_flags & SA_SIGINFO)){
           if(_act.sa_handler == SigAction::Default()){
               return "SigAction::Default";
           }
           else if(_act.sa_handler == SigAction::Ignore()){
               return "SigAction::Ignore";
           }
           else{
               return "[Unknown function of HandlerType1]";
           }
        }
        else {
            if(_act.sa_handler != nullptr){
                return "[Unknown function of HandlerType2]";
            }
            else{
                return "[WTF?! SA_SIGINFO is set, but sa_handler == null]";
            }
        }
        return "<null>";
    }

    std::string SigAction::to_string(int level) const {
        std::string prefix(2*level, ' ');
        std::stringstream ss;
        ss << prefix << "SigAction {\n"
           << prefix << "  handler: " << _handler_name() << "\n"
           << prefix << "  masked:  [";
        bool first = true;
        for(const auto & p : signal_map){
            if(mask_is_set(p.second)){
                if(!first) {
                    ss << ", ";
                }
                ss << prefix << _unix::signals::to_string(p.second);
            }
            first = false;
        }
        ss << prefix << "]\n"
           << prefix << "  flags:   [";

        first = true;
        for(const auto & p : sigaction_map){
            if(_act.sa_flags & p.first){
                if(!first) ss << ", ";
                ss << _unix::signals::to_string(p.second);
                first = false;
            }
        }
        ss << prefix << "]\n"
           << prefix << "}";

        return ss.str();
    }

    std::string to_string(Signal s) { return signal_names.find(s)->second; }
    std::string to_string(SigActionFlag f) { return sigaction_names.find(f)->second; }

    // call sigaction, ignore old action
    int sigaction(Signal signum, const SigAction & newact){
        return ::sigaction(cpp::to_underlying(signum), newact.action(), nullptr);
    }

	} // ns signals

} // ns unix
