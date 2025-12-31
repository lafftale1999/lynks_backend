/**
 * @author lafftale1999
 * 
 * @brief Defines lynks::network::user_repository, a data-access abstraction responsible for retrieving 
 * user records from the database. It uses the shared db_connection to execute asynchronous queries and 
 * maps query results into models.
 */

#ifndef USER_REPOSITORY_HPP_
#define USER_REPOSITORY_HPP_

#include "network_common.hpp"
#include "network_user.hpp"
#include "network_mysql.hpp"

namespace lynks::network {
    class user_repository {
        public:
            user_repository(db_connection& _db);
            asio::awaitable<std::optional<user>> find_user_by_id(const uint32_t id);
            asio::awaitable<std::optional<user>> find_user_by_username(const std::string& username);

        private:
            std::optional<user> construct_user_from_result(const mysql::results& result);

            db_connection& db;
    };
}

#endif