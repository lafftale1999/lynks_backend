#ifndef NETWORK_MYSQL_HPP_
#define NETWORK_MYSQL_HPP_

#include "network_common.hpp"
#include "network_queue.hpp"

#include <boost/mysql/any_connection.hpp>
#include <boost/mysql/connection_pool.hpp>
#include <boost/mysql/connect_params.hpp>
#include <boost/mysql/error_with_diagnostics.hpp>
#include <boost/mysql/results.hpp>

#include "network_secrets.hpp"

namespace mysql = boost::mysql;

namespace lynks {
    namespace network {
        class db_connection {
            public:
                /* 
                Creates the db_connection and initializes the connection_pool. This will grow
                with available connections as our server start using it more. The default cap
                is set at 151 available pooled_connections.

                @param context& the context for the server which the connection is needed for.
                */
                explicit db_connection(asio::io_context& context);

                /* 
                ASYNC
                Sends your prepared query to the db. Templated function to be able to accept
                any parameter fitting for `mysql::field_view`.

                @param sql a statement template for example "SELECT * WHERE user.id = ?"
                @param params perfect forwarding of parameters fitting for mysql::field_view

                @return std::optional with the result of the query. If the query failed the 
                returned object will be of type `std::nullopt`. 
                */
                template<class... Params>
                asio::awaitable<std::optional<mysql::results>> send_query(
                    std::string_view sql, Params&&... params
                ) {
                    mysql::field_view arr[] = { mysql::field_view(params)... };
                    co_return co_await send_query_impl(sql, arr, sizeof...(Params));
                }
            
            protected:
                /* 
                ASYNC
                Low-level implementation of the `send_query()` member function. Accepts the
                the high-level wrapper translation from the public function.

                @param sql a statement template for example "SELECT * WHERE user.id = ?"
                @param params pointer to the parameters to be bound to the statement
                @param params_size sizeof(params)

                @return std::optional with the result of the query. If the query failed the 
                returned object will be of type `std::nullopt`. 
                */
                asio::awaitable<std::optional<mysql::results>> send_query_impl(
                    std::string_view sql,
                    mysql::field_view const* params,
                    std::size_t params_size
                );

            private:
                asio::io_context& context;
                mysql::connection_pool connection_pool;

                /* 
                Static initializing function for the parameters needed in the constructor
                for the `db_connection` object.

                @return An initialized `mysql::pool_params` which is used to properly initialize
                the db_connections `connection_pool`.
                */
                static mysql::pool_params get_params();
                
                static constexpr uint8_t MAX_RECONNECT_TRIES = 5;
        };
    }
}

#endif