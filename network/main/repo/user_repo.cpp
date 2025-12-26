#include "user_repo.hpp"

namespace lynks::network {
    
    user_repo::user_repo(db_connection& _db) 
    : db(_db)
    {

    }

    asio::awaitable<std::optional<user>> user_repo::find_user_by_id(const uint32_t id) {
        
    }

    asio::awaitable<std::optional<user>> user_repo::find_user_by_username(const std::string& username) {

    }
}