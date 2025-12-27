#include "user_service.hpp"

namespace lynks::network {
    user_service::user_service(db_connection& db) 
    : user_repo(db) {}

    awaitable_opt_str user_service::log_in_user(const std::string& request_body_json) {
        user temp(request_body_json);

        auto result = co_await user_repo.find_user_by_username(temp.get_username());

        if (!result) co_return std::nullopt;

        auto& fetched_user = *result;

        if (!(temp.get_password() == fetched_user.get_password())) {
            co_return std::nullopt;
        }

        co_return "{\"action\": \"succesful\"}";
    }

    /* awaitable_opt_str user_service::create_session(const user& _user) {

    }

    awaitable_opt_str user_service::join_session(const user& _user, const std::string& session_id) {

    }

    awaitable_opt_str user_service::end_session(const user& _user, const std::string& session_id) {

    } */
}