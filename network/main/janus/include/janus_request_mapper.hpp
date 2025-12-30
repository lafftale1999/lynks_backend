#ifndef JANUS_REQUEST_MAPPER_HPP_
#define JANUS_REQUEST_MAPPER_HPP_

#include "janus_common.hpp"
#include <functional>
#include <unordered_map>

namespace janus {
    enum class request_type {
        CREATE_SESSION,
        ACTIVATE_VIDROOM_PLUGIN,
        CREATE_ROOM,
        GET_INFO,
        LONG_POLL_EVENTS
    };
    
    class request_mapper {
        using request_map = std::unordered_map<
                request_type, 
                std::function<http_request(
                    std::optional<std::string>,
                    const std::string&,
                    const std::string&)>
                >;

        using optional_body = std::optional<std::string>;

        public:
            static std::optional<http_request> get_request(
                request_type request_type, 
                optional_body body,
                const std::string& host,
                const std::string& path
            );

        private:
            static request_map _request_map;

            static void set_up_standard_post_request(
                http_request& request,
                optional_body body,
                const std::string& host,
                const std::string& path
            );

            static http_request create_session(optional_body body, const std::string& host, const std::string& path);
            static http_request activate_vidroom_plugin(optional_body body, const std::string& host, const std::string& path);
            static http_request create_room(optional_body body, const std::string& host, const std::string& path);
            static http_request get_info(optional_body body, const std::string& host, const std::string& path);
            static http_request long_poll_events(optional_body body, const std::string& host, const std::string& path);
            static void post_request_validator(optional_body body);
    };
}

#endif