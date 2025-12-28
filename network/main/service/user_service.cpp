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

        auto token = sessions.new_session(fetched_user.get_username());
        if (!token) {
            co_return std::nullopt;
        }

        auto json = std::format(
            "{{\"action\": \"succesful\",\"token\": \"{}\"}}",
            *token
        );

        co_return json;
    }

    awaitable_opt_str user_service::create_meeting(const std::string& token) {
        std::cout << "Token: " << token << std::endl;
        
        if (sessions.validate_session(token)) {
            std::cout << "Session is active\n";

            auto username = sessions.get_username_by_token(token);
            if (!username) co_return std::nullopt;
            std::cout << "Username found\n";

            auto opt_user = co_await user_repo.find_user_by_username(*username);
            if (!opt_user) co_return std::nullopt;

            std::cout << "User found\n";

            co_return "{\"action\": \"created\"}";
            // TODO: Create meeting
        }
        std::cout << "Sessions invalid\n";
        co_return std::nullopt;
    }

    /* awaitable_opt_str user_service::join_session(const user& _user, const std::string& session_id) {

    }

    awaitable_opt_str user_service::end_session(const user& _user, const std::string& session_id) {

    } */
}