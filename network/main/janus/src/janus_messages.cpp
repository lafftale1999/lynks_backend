#include "janus_messages.hpp"
#include "nlohmann/json.hpp"

namespace janus::messages {
    namespace session { /* ------------------------ SESSION -----------------------------------------------------------------*/
        
        /**
         * --------------------------------------------------------------------------------------------------------------------------
         * CREATE SESSION REQUEST
         * --------------------------------------------------------------------------------------------------------------------------
         */

        create_session_request::create_session_request(std::string tx)
        : janus("create"), transaction(tx) {}

        std::string create_session_request::to_json() const {
            nlohmann::json json;
            
            json["janus"] = janus;
            json["transaction"] = transaction;

            return json.dump();
        }


        /**
         * --------------------------------------------------------------------------------------------------------------------------
         * CREATE SESSION RESPONSE
         * --------------------------------------------------------------------------------------------------------------------------
         */

        create_session_response::create_session_response(std::string json_str) 
        : json_org(json_str)
        {
            auto json = nlohmann::json::parse(json_str);
            
            janus = json["janus"].get<std::string>();
            transaction = json["transaction"].get<std::string>();
            session_id = std::to_string(json["data"]["id"].get<uint64_t>());
        }

        const std::string& create_session_response::get_json() const {
            return json_org;
        }

        const std::string& create_session_response::get_transaction() const {
            return transaction;
        }

        const std::string& create_session_response::get_session_id() const {
            return session_id;
        }

        const std::string& create_session_response::get_janus() const {
            return janus;
        }

        /**
         * --------------------------------------------------------------------------------------------------------------------------
         * ATTACH PLUGIN REQUEST
         * --------------------------------------------------------------------------------------------------------------------------
         */

        attach_plugin_request::attach_plugin_request(std::string tx, std::string plugin_name) 
        : janus("attach"), plugin_name(plugin_name), transaction(tx) {}

        std::string attach_plugin_request::to_json() {
            nlohmann::json json;

            json["janus"] = janus;
            json["plugin"] = plugin_name;
            json["transaction"] = transaction;

            return json.dump();
        }

        /**
         * --------------------------------------------------------------------------------------------------------------------------
         * ATTACH PLUGIN RESPONSE
         * --------------------------------------------------------------------------------------------------------------------------
         */

        attach_plugin_response::attach_plugin_response(std::string json_str)
        : json_org(json_str)
        {
            
            auto json = nlohmann::json::parse(json_str);
            janus = json["janus"].get<std::string>();
            transaction = json["transaction"].get<std::string>();
            plugin_handle = json["data"]["id"].get<std::string>();
        }

        const std::string& attach_plugin_response::get_json() const {
            return json_org;
        }

        const std::string& attach_plugin_response::get_transaction() const {
            return transaction;
        }

        const std::string& attach_plugin_response::get_plugin_handle() const {
            return plugin_handle;
        }

        const std::string& attach_plugin_response::get_janus() const {
            return janus;
        }

        /**
         * --------------------------------------------------------------------------------------------------------------------------
         * KEEP ALIVE MESSAGE
         * --------------------------------------------------------------------------------------------------------------------------
         */

        keep_alive_message::keep_alive_message(std::string json_str)
        : json_org(json_str) {
            auto json = nlohmann::json::parse(json_str);

            janus = json["janus"].get<std::string>();
        }

        const std::string& keep_alive_message::get_janus() const {
            return janus;
        }

        const std::string& keep_alive_message::get_json() const {
            return json_org;
        }

    } // session

    namespace video_room { /* ------------------------ video_room -----------------------------------------------------------------*/
        
        /**
         * --------------------------------------------------------------------------------------------------------------------------
         * CREATE ROOM REQUEST
         * --------------------------------------------------------------------------------------------------------------------------
         */
    
        create_room_request::create_room_request()
        : request("create"), is_private("false") {}

        std::string create_room_request::to_json() const {
            nlohmann::json json;

            json["request"] = request;
            json["is_private"] = is_private;

            return json.dump();
        }

        /**
         * --------------------------------------------------------------------------------------------------------------------------
         * CREATE ROOM RESPONSE
         * --------------------------------------------------------------------------------------------------------------------------
         */

        create_room_response::create_room_response(std::string json_str)
        : json_org(json_str)
        {
            auto json = nlohmann::json::parse(json_str);

            videoroom = json["videoroom"].get<std::string>();
            room_id = json["room"].get<std::string>();
        }   
        
        const std::string& create_room_response::get_json() const {
            return json_org;
        }
        
        const std::string& create_room_response::get_video_room() const {
            return videoroom;
        }
        
        const std::string& create_room_response::get_room_id() const {
            return room_id;
        }
    }
}