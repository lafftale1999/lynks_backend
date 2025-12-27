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
                : rng(device), distribution(min, max)
                {
                    rng.seed(create_seed());
                }

                T generate_number() {
                    return distribution(rng);
                }


            private:
                std::random_device device;
                std::mt19937 rng;
                std::uniform_int_distribution<std::mt19937::result_type> distribution;

                static uint32 create_seed() {
                    auto first_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                    auto second_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

                    float raw_seed = static_cast<float>(first_time) / static_cast<float>(second_time);
                    return static_cast<uint32_t>raw_seed;
                }
        };
    }
}

#endif