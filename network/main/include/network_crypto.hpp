#ifndef NETWORK_CRYPTOGRAPHY_HPP_
#define NETWORK_CRYPTOGRAPHY_HPP_

#include "network_common.hpp"
#include <type_traits>
#include <random>

namespace lynks {
    namespace network {
        namespace crypto {

            /**
             * @brief Hashes the incoming string to a 64-char string.
             * 
             * @param str& string to hash.
             * 
             * @return Hashed 64-char string based on `str`
             */
            std::string hash256(const std::string& str);

            /**
             * @brief A machine for generating random numbers.
             * 
             * @attention Only accepts integrals as T.
             */
            template <typename T>
            requires std::is_integral_v<T>
            class random_engine {
                public:
                    /**
                     * @brief Constructs the machine and seeds it.
                     * 
                     * @param min lowest number that can be used.
                     * @param max highest number that can be used.
                     */
                    random_engine(T min, T max) 
                    : distribution(std::min(min, max), std::max(min, max))
                    {
                        std::random_device device;
                        std::seed_seq sequence{
                            device(), device(), device(), device(), device() 
                        };

                        rng.seed(sequence);
                    }

                    /**
                     * @brief Generates a random number in your assigned range
                     * when constructing the machine.
                     */
                    T generate_number() { return distribution(rng); }

                private:
                    std::mt19937 rng;
                    std::uniform_int_distribution<T> distribution;
            };
        }  
    }
}

#endif