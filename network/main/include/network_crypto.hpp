#ifndef NETWORK_CRYPTOGRAPHY_HPP_
#define NETWORK_CRYPTOGRAPHY_HPP_

#include "network_common.hpp"

namespace lynks {
    namespace network {
        class crypto {
            public:
                static std::string hash256(const std::string& str);
        };
    }
}

#endif