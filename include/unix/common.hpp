#pragma once

#include <string>
#include <cstring>

namespace unix {
    // basically wrapper around strerror_r
    inline std::string errno_str(int e){
        char buf[256];
		// This is how big boys get around stupid compliance issues (XSI vs GNU) :D
        auto ret = ::strerror_r(e, buf, sizeof(buf));
        return (!ret) ? ("Unknown error code: " + std::to_string(e)) : std::string(buf);
    }
}
