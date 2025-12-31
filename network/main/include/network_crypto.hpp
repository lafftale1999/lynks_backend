/**
 * @author lafftale1999
 * 
 * @brief Defines small cryptographic and randomness utilities used by the networking layer. It provides 
 * a hash256 function for hashing arbitrary strings into fixed-length 64-character values, which is suitable 
 * for identifiers and tokens and a templated random_engine constrained to integral types for generating 
 * pseudo-random numbers within a defined range. The random_engine also offers a higher-level generate_token 
 * helper that combines multiple random values and hashing steps to produce non-guessable tokens.
 */

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

                    std::string generate_token() {
                        // First hash
                        auto raw = generate_number();
                        auto hash_1 = crypto::hash256(std::to_string(raw));

                        // Second hash
                        raw = generate_number();
                        auto hash_2 = crypto::hash256(std::to_string(raw));

                        // Combine together
                        std::stringstream ss;
                        ss << hash_1 << hash_2;

                        return crypto::hash256(ss.str());
                    }

                private:
                    std::mt19937 rng;
                    std::uniform_int_distribution<T> distribution;
            };
        }  
    }
}

#endif