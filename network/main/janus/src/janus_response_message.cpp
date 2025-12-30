#include "janus_response_message.hpp"
#include "nlohmann/json.hpp"

namespace janus {
    response_message::response_message(std::string json_str) {
        *this = std::move(parse_json(json_str));
    }

    response_message::response_message(std::string ev_type, std::string transaction, std::string body) {
        this->event_type = ev_type;
        this->transaction = transaction;
        this->body = body;
    }

    const std::string& response_message::get_event_type() const {
        return event_type;
    }
    
    const std::string& response_message::get_transaction() const {
        return transaction;
    }

    const std::string& response_message::get_body() const {
        return body;
    }

    response_message response_message::parse_json(std::string json_str) {
        response_message temp;

        auto json = nlohmann::json::parse(json_str);
        temp.event_type = json["janus"].get<std::string>();
        if (json.contains("transaction"))
            temp.transaction = json["transaction"].get<std::string>();
        
        temp.body = json.dump();

        return temp;
    }

    std::string response_message::to_json() const {
        try {
            nlohmann::json json;
            json["janus"] = event_type;
            json["transaction"] = transaction;
            json["data"] = body;

            return json.dump();
        } catch (const std::exception& e) {
            std::cerr << "[JANUS] failed to parse message:\n" << *this;
        }

        return "null";
    }

    std::string response_message::to_string() const {
        std::stringstream ss;

        ss <<
            "\n----------------------------------------------\n" <<
            "MESSAGE:\t" << transaction << std::endl <<
            "TYPE:\t" << event_type << std::endl <<
            "BODY:\n" << body <<
            "\n----------------------------------------------\n";
        
        return ss.str();
    }

    std::ostream& operator << (std::ostream& os, const response_message& msg) {
        return os << 
            "\n----------------------------------------------\n" <<
            "MESSAGE:\t" << msg.transaction << std::endl <<
            "TYPE:\t" << msg.event_type << std::endl <<
            "BODY:\n" << msg.body <<
            "\n----------------------------------------------\n";
    }
}