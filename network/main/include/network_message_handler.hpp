#ifndef NETWORK_REQUEST_HANDLER_HPP_
#define NETWORK_REQUEST_HANDLER_HPP_

#include "network_common.hpp"

namespace lynks {
    namespace network {
        
        /* Forward declaration of the connection class */
        class connection;

        /* 
        Our response handle for the requests made by the clients.
        */
        template <typename T>
        class message_handle {
            public:
                T data;

            public:
                friend std::ostream& operator << (std::ostream os, const message_handle& response) {
                    os << response;
                    return os;
                }
        };

        template <typename T>
        class owned_message_handle {
            public:
                std::shared_ptr<lynks::network::connection> client_connection = nullptr;
                message_handle<T> msg;
        };
    }
}

#endif