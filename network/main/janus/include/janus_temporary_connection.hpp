/**
 * @author lafftale1999
 * 
 * @brief This header defines janus::temporary_connection, an abstraction for sending single HTTP requests 
 * to the Janus WebRTC REST API using asynchronous I/O.
 * 
 * Rather than maintaining a persistent connection, temporary_connection opens a TCP connection, sends a 
 * prepared http_request, reads the corresponding http_response and then shuts down cleanly. This pattern 
 * matches Janus’s REST interaction model for non–long-poll operations such as session creation, plugin 
 * attachment and room management.
 * 
 * The class exposes a single high-level static coroutine, send_request, which encapsulates DNS resolution, 
 * connection establishment, request transmission and response parsing.
 */

#ifndef JANUS_CONNECTION_HPP_
#define JANUS_CONNECTION_HPP_

#include "janus_common.hpp"

namespace janus {

    /**
     * @brief ASYNC
     * 
     * Exposes the static function `send_request(...)` which opens a temporary connection
     * to the given `http:/host:port`, sends the `http_request`, reads and returns the `http_response`.
     */
    class temporary_connection {
        public:

            /**
             * @brief ASYNC
             * 
             * High-level implementation of sending a request to the assigned
             * janus server.
             * 
             * @param context& the caller of the function
             * @param request& request to be sent.
             * @param host& host ip in string format.
             * @param port port number in 4-byte unsigned integer format
             * 
             * @return `http_response` if succesful or `std::nullopt` if failed.
             */
            static asio::awaitable<std::optional<http_response>> send_request(asio::io_context& context, const http_request& request, const std::string& host, const uint16_t port);

        private:
            /**
             * @brief Constructs the request object, initializes `socket`
             * with the bound reference to the callers `context&` and populates `host` and `port`.
             * 
             * @param context& the caller executor
             * @param host can handle both dns and ip
             * @param port port to reach
             */
            temporary_connection(asio::io_context& context, std::string host, uint16_t port);

            /**
             * @brief Graceful shutdown of the connection
             * for each individual request.
             */
            ~temporary_connection();

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
            
        private:
            asio::io_context&           context;    /*< context of the caller*/
            asio::ip::tcp::socket       socket;     /*< socket for the connection*/
            std::string                 host;       /*< host as string*/
            std::string                 port;       /*< port as string*/
            boost::beast::flat_buffer   buffer;     /*< buffer for reading the response*/
    };
}

#endif