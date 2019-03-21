#pragma once

#include <sched.h>

#include <vector>
#include <string>
#include <ostream>
#include <initializer_list>
#include <thread>

#include <unix/common.hpp>
#include <cpp.hpp>

namespace _unix {

namespace sched {

class CPUSet
{
public:

    // For convenience, you may get implicit conversion from {a,b,c} -> CPUSet object.
    // See SchedSetAffinity etc (below).
    CPUSet(const std::initializer_list<int> & cpus){ zero(); for(auto & cpu : cpus){ set(cpu); } }
    CPUSet(const std::vector<int> & cpus){ zero(); for(auto & cpu : cpus){ set(cpu); } }

    CPUSet() { zero(); }

    void zero();

    void   set(int cpu);
    void unset(int cpu);

    bool is_set(int cpu) const;
    int count() const;

    std::vector<int> cpu_list(int limit=CPU_SETSIZE) const;

    CPUSet operator&(const CPUSet &) const;
    CPUSet operator|(const CPUSet &) const;
    CPUSet operator^(const CPUSet &) const;

    bool operator==(const CPUSet &) const;
    bool operator!=(const CPUSet &) const;

    std::string repr() const;

    friend CPUSet affinity_get_thread(std::thread & t);
    friend void   affinity_set_thread(std::thread & t, const CPUSet &);

    friend CPUSet affinity_get(pid_t);
    friend void   affinity_set(pid_t, const CPUSet &);

private:

    void check(std::string func, int cpu) const;
    cpu_set_t m_set;
};

inline std::string to_string(const CPUSet & cs){ return cs.repr(); }

// TODO? For some reason these declarations need to be repeated here.
// It was not the case before I added the inner 'affinity' namespace...?

// Get affinity of a thread specified by the standard thread object.
// These are simple wrappers for pthread_XXXaffinity functions.
// These could be overloaded with affinity_get, but c++ impliciltly
// converts pid_t to thread objects..
CPUSet affinity_get_thread(std::thread & t);
void   affinity_set_thread(std::thread & t, const CPUSet &);

// Set the affinity of a process / thread.
// These are simple wrappers for sched_XXXaffinity functions.
CPUSet affinity_get(pid_t = 0);
void   affinity_set(pid_t, const CPUSet &);

inline void affinity_set(const CPUSet & cs) { return affinity_set(0, cs); }

// Now can simply say "SchedSetAffinity({1,2})", for example.
//inline int  SchedSetAffinity(const std::initializer_list<int> & l) { CPUSet cs(l); return SchedSetAffinity(0, cs); }
//inline int ThreadSetAffinity(std::thread & t, const std::initializer_list<int> & l) { CPUSet cs(l); return ThreadSetAffinity(tr, cs); }

enum class Policy : int
{
    // "Normal" policies
    Other = SCHED_OTHER,
    Batch = SCHED_BATCH,
    Idle  = SCHED_IDLE,

    // "Realtime" policies
    FIFO       = SCHED_FIFO,
    RoundRobin = SCHED_RR,
};

inline std::string to_string(const Policy & p){
    switch(p){
        case Policy::Other: return "Policy::Other";
        case Policy::Batch: return "Policy::Batch";
        case Policy::Idle:  return "Policy::Idle";
        case Policy::FIFO:  return "Policy::FIFO";
        case Policy::RoundRobin:  return "Policy::RoundRobin";
        default: return "<Unknown value of type 'enum class Policy'>";
    }
    // should no get here
}

using Priority = int;

inline std::string to_string(const Priority & p){
    return std::to_string((p));
}

inline bool priority_range_check(Policy p, Priority pri){
    auto mini = ::sched_get_priority_min(cpp::to_underlying(p));
    auto maxi = ::sched_get_priority_max(cpp::to_underlying(p));
    int v = static_cast<int>(pri);
    return (mini <= v) && (v <= maxi);
}

inline Policy policy_get(pid_t pid = 0){
    auto ret = ::sched_getscheduler(pid);
    if(ret < 0){
        auto err = errno;
        throw std::runtime_error("sched_getscheduler error: " + _unix::errno_str(err));
    }
    return static_cast<Policy>(ret);
}

using namespace cpp;

inline int policy_set_normal(pid_t pid, Policy policy){
    if(!cpp::element_in(policy, {Policy::Other, Policy::Batch, Policy::Idle})){
        throw std::runtime_error("Invalid Policy: " + to_string(policy) + " is not a 'Normal' policy");
    }

    struct sched_param prm; prm.sched_priority = 0;
    return ::sched_setscheduler(pid, cpp::to_underlying(policy), &prm);
}

inline int policy_set_normal(Policy policy){
    return policy_set_normal(0, policy);
}

inline int policy_set_realtime(pid_t pid, Policy policy, Priority priority){
    if(!cpp::element_in(policy, {Policy::FIFO, Policy::RoundRobin})){
        throw std::runtime_error("Invalid Policy: " + to_string(policy) + " is not a 'Realtime' policy");
    }
    if(!priority_range_check(policy, priority)){
        throw std::runtime_error("Invalid priority value '" + std::to_string(priority) + "' for policy " + to_string(policy));
    }

    struct sched_param prm; prm.sched_priority = static_cast<int>(priority);
    return ::sched_setscheduler(pid, cpp::to_underlying(policy), &prm);

}
inline int policy_set_realtime(Policy policy, Priority priority){
    return policy_set_realtime((pid_t)0, policy, priority);
}

inline Maybe<Priority> priority_get(pid_t pid = 0){
    struct sched_param param;
    int ret = ::sched_getparam(pid, &param);
    if(ret < 0){
        throw std::runtime_error("What? " + _unix::errno_str(errno));
    }
    return param.sched_priority;
}

} // ns sched
} // ns unix

std::ostream& operator<<(std::ostream& os, const _unix::sched::CPUSet & cs);
inline std::ostream& operator<<(std::ostream& os, const _unix::sched::Policy & p){ os << _unix::sched::to_string(p); return os; }

