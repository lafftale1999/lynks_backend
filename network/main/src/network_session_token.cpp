#include "network_session_token.hpp"

namespace lynks::network {
    /* 
    --------------------------- CONSTRUCTORS --------------------------------------
    */
    session_token::session_token(std::string owner_identifier, std::string token)
    : token(token), owner_identifier(owner_identifier), life_ms(get_now_ms())
    {}

    /* 
    --------------------------- PUBLIC MEMBER FUNCTIONS --------------------------------------
    */

    bool session_token::validate_token() {
        if (is_active()) {
            update_life();
            return true;
        } else {
            return false;
        }
    }

    bool session_token::is_active() const {
        return get_now_ms() - life_ms < max_life_ms;
    }

    const std::string& session_token::get_owner() const {
        return owner_identifier;
    }

    const std::string& session_token::get_token() const {
        return token;
    }

    /* 
    --------------------------- PRIVATE MEMBER FUNCTIONS --------------------------------------
    */

    void session_token::update_life() {
        life_ms = get_now_ms();
    }

    uint64_t session_token::get_now_ms() const {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
    }
}