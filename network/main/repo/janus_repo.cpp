#include "janus_repo.hpp"

#include "janus_request_mapper.hpp"
#include "janus_messages.hpp"

namespace lynks::network {
    janus_repository::janus_repository(std::string host, uint16_t port) 
    : _host(std::move(host)), port(std::move(port)), context(_host, port) {}

    asio::awaitable<std::optional<janus::response_message>> janus_repository::get_info() {
        auto request = janus::request_mapper::get_request(
            janus::request_type::GET_INFO,
            std::nullopt, _host, "/janus/info"
        );

        co_return co_await context.send_request(*request, _host, port);
    }

    asio::awaitable<std::optional<janus::response_message>> janus_repository::create_video_meeting() {
        janus::messages::video_room::create_room_request msg_request;

        auto request = janus::request_mapper::get_request(
            janus::request_type::CREATE_ROOM,
            msg_request.to_json(),
            _host, context.get_path()
        );

        co_return co_await context.send_request(*request, _host, port);
    }

    asio::awaitable<std::optional<janus::response_message>> janus_repository::list_participants(const std::string& body) {
        janus::messages::video_room::list_participants_request msg_request(body);

        auto request = janus::request_mapper::get_request(
            janus::request_type::LIST_MEETING_PARTICIPANTS,
            msg_request.to_json(),
            _host, context.get_path()
        );

        co_return co_await context.send_request(*request, _host, port);
    }
}
