#include <unix/sched.hpp>

#include <sstream>
#include <iostream>


namespace _unix {

namespace sched {

void CPUSet::zero()         { CPU_ZERO(&m_set); }
void CPUSet::set(int cpu)   { check("set()",   cpu); CPU_SET(cpu, &m_set); }
void CPUSet::unset(int cpu) { check("unset()", cpu); CPU_SET(cpu, &m_set); }

bool CPUSet::is_set(int cpu) const { check("is_set()", cpu); return CPU_ISSET(cpu, &m_set) != 0; }
int  CPUSet::count()         const { return CPU_COUNT(&m_set); }

std::vector<int> CPUSet::cpu_list(int limit) const {
    const int want = count();
    std::vector<int> cpus;
    for(int i = 0; ((int)cpus.size() < want && i < limit); ++i){
        if(is_set(i)){
            cpus.push_back(i);
        }
    }
    if((int)cpus.size() != want){
        throw std::runtime_error(
                "for some reason all CPUs could no be listed. Wanted: " +
                std::to_string(want) + ", got: " + std::to_string(cpus.size()));

    }
    return cpus;
}

CPUSet CPUSet::operator&(const CPUSet & o) const{
    CPUSet cs;
    CPU_AND(&cs.m_set, &m_set, &o.m_set);
    return cs;
}

CPUSet CPUSet::operator|(const CPUSet & o) const {
    CPUSet cs;
    CPU_OR(&cs.m_set, &m_set, &o.m_set);
    return cs;
}

CPUSet CPUSet::operator^(const CPUSet & o) const {
    CPUSet cs;
    CPU_XOR(&cs.m_set, &m_set, &o.m_set);
    return cs;
}

bool CPUSet::operator==(const CPUSet & o) const {
    return CPU_EQUAL(&m_set, &o.m_set) != 0;
}
bool CPUSet::operator!=(const CPUSet & o) const {
    return !(*this == o);
}

std::string CPUSet::repr() const {
    auto cpus = cpu_list();
    std::stringstream ss;
    ss << "CPUSet {";
    for(size_t i = 0; i < cpus.size(); ++i){
        if(i > 0){ ss << ", "; }
        ss << cpus[i];
    }
    ss << "}";
    return ss.str();
}

void CPUSet::check(std::string func, int cpu) const{
    if(cpu >= CPU_SETSIZE){
        std::cerr
            << "WARNING: CPUSet::" << func << ": cpu value '" << cpu << "'"
            << " is beyond storing capability of cpu_set_t "
            << "(CPU_SETSIZE = " << CPU_SETSIZE << ").\n";
    }
}

void affinity_set_thread(std::thread & t, const CPUSet & cs){
    int ret = ::pthread_setaffinity_np(t.native_handle(), sizeof(cs.m_set), &cs.m_set);
    if(ret != 0){
        throw std::runtime_error("pthread_setaffinity_np(): " + _unix::errno_str(errno));
    }
}

CPUSet affinity_get_thread(std::thread & t){
    CPUSet cs; cs.zero();
    int ret = ::pthread_getaffinity_np(t.native_handle(), sizeof(cs.m_set), &cs.m_set);
    if(ret != 0){
        throw std::runtime_error("pthread_getaffinity_np(): " + _unix::errno_str(errno));
    }
    return cs;
}

void affinity_set(pid_t pid, const CPUSet & cs){
    int ret = ::sched_setaffinity(pid, sizeof(cs.m_set), &cs.m_set);
    if(ret != 0){
        throw std::runtime_error("sched_setaffinity(): " + _unix::errno_str(errno));
    }
}

CPUSet affinity_get(pid_t pid){
    CPUSet cs; cs.zero();
    int ret = ::sched_getaffinity(pid, sizeof(cs.m_set), &cs.m_set);
    if(ret != 0){
        throw std::runtime_error("sched_getaffinity(): " + _unix::errno_str(errno));
    }
    return cs;
}

} // ns affinity
} // ns _unix

std::ostream& operator<<(std::ostream& os, const _unix::sched::CPUSet & cs){
    os << cs.repr();
    return os;
}
