/**
 * @author lafftale1999
 * 
 * @brief Defines lynks::network::session_token, an object used to represent and track an individual 
 * authentication sessions. Each token binds a 64-character hash to an owner identifier (such as a 
 * username) and maintains a timestamp-based lifetime. This enables lookups with token to receive 
 * the owners username for example.
 */

#ifndef NETWORK_SESSION_TOKEN_HPP_
#define NETWORK_SESSION_TOKEN_HPP_

#include "network_common.hpp"

namespace lynks::network {
    /**
     * @brief token used for controlling sessions.
     */
    class session_token {
        public:
            /**
             * @param owner_identifier is the value that identifies the client owning the token.
             * 
             * @param token Need to be a 64-char hash to pass validation when created..
             */
            session_token(std::string owner_identifier, std::string token);
            
            /**
             * @brief Checks if the token is still valid and updates its current life.
             * 
             * @return `true` if still active and `false` if not.
             */
            bool validate_token();

            /**
             * @brief Checks if the token is still active.
             */
            bool is_active() const;

            const std::string& get_owner() const;
            const std::string& get_token() const;

        private:
            /**
             * @brief Updates the current `life_ms` to now. This will
             * inherently extend the token's life-time.
             */
            void update_life();

            /**
             * @brief Retrieves now() from `std::chrono` casted as ms since epoch.
             * 
             * @return `ms` since epoch.
             */
            uint64_t get_now_ms() const;

        private:
            std::string token;
            std::string owner_identifier;
            uint64_t life_ms;

            static constexpr uint64_t max_life_ms = 300'000; 
    };
}


#endif