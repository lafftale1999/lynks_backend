#include "network_mysql.hpp"

namespace lynks::network {
    mysql::pool_params db_connection::get_params() {
        mysql::pool_params params;
        params.server_address.emplace_host_and_port(std::string(hostname));
        params.database = std::string(database);
        params.username = std::string (username);
        params.password = std::string(password);
        params.thread_safe = true;

        return params;
    }

    /* 
    --------------------------- CONSTRUCTORS --------------------------------------
    */
    
    db_connection::db_connection(asio::io_context& context) 
    : context(context), connection_pool(context, std::move(get_params()))
    {
        connection_pool.async_run(asio::detached);
    }

    /* 
    --------------------------- PRIVATE MEMBER FUNCTIONS --------------------------------------
    */

    asio::awaitable<std::optional<mysql::results>> db_connection::send_query_impl(
        std::string_view sql,
        mysql::field_view const* params,
        std::size_t params_size
    ) {
        // fetch a connection from the connection_pool
        boost::system::error_code ec;
        mysql::pooled_connection connection = co_await connection_pool.async_get_connection(
            asio::redirect_error(asio::use_awaitable, ec)
        );
        if (ec) co_return std::nullopt;

        // prepare statement
        mysql::statement statement = co_await connection->async_prepare_statement(
            sql,
            asio::redirect_error(asio::use_awaitable, ec)
        );
        if (ec) co_return std::nullopt;

        // execute the query
        mysql::results result;
        auto token = asio::cancel_after(
            std::chrono::seconds(5),
            asio::redirect_error(asio::use_awaitable, ec)
        );
        
        co_await connection->async_execute(
            statement.bind(params, params + params_size),
            result,
            token
        );

        if (ec) co_return std::nullopt;

        co_return result;
    }
}