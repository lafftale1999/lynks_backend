#ifndef USER_SERVICE_HPP_
#define USER_SERVICE_HPP_

#include "network_common.hpp"
#include "user_repo.hpp"
#include "janus_repo.hpp"
#include "network_session_handler.hpp"

using awaitable_opt_str = asio::awaitable<std::optional<std::string>>;

namespace lynks::network {
    class user_service {
        public:
            user_service(db_connection& db);

            awaitable_opt_str log_in_user(const std::string& request_body_json);
            awaitable_opt_str create_meeting(const std::string& token);

            //awaitable_opt_str join_session(const user& _user, const std::string& session_id);
            //awaitable_opt_str end_session(const user& _user, const std::string& session_id);
        private:
            user_repository user_repo;
            janus_repository janus_repo;
            
            session_handler sessions;
    };
}

#endif