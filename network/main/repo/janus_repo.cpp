#include "janus_repo.hpp"

namespace lynks::network {
    janus_repository::janus_repository(std::string host, uint16_t port) 
    : host(std::move(host)), port(std::move(port)) {}

    janus_repository::~janus_repository() {
        janus_request::shutdown();
    }

    asio::awaitable<std::optional<http_response>> janus_repository::get_info() {
        http_request request{};
        request.target("/janus/info");
        request.method(http::verb::get);
        request.version(11);
        request.set(http::field::host, host);
        request.set(http::field::user_agent, "lynks/1.0");
        request.set(http::field::accept, "application/json");
        request.set(http::field::connection, "close");

        co_return co_await janus_request::send_request(request, host, port);
    }
}
