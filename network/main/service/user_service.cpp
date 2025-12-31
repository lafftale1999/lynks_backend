#include "user_service.hpp"
#include "janus_messages.hpp"

namespace lynks::network {
    user_service::user_service(db_connection& db) 
    : user_repo(db){}

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
        if (sessions.validate_session(token)) {
            auto username = sessions.get_username_by_token(token);
            if (!username) co_return std::nullopt;

            auto opt_user = co_await user_repo.find_user_by_username(*username);
            if (!opt_user) co_return std::nullopt;

            auto janus_response = co_await janus_repo.create_video_meeting();
            if (!janus_response) {
                std::cerr << "[SERVICE] failed get information from janus\n";
                co_return std::nullopt;
            }

            janus::messages::video_room::user_create_video_response msg_response(janus_response->get_body());

            co_return msg_response.to_json();
        }
        
        std::cerr << "[SERVICE] WARNING: Sessions invalid\n";
        co_return std::nullopt;
    }

    awaitable_opt_str user_service::list_participants(const std::string& token, const std::string& body) {
        if (sessions.validate_session(token)) {
            auto username = sessions.get_username_by_token(token);
            if (!username) co_return std::nullopt;

            auto opt_user = co_await user_repo.find_user_by_username(*username);
            if (!opt_user) co_return std::nullopt;

            auto janus_response = co_await janus_repo.list_participants(body);
            if (!janus_response) {
                std::cerr << "[SERVICE] failed to get information from janus\n";
                co_return std::nullopt;
            }

            janus::messages::video_room::list_participants_response msg_response(janus_response->get_body());
            co_return msg_response.to_json();
        }

        co_return std::nullopt;
    }
}