#ifndef NETWORK_CRYPTOGRAPHY_HPP_
#define NETWORK_CRYPTOGRAPHY_HPP_

#include "network_common.hpp"
#include <type_traits>
#include <random>

namespace lynks {
    namespace network {
        class crypto {
            public:
                static std::string hash256(const std::string& str);
        };

        template <typename T>
        requires std::is_integral_v<T>
        class random_engine {
            public:
                random_engine(T min, T max) 
                : distribution(std::min(min, max), std::max(min, max))
                {
                    std::random_device device;
                    std::seed_seq sequence{
                        device(), device(), device(), device(), device() 
                    };

                    rng.seed(sequence);
                }

                T generate_number() { return distribution(rng); }

            private:
                std::mt19937 rng;
                std::uniform_int_distribution<T> distribution;
        };
    }
}

#endif