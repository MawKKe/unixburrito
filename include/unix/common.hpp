#pragma once

#include <string>
#include <cstring>

// Compiler should select the corret overload depending on your ::strerror_r variant
// GNU version
static inline std::string __strerror_r(const char* ret, const char * buf, int e) {
	(void)buf;
	return (ret == nullptr) ? ("Unknown error code: " + std::to_string(e)) : std::string(ret);
}
// XSI version
static inline std::string __strerror_r(int ret, const char * buf, int e){
	return (ret == 0) ? std::string((const char*)buf) : ("Unknown error code: " + std::to_string(e));
}

namespace _unix {

	// Actual interface
    inline std::string errno_str(int e){
        char buf[256] = {};
        return __strerror_r(::strerror_r(e, buf, sizeof(buf)), buf, e);

    }
}
