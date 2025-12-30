#ifndef NETWORK_ROUTER_HPP_
#define NETWORK_ROUTER_HPP_

#include "network_common.hpp"
#include "user_service.hpp"

namespace lynks {
    namespace network {
        /* 
        Class for routing incoming requests.
        */
        class router {
            public:
                router(db_connection& db)
                : _user_service(db) {}

                asio::awaitable<http_response> handle_request(const http_request& request) {
                    return route_request(request);
                }
            
            private:
                asio::awaitable<http_response> route_request(const http_request& request) {
                    auto path = request.target();

                    if (path == "/login") {
                        co_return co_await login_user(request);
                    } else if (path == "/create") {
                        co_return co_await create_meeting(request);
                    } else if (path == "/list_participants") {
                        co_return co_await list_participants(request);
                    } else if (path == "/leave") {

                    } else {
                        std::cout << "[ROUTER] unexpected path received: " << request.target() << std::endl;
                    }

                    co_return not_found(request);
                }

                asio::awaitable<http_response> login_user(const http_request& request) {
                    std::cout << request << std::endl;
                    auto result_string = co_await _user_service.log_in_user(request.body());

                    if (!result_string) co_return bad_request(request);

                    co_return succesful_request(request, *result_string);
                }

                asio::awaitable<http_response> create_meeting(const http_request& request) {
                    try {
                        std::cout << request << std::endl;
                        auto token = request.at(http::field::authorization);
                        std::cout << "[ROUTER] token found: " << token << std::endl;

                        auto result_string = co_await _user_service.create_meeting(token);
                        std::cout << "[ROUTER] meeting created: " << token << std::endl;

                        if (!result_string) co_return bad_request(request);

                        co_return succesful_request(request, *result_string);
                    } catch (const std::exception& e) {
                        std::cerr << "[ROUTER] failed create meeting: " << e.what() << std::endl;
                    }

                    co_return bad_request(request);
                }

                asio::awaitable<http_response> list_participants(const http_request& request) {
                    try {
                        auto token = request.at(http::field::authorization);
                        auto result_string = co_await _user_service.list_participants(token, request.body());
                        if (!result_string) co_return bad_request(request);
                        std::cout << "[DEBUG] body: " << *result_string << std::endl;
                        co_return succesful_request(request, *result_string);
                    } catch (const std::exception& e) {
                        std::cerr << "[ROUTER] list_participants failed: " << e.what() << std::endl;
                    }

                    co_return bad_request(request);
                }

                http_response succesful_request(const http_request& request, const std::string& body) {
                    http::response<http::string_body> response;
                    response.version(request.version());
                    response.result(http::status::ok);
                    response.set(http::field::server, "My HTTP Server");
                    response.set(http::field::content_type, "application/json");
                    response.body() = body;
                    response.prepare_payload();

                    return response;
                }

                http_response not_found(const http_request& request) {
                    http::response<http::string_body> response;
                    response.version(request.version());
                    response.result(http::status::not_found);
                    response.set(http::field::server, "My HTTP Server");
                    response.set(http::field::content_type, "text/plain");
                    response.body() = "404 not found";
                    response.prepare_payload();

                    return response;
                }

                http_response bad_request(const http_request& request) {
                    http::response<http::string_body> response;
                    response.version(request.version());
                    response.result(http::status::bad_request);
                    response.set(http::field::server, "My HTTP Server");
                    response.set(http::field::content_type, "text/plain");
                    response.body() = "400 bad request";
                    response.prepare_payload();

                    return response;
                } 
            
                user_service _user_service;
        };
    } // network
} // lynks

#endif