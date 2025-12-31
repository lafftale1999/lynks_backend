/**
 * @author lafftale1999
 * 
 * @brief Defines the lynks::network::connection class, which represents a single asynchronous TCP 
 * connection between the HTTP server and a client. It is responsible for managing the full lifetime 
 * of a client connection, including reading incoming HTTP requests, forwarding them into a shared 
 * request queue for processing, and serializing outgoing HTTP responses back to the client.
 */

#ifndef NETWORK_CONNECTION_HPP_
#define NETWORK_CONNECTION_HPP_

#include "network_queue.hpp"
#include "network_message_handler.hpp"

namespace lynks {
    namespace network {
        /**
         * @brief The connection between server and client.
         * 
         * @note Inherits from `std::enable_shared_from_this<connection>` which enables to create shared ptrs to this instance.
         * 
         */
        class connection : public std::enable_shared_from_this<connection> {
            public:
                /**
                 * @brief Instantiates the connection.
                 * 
                 * @param context& reference to the context in which the connection is used.
                 * @param socket the socket to which the client is connected.
                 * @param requests& reference to the requests queue. This is used for pushing incoming requests to.
                 */
                connection(asio::io_context& context, asio::ip::tcp::socket socket, queue<owned_message_handle<http_request>>& requests) 
                : socket(std::move(socket)), context(context), requests(requests) 
                {}
                
                virtual ~connection() {
                    disconnect();
                }

                /**
                 * @brief Checks if the client is connected and calls `read_request()`.
                 * 
                 * @param uid unique id. 0 as default.
                 */
                void connect_to_client(uint32_t uid = 0) {
                    if (socket.is_open()) {
                        id = uid;
                        read_request();
                    }
                }

                /**
                 * @brief Disctonnect the client from the server.
                 */
                void disconnect() {
                    if (is_connected()) {
                        boost::asio::post(context, [this](){ socket.close(); });
                    }
                }

                /**
                 * @brief Checks if the socket is open.
                 * 
                 * @return `bool`
                 */
                bool is_connected() const {
                    return socket.is_open();
                }

                /**
                 * @brief Sends the response to the client.
                 * 
                 * @param response& the message_handle used to send messages to the client.
                 */
                void send_response(const lynks::network::message_handle<http_response>& response) {
                    boost::asio::post(context, [this, response]() {
                        bool is_writing = !responses.is_empty();
                        responses.push_back(response);
                        if (!is_writing) write_response();
                    });
                }

                /**
                 * @brief returns the current connection id.
                 * 
                 * @return client id
                 */
                uint32_t get_id() const {
                    return id;
                }

            private:
                /**
                 * @brief 
                 * 
                 * -- ASYNC --
                 * 
                 * Reads incoming requests and calls `add_to_incoming_requests`.
                 * Utilizes the field `request`, which is populated by this method.
                 * 
                 * If the reading fails, the connection will close.
                 */
                void read_request() {
                    boost::beast::http::async_read(socket, buffer, request.data,
                    [this](boost::beast::error_code ec, std::size_t length){
                        if (!ec) {
                            add_to_incoming_requests();
                        } else {
                            std::cout << "[" << id << "] read request failed\n";
                            std::cerr << ec.message() << std::endl;
                            socket.shutdown(asio::ip::tcp::socket::shutdown_both);
                            socket.close();
                        }
                    });
                }

                /**
                 * @brief Pushes the incoming request to the shared `requests`-queue.
                 */
                void add_to_incoming_requests() {
                    auto msg = std::move(request);
                    requests.push_back({ this->shared_from_this(), msg });
                    request = {};
                    read_request();
                }

                /**
                 * @brief
                 * 
                 * -- ASYNC --
                 * 
                 * Keeps writing responses from `responses` until the queue is empty.
                 */
                void write_response() {
                    boost::beast::http::async_write(socket, responses.front().data,
                    [this](boost::beast::error_code ec, std::size_t length){
                        if (!ec) {
                            responses.pop_front();

                            if (!responses.is_empty()) write_response();
                        } else {
                            std::cout << "[" << id << "] failed to write response\n";
                            std::cerr << "Error message: " << ec.message() << std::endl;
                            socket.close();
                        }
                    });
                }

            private:
                asio::ip::tcp::socket socket;
                asio::io_context& context;

                lynks::network::queue<lynks::network::owned_message_handle<http_request>>& requests;
                lynks::network::queue<lynks::network::message_handle<http_response>> responses;

                uint32_t id;
                lynks::network::message_handle<http_request> request;
                boost::beast::flat_buffer buffer;
        };
    } // network
} // lynks

#endif