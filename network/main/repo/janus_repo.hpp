#ifndef JANUS_REPOSITORY_HPP_
#define JANUS_REPOSITORY_HPP_

#include "network_common.hpp"
#include "network_janus.hpp"

namespace lynks::network {
    class janus_repository {
        public:
            janus_repository(std::string host = JANUS_HOST, uint16_t port = JANUS_PORT);
            ~janus_repository();

            asio::awaitable<std::optional<http_response>> get_info();
            asio::awaitable<std::optional<http_response>> create_video_meeting();

        private:
            std::string host;
            uint16_t port;
    };
}

#endif