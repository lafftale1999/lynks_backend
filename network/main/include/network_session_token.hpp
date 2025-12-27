#ifndef NETWORK_SESSION_TOKEN_HPP_
#define NETWORK_SESSION_TOKEN_HPP_

#include "network_common.hpp"
#include "network_crypto.hpp"
#include "network_queue.hpp"

#include <unordered_map>

namespace lynks::network {
    class sessions_handler {
        public:
            sessions_handler() = default;

            const std::string add_session(std::string username);
            bool is_active(const std::string& token);

        private:
            void update_session(std::string token);
            void end_session(std::string token);

            class session_token;
            std::unordered_map<session_token, std::string> sessions;
    };
}

#endif