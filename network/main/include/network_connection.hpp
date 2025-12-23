#ifndef NETWORK_CONNECTION_HPP_
#define NETWORK_CONNECTION_HPP_

#include "network_queue.hpp"
#include "network_message_handler.hpp"

namespace lynks {
    namespace network {
        class connection : public std::enable_shared_from_this<connection> {
            public:
                connection(asio::io_context& context, asio::ip::tcp::socket socket, lynks::network::queue<lynks::network::owned_message_handle<http_request>>& requests) 
                : socket(std::move(socket)), context(context), requests(requests) 
                {}
                
                virtual ~connection() {}

                void connect_to_client(uint32_t uid = 0) {
                    if (socket.is_open()) {
                        id = uid;
                        read_request();
                    }
                }

                void disconnect() {
                    if (is_connected()) {
                        boost::asio::post(context, [this](){ socket.close(); });
                    }
                }

                bool is_connected() const {
                    return socket.is_open();
                }

                void send_response(const lynks::network::message_handle<http_response>& response) {
                    boost::asio::post(context, [this, response]() {
                        bool is_writing = !responses.is_empty();
                        responses.push_back(response);
                        if (!is_writing) write_response();
                    });
                }

                uint32_t get_id() const {
                    return id;
                }

            // ASYNC METHODS
            private:
                void read_request() {
                    boost::beast::http::async_read(socket, buffer, request.data,
                    [this](boost::beast::error_code ec, std::size_t length){
                        if (!ec) {
                            add_to_incoming_requests();
                        } else {
                            std::cout << "[" << id << "] read request failed\n";
                            socket.close();
                        }
                    });
                }

                void add_to_incoming_requests() {
                    auto msg = std::move(request);
                    requests.push_back({ this->shared_from_this(), msg });
                    request = {};
                    read_request();
                }

                void write_response() {
                    boost::beast::http::async_write(socket, responses.front().data,
                    [this](boost::beast::error_code ec, std::size_t length){
                        if (!ec) {
                            responses.pop_front();

                            if (!responses.is_empty()) write_response();
                        } else {
                            std::cout << "[" << id << "] failed to write response\n";
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