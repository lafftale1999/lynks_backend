#include "janus_request_mapper.hpp"

static constexpr std::string USER_AGENT = "lynks_janus/1.0";

namespace janus {
    request_mapper::request_map request_mapper::_request_map = 
        {
            {request_type::GET_INFO,                    &request_mapper::get_info},
            {request_type::CREATE_SESSION,              &request_mapper::create_session},
            {request_type::ACTIVATE_VIDROOM_PLUGIN,     &request_mapper::activate_vidroom_plugin},
            {request_type::CREATE_ROOM,                 &request_mapper::create_room},
            {request_type::LONG_POLL_EVENTS,            &request_mapper::long_poll_events},
            {request_type::LIST_MEETING_PARTICIPANTS,   &request_mapper::list_meeting_participants}
        };

    std::optional<http_request> request_mapper::get_request(
        request_type request_type, 
        optional_body body,
        const std::string& host,
        const std::string& path
    ) {
        auto it = _request_map.find(request_type);

        if (it == _request_map.end()) {
            std::cerr << "[JANUS] invalid request type\n";
            return std::nullopt;
        }

        return it->second(std::move(body), host, path);
    }

    void request_mapper::post_request_validator(optional_body body) {
        if (!body) {
            throw std::invalid_argument("[JANUS] failed create session: no body passed as argument");
        }
    }
    
   void request_mapper::set_up_standard_post_request(
        http_request& request,
        optional_body body,
        const std::string& host,
        const std::string& path
    ) {
        post_request_validator(body);
        request.target(path);
        request.method(http::verb::post);
        request.version(11);
        request.set(http::field::host, host);
        request.set(http::field::user_agent, USER_AGENT);
        request.set(http::field::accept, "application/json");
        request.set(http::field::content_type, "application/json");
        request.set(http::field::connection, "close");
        request.body() = *body;
        request.prepare_payload();
    }

    // POST
    http_request request_mapper::create_session(optional_body body, const std::string& host, const std::string& path) {
        http_request request;
        set_up_standard_post_request(request, body, host, path);
        return request;
    }

    // POST
    http_request request_mapper::activate_vidroom_plugin(optional_body body, const std::string& host, const std::string& path) {
        http_request request;
        set_up_standard_post_request(request, body, host, path);
        return request;
    }

    http_request request_mapper::create_room(optional_body body, const std::string& host, const std::string& path) {
        http_request request;
        set_up_standard_post_request(request, body, host, path);
        return request;
    }

    http_request request_mapper::list_meeting_participants(optional_body body, const std::string& host, const std::string& path) {
        http_request request;
        set_up_standard_post_request(request, body, host, path);
        return request;
    }

    http_request request_mapper::get_info(optional_body body, const std::string& host, const std::string& path) {
        http_request request{};
        request.target(path);
        request.method(http::verb::get);
        request.version(11);
        request.set(http::field::host, host);
        request.set(http::field::user_agent, USER_AGENT);
        request.set(http::field::accept, "application/json");
        request.set(http::field::connection, "close");

        return request;
    }

    http_request request_mapper::long_poll_events(optional_body body, const std::string& host, const std::string& path) {
        http_request request{};
        request.target(path);
        request.method(http::verb::get);
        request.version(11);
        request.set(http::field::host, host);
        request.set(http::field::user_agent, USER_AGENT);
        request.set(http::field::accept, "application/json");
        request.set(http::field::connection, "keep-alive");

        return request;
    }
}
