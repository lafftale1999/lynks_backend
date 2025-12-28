#include "network_janus.hpp"

namespace lynks::network {

    std::atomic<bool> janus_request::is_running = false;
    std::thread janus_request::context_thread{};
    asio::io_context janus_request::context{};
    std::mutex janus_request::mtx{};
    std::optional<janus_request::work_guard_t> janus_request::work_guard{};

    void janus_request::ensure_context_started() {
        std::scoped_lock<std::mutex> lock(mtx);

        if (is_running.load()) return;

        work_guard.emplace(context.get_executor());

        is_running.store(true);
        context_thread = std::thread([]{
            context.run();
        });
    }

    void janus_request::shutdown(bool restartable){
        std::scoped_lock<std::mutex> lock(mtx);

        if (!is_running.load()) return;

        work_guard.reset();
        context.stop();

        if (context_thread.joinable()) context_thread.join();
        if (restartable) context.restart();
        is_running.store(false);
    }

    asio::awaitable<std::optional<http_response>> janus_request::send_request(const http_request& request, const std::string& host, const uint16_t port) {
        janus_request jr(host, port);

        if (co_await jr.connect_to_server()) {
            if (co_await jr.send_request_impl(request)) {
                auto result = co_await jr.read_response();

                if (!result) co_return std::nullopt;

                co_return *result;
            }
        }

        co_return std::nullopt;
    }

    janus_request::janus_request(std::string host, uint16_t port)
    : socket(context), host(std::move(host)), port(std::move(std::to_string(port))) {}

    asio::awaitable<bool> janus_request::connect_to_server() {
        if (!ensure_context_started()) {
            std::cerr << "[SERVER] unable to run janus context\n";
            co_return false;
        }

        // Error handling
        boost::system::error_code ec;
        auto token = asio::cancel_after(
            std::chrono::seconds(5),
            asio::redirect_error(asio::use_awaitable, ec)
        );

        // Resolve endpoint
        asio::ip::tcp::resolver resolver(context);
        auto endpoints = co_await resolver.async_resolve(host, port, token);
        if (ec) {
            std::cerr << "[SERVER] failed to resolve janus endpoint: " << ec.message() << std::endl;
            co_return false;
        }

        // Connect to server
        auto result = co_await boost::asio::async_connect(socket, endpoints, token);
        if (ec) {
            std::cerr << "[SERVER] failed to connect to janus server: " << ec.message() << std::endl;
            co_return false;
        }

        co_return true;
    }

    asio::awaitable<bool> janus_request::send_request_impl(const http_request& request) {
        boost::system::error_code ec;
        auto token = asio::cancel_after(
            std::chrono::seconds(5),
            asio::redirect_error(asio::use_awaitable, ec)
        );
        
        co_await http::async_write(socket, request, token);

        if (!ec) co_return true;
       
        std::cerr << "[SERVER] failed to send request to server: " << ec.message() << std::endl;
        co_return false;
    }

    asio::awaitable<std::optional<http_response>> janus_request::read_response() {
        boost::system::error_code ec;
        auto token = asio::cancel_after(
            std::chrono::seconds(5),
            asio::redirect_error(asio::use_awaitable, ec)
        );

        http_response response;

        co_await http::async_read(socket, buffer, response, token);

        if (ec) {
            std::cerr << "[SERVER] failed to read janus response: " << ec.message() << std::endl;
            co_return std::nullopt;
        }

        co_return response;
    }

    
}