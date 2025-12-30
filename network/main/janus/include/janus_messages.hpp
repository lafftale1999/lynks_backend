#ifndef JANUS_MESSAGES_HPP_
#define JANUS_MESSAGES_HPP_

#include "janus_common.hpp"

// create session request
// create session response

namespace janus::messages {
    namespace session {
        class create_session_request {
            public:
                create_session_request(std::string tx);
                std::string to_json() const;

            private:
                std::string janus;
                std::string transaction;
        };

        class create_session_response {
            public:
                create_session_response(std::string json_str);
                const std::string& get_json() const;
                const std::string& get_transaction() const;
                const std::string& get_session_id() const;
                const std::string& get_janus() const;

            private:
                std::string json_org;
                std::string janus;
                std::string transaction;
                std::string session_id;
        };

        class attach_plugin_request {
            public:
                attach_plugin_request(std::string tx, std::string plugin_name);
                std::string to_json();

            private:
                std::string janus;
                std::string plugin_name;
                std::string transaction;
        };

        class attach_plugin_response {
            public:
                attach_plugin_response(std::string json_str);
                const std::string& get_json() const;
                const std::string& get_transaction() const;
                const std::string& get_plugin_handle() const;
                const std::string& get_janus() const;

            private:
                std::string json_org;
                std::string janus;
                std::string transaction;
                std::string plugin_handle;
        };
    
        class keep_alive_message {
            public:
                keep_alive_message(std::string json_str);
                
                const std::string& get_janus() const;
                const std::string& get_json() const;

            private:
                std::string json_org;
                std::string janus;
        };
    }
    
    namespace video_room {
        class create_room_request {
            public:
                create_room_request();
                std::string to_json() const;

            private:
                std::string request;
                std::string is_private;
        };

        class create_room_response {
            public:
                create_room_response(std::string json_str);
                const std::string& get_json() const;
                const std::string& get_video_room() const;
                const std::string& get_room_id() const;

            private:
                std::string json_org;
                std::string videoroom;
                std::string room_id;
        };
    }
}

#endif