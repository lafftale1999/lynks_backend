#ifndef JANUS_REPOSITORY_HPP_
#define JANUS_REPOSITORY_HPP_

#include "janus_context.hpp"
#include "network_secrets.hpp"

namespace lynks::network {
    class janus_repository {
        public:
            janus_repository(std::string host = JANUS_HOST, uint16_t port = JANUS_PORT);
            ~janus_repository() = default;

            asio::awaitable<std::optional<janus::response_message>> get_info();
            asio::awaitable<std::optional<janus::response_message>> create_video_meeting();
            asio::awaitable<std::optional<janus::response_message>> list_participants(const std::string& body);

        private:
            std::string _host;
            uint16_t port;

            janus::janus context;
    };
}

#endif