#include "network_mysql.hpp"

namespace lynks::network {
    mysql::pool_params db_connection::get_params() {
        mysql::pool_params params;
        params.server_address.emplace_host_and_port(
            std::string(MYSQL_HOST),
            MYSQL_PORT
        );
        params.database = MYSQL_DBNAME;
        params.username = MYSQL_USERNAME;
        params.password = MYSQL_PASSWORD;
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
        #ifdef LYNKS_BACKEND_DEBUG 
            debug_incoming_query(sql, params, params_size);
        #endif

        boost::system::error_code ec;
        auto token = asio::cancel_after(
            std::chrono::seconds(5),
            asio::redirect_error(asio::use_awaitable, ec)
        );

        // fetch a connection from the connection_pool
        mysql::pooled_connection connection = co_await connection_pool.async_get_connection(
            token
        );
        if (ec) {
            std::cerr << "[SERVER] Fetching connection failed: " << ec.message() << std::endl;
            co_return std::nullopt;
        }

        // prepare statement
        mysql::statement statement = co_await connection->async_prepare_statement(
            sql,
            token
        );
        if (ec) {
            std::cerr << "[SERVER] Preparing statement failed: " << ec.message() << std::endl;
            co_return std::nullopt;
        }

        // execute the query
        mysql::results result;
        co_await connection->async_execute(
            statement.bind(params, params + params_size),
            result,
            token
        );

        if (ec) {
            std::cerr << "[SERVER] Executing query failed: " << ec.message() << std::endl;
            co_return std::nullopt;
        }

        std::cout << "[SERVER] Query Executed\n";

        co_return result;
    }

    /**
     * @brief Static helper function for printing out field_views.
     */
    static void print_field(const mysql::field_view& f){
    using mysql::field_kind;

    switch (f.kind()) {
        case field_kind::null:
            std::cout << "NULL";
            break;

        case field_kind::int64:
            std::cout << f.as_int64();
            break;

        case field_kind::uint64:
            std::cout << f.as_uint64();
            break;

        case field_kind::float_:
            std::cout << f.as_float();
            break;

        case field_kind::double_:
            std::cout << f.as_double();
            break;

        case field_kind::string:
            std::cout << '"' << f.as_string() << '"';
            break;

        case field_kind::blob:
            std::cout << "<blob>(" << f.as_blob().size() << " bytes)";
            break;

        case field_kind::date:
        case field_kind::datetime:
        case field_kind::time:
            std::cout << f;
            break;

        default:
            std::cout << "<unknown>";
            break;
        }
    }

    void db_connection::debug_incoming_query(
        std::string_view sql,
        mysql::field_view const* params,
        std::size_t params_size
    ) {
        std::cout << "Incoming arguments:\n";
        std::cout << "SQL: " << sql << std::endl;
        std::cout << "Params (" << params_size << "):\n";
        
        for (std::size_t i = 0; i < params_size; ++i) {
            std::cout << "  [" << i << "] = ";
            print_field(params[i]);
            std::cout << "\n";
        }
    }
}