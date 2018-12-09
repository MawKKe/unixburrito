#pragma once

#include <experimental/optional>
#include <vector>
#include <algorithm>

namespace cpp {
    template <typename T>
    using Maybe = std::experimental::optional<T>;

    constexpr auto Nothing() { return std::experimental::nullopt; }


    template <typename E>
    constexpr auto to_underlying(E e) noexcept
    {
        return static_cast<std::underlying_type_t<E>>(e);
    }

    template <typename T>
    void extend(std::vector<T> & dest, const std::vector<T> & src){
        dest.reserve(dest.size() + src.size());
        dest.insert(dest.end(), src.begin(), src.end());
    }

    // Useful: if(element_in(variable, {"foo", "bar"})) { ... }
    template <typename T>
    bool element_in(const T & e, const std::vector<T> & v){
        return std::find(v.begin(), v.end(), e) != v.end();
    }
}

template <typename T>
std::ostream & operator<<(std::ostream & os, const cpp::Maybe<T> & m){
    if(!m){ 
        os << "Nothing" ;
    }
    else{ 
        os << *m;
    }
    return os;
}
