#include "network_session_token.hpp"

namespace lynks::network {

    static uint64_t get_now_ms() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
    }

    class session_token {
        public:
            session_token(std::string identifier, std::string token)
            : token(token), identifier(identifier), life_ms(get_now_ms())
            {}

            bool is_active() {
                auto now = get_now_ms();

                if (now - life_ms < max_life_ms) {
                    life_ms = now;
                    return true;
                }
                
                return false;
            }

            bool friend operator <(const session_token& a, const session_token& b) {
                return a.identifier < b.identifier;
            }

        private:
            std::string token;
            std::string identifier;
            uint64_t life_ms;

            static constexpr uint64_t max_life_ms = 30000; 
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
