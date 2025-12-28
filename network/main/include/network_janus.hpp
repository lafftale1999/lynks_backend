#ifndef NETWORK_JANUS_HPP_
#define NETWORK_JANUS_HPP_

#include "network_common.hpp"

namespace lynks::network {

    class janus_request {
        public:
            static asio::awaitable<std::optional<http_response>> send_request(const http_request& request, const std::string& host, const uint16_t port);
            static void shutdown(bool restartable = false);

        private:
            janus_request(std::string host, uint16_t port);
            asio::awaitable<bool> connect_to_server();
            asio::awaitable<bool> send_request_impl(const http_request& request);
            asio::awaitable<std::optional<http_response>> read_response();

            asio::ip::tcp::socket socket;
            std::string host;
            std::string port;
            boost::beast::flat_buffer buffer;

        private:
            static std::mutex mtx;
            static std::atomic<bool> is_running;
            static std::thread context_thread;
            static asio::io_context context;
            static void ensure_context_started();

            using work_guard_t =
                asio::executor_work_guard<asio::io_context::executor_type>;
            static std::optional<work_guard_t> work_guard;
    };
}

#endif