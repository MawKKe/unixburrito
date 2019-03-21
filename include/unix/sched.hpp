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

} // ns affinity
} // ns unix

std::ostream& operator<<(std::ostream& os, const _unix::sched::CPUSet & cs);

