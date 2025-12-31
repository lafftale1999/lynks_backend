/**
 * @author lafftale1999
 * 
 * @brief Defines lynks::network::session_handler, a thread-safe manager for active login 
 * sessions backed by a fixed-size container of session_token objects. It can create new 
 * sessions (issuing a 64-character token), validate tokens (and refresh their lifetime on 
 * use) and look up the associated username for an authenticated request. To keep the 
 * session set clean, it also runs a dedicated cleanup thread that periodically removes 
 * expired/inactive sessions and can be explicitly called via clean_inactive_sessions().
 */

#ifndef NETWORK_SESSION_HANDLER_HPP_
#define NETWORK_SESSION_HANDLER_HPP_

#include "network_common.hpp"
#include "network_crypto.hpp"
#include "network_queue.hpp"
#include "network_session_token.hpp"

namespace lynks::network {
    
    /**
     * @brief Thread-safe container for handling active sessions.
     * This will also spawn an extra thread for cleaning up expired sessions.
     */
    class session_handler {
        public:
            /**
             * @brief Constructs the session_handler container and spawns
             * a thread with the `cleanup_task()`.
             * 
             * @param max_sessions the maximum amount of sessions able to run at the same
             * time. Set to 1000 by default.
             */
            session_handler(uint16_t max_sessions = 1000);

            /**
             * @brief Destructs the session_handler and joins the cleanup thread
             */
            ~session_handler();

            /**
             * @brief Creates and adds a new session_token to the session_handler container.
             * 
             * @param username this is the username used to identify the user.
             * 
             * @return Either a string containing a 64-char sized hash-string or
             * std::nullopt if it failed.
             */
            std::optional<std::string> new_session(std::string username);
            
            /**
             * @brief Validates the token passed. This will also update the tokens lifetime if it
             * exists and is valid.
             * 
             * @param token The 64-char hash-string of the token.
             * 
             * @return `bool`
             */
            bool validate_session(const std::string& token);

            /**
             * @brief Fetches the username based on the token passed as argument.
             * 
             * @param token The 64-char hash-string of the token.
             * 
             * @return Either a string containing the username or std::nullopt if
             * the token doesn't exist or if it is invalid.
             */
            std::optional<std::string> get_username_by_token(const std::string& token);

            /**
             * @brief Notifies the `cleanup_thread` to clean the container
             * from inactive sessions. This will also happen periodically
             * without explicitly calling this method.
             * 
             * For more info about the periodically cleaning see `cleanup_task()`.
             */
            void clean_inactive_sessions();

        private:
            /**
             * @brief Validates the token. Checking if it's active and updates the
             * life-time.
             * 
             * @param token The 64-char hash-string of the token.
             * 
             * @return `bool`
             */
            bool is_active(const std::string& token);

            /**
             * @brief Searches for the token in the sessions container.
             * 
             * @param token The 64-char hash-string of the token.
             * 
             * @return `true` if the token exists && is_active, `false` if not. 
             */
            bool find_token(const std::string& token);

            /**
             * @brief Thread safe cleanup task ran in `cleanup_thread`.
             * Utilizes the following fields:
             * 
             * - `std::atomic<bool> clean` - If it should continue cleaning.
             * 
             * - `std::atomic<bool> panic_clean` - If it needs panic_cleaning.
             * 
             * - `static constexpr uint16_t CLEANUP_INTERVAL_MS` - Intervals for cleaning.
             */
            void cleanup_task();

            /**
             * @brief Generates a 64-char hash-string suitable as token.
             */
            std::string generate_token();

            /**
             * @brief Generates a word based on `word_size`
             * 
             * @param word_size how many letters the word should contain.
             * 
             * @return string containing the generated ascii characters.
             */
            std::string generate_word(size_t word_size);

        private:
            std::vector<session_token> sessions;
            uint16_t max_sessions;
            
            crypto::random_engine<int64_t> random_int64;
            crypto::random_engine<uint8_t> random_byte;

            std::mutex mtx;
            std::condition_variable cv;
            
            std::thread cleanup_thread;
            std::atomic<bool> clean{true};
            std::atomic<bool> panic_clean{false};
            static constexpr uint16_t CLEANUP_INTERVAL_MS = 30000;  
        };
}

#endif