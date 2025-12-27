#include "network_session_token.hpp"

namespace lynks::network {
    class session_token {
        public:
            session_token(std::string identifier) {

            }

            bool friend operator <(const session_token& a, const session_token& b) {
                return a.identifier < b.identifier;
            }

        private:
            std::string token;
            std::string identifier;
            uint16_t life_ms;

            static constexpr uint16_t max_life_ms = 30000; 

            std::string generate_token() {
                
            }
    };

    const std::string sessions_handler::add_session(std::string username) {

    }

    bool sessions_handler::is_active(const std::string& token) {

    }

    void sessions_handler::update_session(std::string token) {

    }

    void sessions_handler::end_session(std::string token) {

    }
}
