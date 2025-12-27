#include "user_repo.hpp"

namespace lynks::network {
    
    /* 
    --------------------------- CONSTRUCTOR --------------------------------------
    */
    user_repository::user_repository(db_connection& _db) 
    : db(_db) {}

    /* 
    --------------------------- PUBLIC MEMBER FUNCTIONS --------------------------------------
    */

    asio::awaitable<std::optional<user>> user_repository::find_user_by_id(const uint32_t id) {
        auto result = co_await db.send_query(
            "SELECT * FROM `users` WHERE user.id = ?",
            id
        );

        if (!result) co_return std::nullopt;

        co_return construct_user_from_result(*result);
    }

    asio::awaitable<std::optional<user>> user_repository::find_user_by_username(const std::string& username) {
        auto result = co_await db.send_query(
            "SELECT * FROM `users` WHERE user.username = ?",
            username
        );

        if (!result) co_return std::nullopt;

        co_return construct_user_from_result(*result);
    }

    /* 
    --------------------------- PRIVATE MEMBER FUNCTIONS --------------------------------------
    */

    std::optional<user> user_repository::construct_user_from_result(const mysql::results& result) {
        auto sets = result.rows();
        
        if (sets.empty()) return std::nullopt;

        mysql::row_view row = result.rows().front();

        std::int64_t id = row.at(0).as_int64();
        std::string username = row.at(1).as_string();
        std::string password = row.at(2).as_string();

        try {
            user temp(id, username, password);
            return temp;
        } catch (const std::exception& e) {
            std::cerr << e.what();
            return std::nullopt;
        }
    }
}