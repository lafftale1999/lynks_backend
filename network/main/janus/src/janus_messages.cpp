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
            plugin_handle = std::to_string(json["data"]["id"].get<uint64_t>());
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
        : janus("message"), request("create"), is_private(false), transaction("test123") {}

        std::string create_room_request::to_json() const {
            nlohmann::json json;

            json["janus"] = janus;
            json["transaction"] = transaction;
            json["body"]["request"] = request; 
            json["body"]["is_private"] = is_private;

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
            room_id = json["room"].get<uint64_t>();
            transaction = json["transaction"].get<std::string>();
        }   
        
        const std::string& create_room_response::get_json() const {
            return json_org;
        }
        
        const std::string& create_room_response::get_video_room() const {
            return videoroom;
        }
        
        uint64_t create_room_response::get_room_id() const {
            return room_id;
        }


        /**
         * --------------------------------------------------------------------------------------------------------------------------
         * USER CREATE VIDEO RESPONSE
         * --------------------------------------------------------------------------------------------------------------------------
         */

        user_create_video_response::user_create_video_response(std::string json_str)
        : json_org(json_str) {
            auto json = nlohmann::json::parse(json_str);

            janus = json["janus"].get<std::string>();
            transaction = json["transaction"].get<std::string>();
            room_id = json["plugindata"]["data"]["room"].get<uint64_t>();
        }

        const std::string& user_create_video_response::get_janus() const {
            return janus;
        }

        const std::string& user_create_video_response::get_transaction() const {
            return transaction;
        }

        uint64_t user_create_video_response::get_room_id() const {
            return room_id;
        }

        const std::string& user_create_video_response::get_original_json() const {
            return json_org;
        }

        std::string user_create_video_response::to_json() const {
            nlohmann::json json;

            json["action"] = janus;
            json["room_id"] = room_id;

            return json.dump();
        }


        /**
         * --------------------------------------------------------------------------------------------------------------------------
         * LIST PARTICIPANTS REQUEST
         * --------------------------------------------------------------------------------------------------------------------------
         */

        list_participants_request::list_participants_request(std::string json_str)
        : janus("message"), transaction("test123"), request("listparticipants") {
            auto json = nlohmann::json::parse(json_str);
            room_id = json["room_id"].get<uint64_t>();
        }

        std::string list_participants_request::to_json() const {
            nlohmann::json json;

            json["janus"] = janus;
            json["transaction"] = transaction;
            json["body"]["request"] = request;
            json["body"]["room"] = room_id;

            return json.dump();
        }

        /**
         * --------------------------------------------------------------------------------------------------------------------------
         * LIST PARTICIPANTS RESPONSE
         * --------------------------------------------------------------------------------------------------------------------------
         */


        list_participants_response::list_participants_response(std::string json_str)
        : json_org(json_str)
        {  
            auto json = nlohmann::json::parse(json_str);
            
            janus = json["janus"].get<std::string>();
            transaction = json["transaction"].get<std::string>();
            videoroom = json["plugindata"]["data"]["videoroom"].get<std::string>();
            room_id = json["plugindata"]["data"]["room"].get<uint64_t>();

            if (
                json["plugindata"]["data"].contains("participants") &&
                json["plugindata"]["data"]["participants"].is_array()
            ) {
                for (const auto& p : json["plugindata"]["data"]["participants"]) {
                    if (!p.is_object()) continue;
                    if (!p["publisher"].get<bool>()) continue;
                    if (!p.contains("id")) continue;
                    feed_ids.push_back(p["id"].get<uint64_t>());
                }
            }
        }

        const std::string& list_participants_response::get_janus() const {
            return janus;
        }

        const std::string& list_participants_response::get_transaction() const {
            return transaction;
        }

        const std::string& list_participants_response::get_original_json() const {
            return json_org;
        }

        std::string list_participants_response::to_json() const {
            nlohmann::json json;

            json["action"] = janus;
            json["room_id"] = room_id;
            json["publishers"] = feed_ids;

            return json.dump();
        }

        uint64_t list_participants_response::get_room_id() const {
            return room_id;
        }

        const std::vector<uint64_t>& list_participants_response::get_feed_ids() const {
            return feed_ids;
        }

    }
}