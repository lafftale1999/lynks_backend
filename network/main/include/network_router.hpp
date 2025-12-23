#ifndef NETWORK_ROUTER_HPP_
#define NETWORK_ROUTER_HPP_

#include "network_common.hpp"

namespace lynks {
    namespace network {
        /* 
        Class for routing incoming requests.
        */
        class router {
            public:
                static http_response route_request(const http_request& request) {
                    auto path = request.target();
                    std::cout << "[ROUTER] " << path << std::endl;
                    http_response response = {};

                    if (path == "/create") {
                        response = handle_request(request);
                    } else {
                        response = not_found(request);
                    }

                    return response;
                }

                static http_response handle_request(const http_request& request) {
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
                }
        };
    } // network
} // lynks

#endif