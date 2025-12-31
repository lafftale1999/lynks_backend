/**
 * @author lafftale1999
 * 
 * @brief This header defines janus::response_message, a lightweight value type representing a single event or 
 * response delivered by Janus via long polling.
 * 
 * The class encapsulates the essential fields needed to route and process Janus messages: the event type,
 * transaction identifier and the associated JSON payload. It provides helpers for parsing raw JSON strings
 * into structured data, serializing messages back to JSON and producing human-readable output for logging
 * and debugging.
 */

#ifndef JANUS_RESPONSE_MESSAGE_HPP_
#define JANUS_RESPONSE_MESSAGE_HPP_

#include "janus_common.hpp"

namespace janus {

    /**
     * @brief Class representing the body of an incoming janus
     * long polling response.
     */
    class response_message {
        public:
            response_message() = default;
            response_message(std::string json_str);
            response_message(std::string ev_type, std::string transaction, std::string body);

            const std::string& get_event_type() const;
            const std::string& get_transaction() const;
            const std::string& get_body() const;
            
            response_message parse_json(std::string json_str);
            std::string to_json() const;
            std::string to_string() const;

            friend std::ostream& operator << (std::ostream& os, const response_message& msg);

        private:
            std::string event_type;     /*< dynamic string for accepting different event types.*/
            std::string transaction;    /*< transaction number for identifying owner of messages*/
            std::string body;           /*< body of the received response*/
    };
}

#endif