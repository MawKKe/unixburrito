#pragma once

#include <sched.h>

#include <vector>
#include <string>
#include <ostream>
#include <initializer_list>

namespace _unix {

class CPUSet
{
public:

    CPUSet(const std::initializer_list<int> & cpus){ zero(); for(auto & cpu : cpus){ set(cpu); } }

    CPUSet() { zero(); }

    void zero();

    void   set(int cpu);
    void unset(int cpu);

    bool is_set(int cpu) const;
    int count() const;

    std::vector<int> cpu_list(int limit=CPU_SETSIZE) const;

    CPUSet operator&(const CPUSet & o) const;

    CPUSet operator|(const CPUSet & o) const;

    CPUSet operator^(const CPUSet & o) const;

    bool operator==(const CPUSet & o) const;
    bool operator!=(const CPUSet & o) const;

    std::string repr() const;

private:
    void check(std::string func, int cpu) const;
    cpu_set_t m_set;

};

} // ns unix

std::ostream& operator<<(std::ostream& os, const _unix::CPUSet & cs);

