#ifndef NETWORK_SERVER_HPP_
#define NETWORK_SERVER_HPP_

#include "network_common.hpp"
#include "network_connection.hpp"
#include "network_router.hpp"
#include "user_service.hpp"

namespace lynks {
    namespace network {
        class server_interface {
            public:
                server_interface(uint16_t port) : 
                    acceptor(context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
                    _db_connection(context), router(_db_connection)
                {

                }

                virtual ~server_interface() {
                    stop();
                }

                bool start() {
                    try {
                        wait_for_client_connection();

                        context_thread = std::thread([this](){
                            context.run();
                        });
                    } catch (const std::exception& e) {
                        std::cerr << "[SERVER] Exception: " << e.what() << std::endl;
                        return false;
                    }

                    std::cout << "[SERVER] started\n";
                    return true;
                }

                bool stop() {
                    context.stop();
                    if (context_thread.joinable()) context_thread.join();

                    std::cout << "[SERVER] stopped\n";
                    return true;
                }

                // ASYNC
                void wait_for_client_connection() {
                    acceptor.async_accept(
                        [this](std::error_code ec, boost::asio::ip::tcp::socket socket) {
                            wait_for_client_connection();
                            
                            if (!ec) {
                                std::cout << "[SERVER] New connection: " << socket.remote_endpoint() << std::endl;

                                std::shared_ptr<connection> new_connect = 
                                    std::make_shared<connection>(context, std::move(socket), requests);

                                if (on_client_connect(new_connect)) {
                                    connected_clients.push_back(std::move(new_connect));
                                    connected_clients.back()->connect_to_client(id_counter++);
                                    std::cout << "[" << connected_clients.back()->get_id() << "] has connected succesfully\n";
                                } else {
                                    std::cerr << "[SERVER] connection denied\n";
                                }
                            } else {
                                std::cerr << "[SERVER] New connection error: " << ec.message() << std::endl;
                            }
                        });
                }

                void update(size_t max_requests = -1, bool wait = false) {
                    size_t request_count = 0;

                    if (wait) requests.wait();

                    while (request_count < max_requests && !requests.is_empty()) {
                        auto request = requests.pop_front();

                        on_request(request.client_connection, request.msg);
                    }
                }

                asio::io_context& get_context() {
                    return context;
                }

            protected:
                virtual bool on_client_connect(std::shared_ptr<connection> client) {
                    return false;
                }

                virtual void on_client_disconnect(std::shared_ptr<connection> client) {
                    std::cout << "[SERVER] client disconnected: " << client->get_id() << std::endl;
                }

                virtual void on_request(std::shared_ptr<connection> client, message_handle<http_request>& request) {
                    if (client->is_connected()) {
                        auto _request = request.data;
                        auto client_keepalive = client;
                        
                        asio::co_spawn(
                            context,
                            [this, client_keepalive, _request]() -> asio::awaitable<void> {
                                http_response raw_response = co_await router.handle_request(_request);
                                message_handle<http_response> response{raw_response};
                                client_keepalive->send_response(response);
                                co_return;
                            },
                            [client_keepalive](std::exception_ptr ptr){
                                if (ptr) {
                                    try {
                                        std::rethrow_exception(ptr);
                                    } catch (const std::exception& e) {
                                        std::cerr << "[SERVER] handled exception: " << e.what() << std::endl;
                                    }
                                }
                            }
                        );

                    } else {
                        on_client_disconnect(client);
                        client.reset();
                        connected_clients.erase(std::remove(connected_clients.begin(), connected_clients.end(), client), connected_clients.end());
                    }
                }
            
            private:
                std::deque<std::shared_ptr<connection>> connected_clients;
                lynks::network::queue<lynks::network::owned_message_handle<http_request>> requests;
                boost::asio::io_context context;
                std::thread context_thread;
                boost::asio::ip::tcp::acceptor acceptor;

                lynks::network::db_connection _db_connection;
                lynks::network::router router;

                uint32_t id_counter = 10000;
        };
    } // network
} // lynks


#endif