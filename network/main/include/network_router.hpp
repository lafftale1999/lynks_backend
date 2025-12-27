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

                    if (path == "/create") {
                        co_return co_await create_session(request);
                    } else if (path == "/join") {

                    } else if (path == "/leave") {

                    } else {

                    }

                    co_return not_found(request);
                }

                asio::awaitable<http_response> create_session(const http_request& request) {
                    auto result = co_await _user_service.log_in_user(request.body());

                    if (!result) co_return bad_request(request);

                    co_return succesful_request(request, *result);
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