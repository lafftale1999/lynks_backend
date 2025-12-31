#include "network_session_handler.hpp"

namespace lynks::network {

    /* 
    --------------------------- CONSTRUCTORS --------------------------------------
    */
    session_handler::session_handler(uint16_t max_sessions)
    : random_int64(0, -1), random_byte(0, 255), max_sessions(max_sessions) {
        cleanup_thread = std::thread([this](){
            cleanup_task();
        });
    }

    /* 
    --------------------------- DESTRUCTORS --------------------------------------
    */
    session_handler::~session_handler() {
        clean.store(false);
        cv.notify_all();

        if (cleanup_thread.joinable()) cleanup_thread.join();
    }

    /* 
    --------------------------- PUBLIC MEMBER FUNCTIONS --------------------------------------
    */
    std::optional<std::string> session_handler::new_session(std::string username) {
        {
            std::scoped_lock<std::mutex> lock(mtx);

            if (sessions.size() < max_sessions) {
                auto token = session_token(username, generate_token());
                auto token_str = token.get_token();
                sessions.push_back(std::move(token));

                return token_str;
            }
        }

        std::cerr << "[SERVER] max_sessions hit\n";
        clean_inactive_sessions();
        return std::nullopt;
    }

    bool session_handler::validate_session(const std::string& token) {
        std::scoped_lock<std::mutex> lock(mtx);
        return is_active(token);
    }

    std::optional<std::string> session_handler::get_username_by_token(const std::string& token) {
        std::scoped_lock<std::mutex> lock(mtx);

        auto it = std::ranges::find_if(sessions, [&token](session_token& t){
            return token == t.get_token();
        });

        if (it != sessions.end()) {
            if (it->is_active()) return it->get_owner();
        }

        return std::nullopt;
    }

    void session_handler::clean_inactive_sessions() {
        panic_clean.store(true);
        cv.notify_one();
    }

    /* 
    --------------------------- PRIVATE MEMBER FUNCTIONS --------------------------------------
    */
    bool session_handler::is_active(const std::string& token) {
        return find_token(token);
    }

    bool session_handler::find_token(const std::string& token) {
        auto it = std::ranges::find_if(sessions, [&token](const session_token& t){
            return token == t.get_token();
        });

        if (it != sessions.end()) {
            return it->validate_token();
        }

        return false;
    }

    void session_handler::cleanup_task() {
        std::unique_lock<std::mutex> lock(mtx);

        while(clean) {
            cv.wait_for(
                lock, 
                std::chrono::milliseconds(CLEANUP_INTERVAL_MS),
                [this]{
                    return !clean || panic_clean;
                }
            );

            if (!clean) break;
            if (!sessions.empty()) {
                auto it = std::ranges::remove_if(sessions, [](session_token& token){
                    return !token.is_active();
                });

                sessions.erase(it.begin(), it.end());
            }
            if (panic_clean) panic_clean.store(false);
        }
    }

    std::string session_handler::generate_token() {
        // First hash
        auto raw = random_int64.generate_number();
        auto hash_1 = crypto::hash256(std::to_string(raw) + generate_word(10));

        // Second hash
        raw = random_int64.generate_number();
        auto hash_2 = crypto::hash256(std::to_string(raw) + generate_word(10));

        // Combine together
        std::stringstream ss;
        ss << hash_1 << hash_2;

        return crypto::hash256(ss.str());
    }

    std::string session_handler::generate_word(size_t word_size) {
        std::string temp;

        for (size_t i = 0; i < word_size; i++) {
            char t = (char)random_byte.generate_number();
            temp.push_back(std::move(t));
        }

        return temp;
    }
}
