#ifndef USER_REPOSITORY_HPP_
#define USER_REPOSITORY_HPP_

#include "network_common.hpp"
#include "network_user.hpp"
#include "network_mysql.hpp"

namespace lynks::network {
    class user_repo {
        public:
            user_repo(db_connection& _db);

            asio::awaitable<std::optional<user>> find_user_by_id(const uint32_t id);
            asio::awaitable<std::optional<user>> find_user_by_username(const std::string& username);

        private:
            db_connection& db;
    };
}

#endif