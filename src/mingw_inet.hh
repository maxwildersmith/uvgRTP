#pragma once

#ifdef _WIN32
#include <winsock2.h>
#endif

namespace uvgrtp {
    namespace mingw {
        /* void*, matching the real POSIX/Winsock signature. Was
         * `struct in_addr *`, which is wrong for AF_INET6 -- and because
         * socket.cc does `using namespace mingw`, that made
         * create_ip6_sockaddr fail to compile with "cannot convert
         * in6_addr* to in_addr*". The AF_INET6 path always wrote 16 bytes
         * regardless; only the declared type was inconsistent. */
        int inet_pton(int af, const char *src, void *dst);
    }
}

namespace uvg_rtp = uvgrtp;
