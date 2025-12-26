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
            private:
                static http_response route_request(const http_request& request) {
                    auto path = request.target();
                    http_response response = {};

                    if (path == "/create") {

                    } else if (path == "/join") {

                    } else if (path == "/leave") {

                    } else {

                    }

                    return response;
                }

                static http_response create_session(const http_request& request) {
                    // TODO: auto db_connection = lynks::network::mysql::get_connection();
                    // TODO: lynks::network::user user(request.body());
                    // TODO: return db_connection.create_session(user);
                }
            
            public:
                static http_response handle_request(const http_request& request) {
                    return route_request(request);
                }

                /* static http_response create_session(const http_request& request) {
                    http::response<http::string_body> response;
                    response.version(request.version());
                    response.result(http::status::ok);
                    response.set(http::field::server, "My HTTP Server");
                    response.set(http::field::content_type, "text/plain");
                    response.body() = "Hello, World!";
                    response.prepare_payload();

                    return response;
                }

                static http_response not_found(const http_request& request) {
                    http::response<http::string_body> response;
                    response.version(request.version());
                    response.result(http::status::not_found);
                    response.set(http::field::server, "My HTTP Server");
                    response.set(http::field::content_type, "text/plain");
                    response.body() = "404 not found";
                    response.prepare_payload();

                    return response;
                } */
        };
    } // network
} // lynks

#endif