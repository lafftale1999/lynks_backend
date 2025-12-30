#include "janus_temporary_connection.hpp"

namespace janus {
    
    asio::awaitable<std::optional<http_response>> temporary_connection::send_request(asio::io_context& context, const http_request& request, const std::string& host, const uint16_t port) {
        auto connection = temporary_connection(context, host, port);

        auto connected = co_await connection.connect_to_server();
        if (!connected) co_return std::nullopt;

        auto request_sent = co_await connection.send_request_impl(request); 
        if (!request_sent) co_return std::nullopt;

        auto response = co_await connection.read_response();
        if (!response) co_return std::nullopt;

        co_return *response;
    }

    temporary_connection::temporary_connection(asio::io_context& context, std::string host, uint16_t port)
    : context(context), socket(context), host(std::move(host)), port(std::move(std::to_string(port))) {}

    temporary_connection::~temporary_connection() {
        socket.close();
    }

    asio::awaitable<bool> temporary_connection::connect_to_server() {

        /* auto ex = co_await asio::this_coro::executor;

        if (ex != context.get_executor()) {
            std::cerr << "[JANUS] BUG: coroutine is not running on janus context executor\n";
            co_return false;
        } */

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
            std::cerr << "[JANUS] resolve failed: " << ec.message() << std::endl;
            co_return false;
        }

        // Connect to server
        auto result = co_await boost::asio::async_connect(socket, endpoints, token);
        if (ec) {
            std::cerr << "[JANUS] connection failed: " << ec.message() << std::endl;
            co_return false;
        }

        co_return true;
    }

    asio::awaitable<bool> temporary_connection::send_request_impl(const http_request& request) {
        boost::system::error_code ec;
        auto token = asio::cancel_after(
            std::chrono::seconds(5),
            asio::redirect_error(asio::use_awaitable, ec)
        );
        
        co_await http::async_write(socket, request, token);

        if (!ec) co_return true;
       
        std::cerr << "[JANUS] write failed: " << ec.message() << std::endl;
        co_return false;
    }

    asio::awaitable<std::optional<http_response>> temporary_connection::read_response() {
        boost::system::error_code ec;
        auto token = asio::cancel_after(
            std::chrono::seconds(5),
            asio::redirect_error(asio::use_awaitable, ec)
        );

        http_response response;

        co_await http::async_read(socket, buffer, response, token);

        if (ec) {
            std::cerr << "[JANUS] read failed: " << ec.message() << std::endl;
            co_return std::nullopt;
        }

        co_return response;
    }
}