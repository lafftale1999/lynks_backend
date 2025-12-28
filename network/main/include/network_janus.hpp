#ifndef NETWORK_JANUS_HPP_
#define NETWORK_JANUS_HPP_

#include "network_common.hpp"
#include "network_secrets.hpp"

namespace lynks::network {

    /**
     * @brief Simple asynchronous connect-per-request class for sending http request to
     * a janus server.
     * 
     * Uses a static asio::io_context for running the awaitable methods inside.
     * 
     * @attention
     * It is necessary to include the `shutdown()` method in the owners destructor for
     * correct joining of the `context_thread`.
     */
    class janus_request {
        public:

            /**
             * @brief ASYNC
             * 
             * High-level implementation of sending a request to the assigned
             * janus server.
             * 
             * @param request& request to be sent.
             * @param host& host ip in string format.
             * @param port port number in 4-byte unsigned integer format
             * 
             * @return `http_response` if succesful or `std::nullopt` if failed.
             */
            static asio::awaitable<std::optional<http_response>> send_request(const http_request& request, const std::string& host, const uint16_t port);
            
            /**
             * @brief Gracefully shuts down the `context_thread`
             * 
             * @param restartable if `true` will be able to re-use the `context`
             */
            static void shutdown(bool restartable = false);

        private:
            /**
             * @brief Constructs the request object, initializes `socket`
             * with the static `context` and populates `host` and `port`.
             * 
             * @param host can handle both dns and ip
             * @param port port to reach
             */
            janus_request(std::string host, uint16_t port);

            /**
             * @brief Graceful shutdown of the connection
             * for each individual request.
             */
            ~janus_request();

            /**
             * @brief ASYNC
             * 
             * Attempts to connect to the janus server based on `host` and `port`.
             * 
             * @return `true` if connected, else `false`
             */
            asio::awaitable<bool> connect_to_server();

            /**
             * @brief ASYNC
             * 
             * Low-level implementation of sending requests to the janus server.
             * 
             * @param request& the `http_request` to be sent to the server.
             * 
             * @return `true` if write is succesful, else `false`
             */
            asio::awaitable<bool> send_request_impl(const http_request& request);

            /**
             * @brief ASYNC
             * 
             * Reads the incoming response from the janus server.
             * 
             * @return `http_response` if read was succesful, else `std::nullopt`
             */
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