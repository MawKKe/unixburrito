#pragma once

#include <iostream>
#include <map>
#include <string>
#include <csignal>

#include <cpp.hpp>

namespace unix {
    // basically wrapper around strerror_r
    std::string errno_str(int e);

    // TODO add more signals....
    enum class Signal : uint32_t {
        Interrupt = SIGINT,
        User1     = SIGUSR1,
        User2     = SIGUSR2,
    };

    // TODO: Add more signals...
    const std::map<int, Signal> signal_map = {
        {SIGINT,  Signal::Interrupt },
        {SIGUSR1, Signal::User1     },
        {SIGUSR2, Signal::User2     },
    };

    // Contrary to other enum classes, it is best to keep the names of the signals as-is.
    // They are widely known as-is, and all cli programs use them as well.
    const std::map<Signal, std::string> signal_names = {
        {Signal::Interrupt, "SIGINT"},
        {Signal::User1,     "SIGUSR1"},
        {Signal::User2,     "SIGUSR2"},
    };

    // without : uint32_t the compile complains that some flags are out of range
    // of the underlying type (int)
    enum class SigActionFlag : uint32_t {
        NoChildStop     = SA_NOCLDSTOP,
        NoChildWait     = SA_NOCLDWAIT,
        NoDefer         = SA_NODEFER,
        AlternateStack  = SA_ONSTACK,
        ResetHandler    = SA_RESETHAND,
        RestartSysCall  = SA_RESTART,
        IncludeSigInfo  = SA_SIGINFO,
        //SA_RESTORER,
    };
    const std::map<int, SigActionFlag> sigaction_map = {
        {SA_NOCLDSTOP, SigActionFlag::NoChildStop      },
        {SA_NOCLDWAIT, SigActionFlag::NoChildWait      },
        {SA_NODEFER,   SigActionFlag::NoDefer          },
        {SA_ONSTACK,   SigActionFlag::AlternateStack   },
        {SA_RESETHAND, SigActionFlag::ResetHandler     },       // a.k.a SA_ONESHOT
        {SA_RESTART,   SigActionFlag::RestartSysCall   },
        {SA_SIGINFO,   SigActionFlag::IncludeSigInfo   },
        //SA_RESTORER, Restorer},           // not intended for application usage
    };
    const std::map<SigActionFlag, std::string> sigaction_names = {
        {SigActionFlag::NoChildStop,   "NoChildStop"    },
        {SigActionFlag::NoChildWait,   "NoChildWait"    },
        {SigActionFlag::NoDefer,       "NoDefer"        },
        {SigActionFlag::AlternateStack,"AlternateStack" },
        {SigActionFlag::ResetHandler,  "ResetHandler"   },       // a.k.a SA_ONESHOT
        {SigActionFlag::RestartSysCall,"RestartSysCall" },
        {SigActionFlag::IncludeSigInfo,"IncludeSigInfo" },
        //SA_RESTORER, Restorer},           // not intended for application usage
    };

    std::string to_string(Signal);
    std::string to_string(SigActionFlag);

    // Fowrard declarations
    class SigAction;
    int sigaction(Signal signum, const SigAction & newact);

    class SigAction 
    {
        using HandlerType1 = void (*)(int);
        using HandlerType2 = void (*)(int, siginfo_t *, void *);

    public:

        // NOTE: Use these as your primary means of creating a SigAction object.
        // Usage:
        //     SigAction sa = SigAction::emptySet();
        //   or
        //     SigAction sa = SigAction::fullSet();
        static SigAction emptySet() { SigAction sa; sa._clear(); return sa; }
        static SigAction fullSet()  { SigAction sa; sa._fill();  return sa; }

        // You may also utilize this one-shot interrupt handling setup. Useful if you wish
        // to just get proper shutdown handling done. Add your handler function and "run"-variable,
        // like this:
        /*
            volatile std::sig_atomic_t run = true;

            static void myHandler(int sig __attribute__ ((unused)))
            {
                run = false;
            }

        */
        // Then in you main():
        //
        //  if(unix::SigAction::handleInterrupt(myHandler) < 0){
        //      ...
        //  }
        //
        template <typename H>
        static int handleInterrupt(H h) { 
            SigAction sa = SigAction::emptySet();
            sa.mask_add(Signal::Interrupt);
            sa.mask_add(Signal::User1);
            sa.set_handler(h);

            std::cerr << sa.to_string() << "\n";

            auto sig = Signal::Interrupt;

            // Set a signal handler to catch SIGINT to allow for graceful termination.
            if((unix::sigaction(sig, sa) != 0))
            {
                std::cerr << "ERROR sigaction(): " << unix::errno_str(errno) << "\n";
                return -1;
            }
            std::cerr << "INFO: signal handler set up for: " << unix::to_string(sig) << "\n";
            return 0;
        }

        // You may use these with setHandler. 
        // Usage:
        //    my_sigact.set_handler(SigAction::Default);
        //    my_sigact.set_handler(SigAction::Ignore);
        static constexpr HandlerType1 Default = SIG_DFL;
        static constexpr HandlerType1 Ignore  = SIG_IGN;

        // The different handler types are mutually exclusive, but fortunately these 
        // overloaded setup funcs will handle all details for you!
        void set_handler(HandlerType1 h);
        void set_handler(HandlerType2 h);

        // Removes current handler (if exists), assigns the default handler (SigAction::Default).
        void set_default_handler();

        void mask_remove(Signal signum);
        void mask_add(Signal signum);
        bool mask_is_set(Signal signum) const;

        std::string to_string(int level = 0) const;

        // With this you can add any flags you like, except IncludeSigInfo (SA_SIGINFO), as it is
        // handled internally by set_handler() funcs. If you try, you will get a warning message into stderr.
        // Usage:
        //  set_flags({SigActionFlag::ResetHandler, ...});
        void set_flags(const std::initializer_list<SigActionFlag> & fl);

        // access to the underlying object is required when calling ::sigaction
        const struct sigaction * action() const;
    private:
        SigAction();
        void _clear();
        void _fill();
        void _set_siginfo(bool on);

        std::string _handler_name() const;

        struct sigaction _act;
    };

    // call sigaction, ignore old action
    inline int sigaction(Signal signum, const SigAction & newact){
        return ::sigaction(cpp::to_underlying(signum), newact.action(), nullptr);
    }

    // call sigaction, store old action to 'oldact'
    inline int sigaction(int signum, const SigAction & newact, SigAction & oldact);


} // ns unix

inline std::ostream & operator<<(std::ostream & os, const unix::SigAction & sa){
    os << sa.to_string();
    return os;
}
