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

	// Reverse of flags_to_int
	template <typename T>
	constexpr auto to_int(const T a, const T b){
		auto x = 0;
		for(auto it = a; it != b; ++it){
			x |= cpp::to_underlying(*it);
		}
		return x;
	}

	// Reverse of above
	// NOTE: this version should generate a compile-time constant, typesafely
	// FIXME: for some reason assigning the return value does not give warning about narrowing conversion
	template <typename T>
	constexpr int to_int(const std::initializer_list<T> & l){
		return to_int(std::begin(l), std::end(l));
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
