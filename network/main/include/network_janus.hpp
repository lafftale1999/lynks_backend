#ifndef NETWORK_JANUS_HPP_
#define NETWORK_JANUS_HPP_

#include "network_common.hpp"
#include "network_secrets.hpp"

#include <boost/asio/experimental/awaitable_operators.hpp>

#include <unordered_map>

using namespace std::chrono_literals;

namespace lynks::network {

    /**
     * @brief Simple asynchronous connect-per-request class for sending http request to
     * a janus server.
     * 
     * Uses a static asio::io_context for running the awaitable methods inside.
     * 
     * @attention
     * It is necessary to include the `shutdown()` method in the owners destructor for
     * correct joining of the `context_thread`.
     */
    class janus_request {
        public:

            /**
             * @brief ASYNC
             * 
             * High-level implementation of sending a request to the assigned
             * janus server.
             * 
             * @param request& request to be sent.
             * @param host& host ip in string format.
             * @param port port number in 4-byte unsigned integer format
             * 
             * @return `http_response` if succesful or `std::nullopt` if failed.
             */
            static asio::awaitable<std::optional<http_response>> send_request(const http_request& request, const std::string& host, const uint16_t port);
            
            /**
             * @brief Gracefully shuts down the `context_thread`
             * 
             * @param restartable if `true` will be able to re-use the `context`
             */
            static void shutdown(bool restartable = false);

        private:
            /**
             * @brief Constructs the request object, initializes `socket`
             * with the static `context` and populates `host` and `port`.
             * 
             * @param host can handle both dns and ip
             * @param port port to reach
             */
            janus_request(std::string host, uint16_t port); // <- Pass in reference to context used to send this request <-----------------

            /**
             * @brief Graceful shutdown of the connection
             * for each individual request.
             */
            ~janus_request();

            /**
             * @brief ASYNC
             * 
             * Attempts to connect to the janus server based on `host` and `port`.
             * 
             * @return `true` if connected, else `false`
             */
            asio::awaitable<bool> connect_to_server();

            /**
             * @brief ASYNC
             * 
             * Low-level implementation of sending requests to the janus server.
             * 
             * @param request& the `http_request` to be sent to the server.
             * 
             * @return `true` if write is succesful, else `false`
             */
            asio::awaitable<bool> send_request_impl(const http_request& request);

            /**
             * @brief ASYNC
             * 
             * Reads the incoming response from the janus server.
             * 
             * @return `http_response` if read was succesful, else `std::nullopt`
             */
            asio::awaitable<std::optional<http_response>> read_response();

            asio::ip::tcp::socket socket;
            std::string host;
            std::string port;
            boost::beast::flat_buffer buffer;

        private:
            static std::mutex mtx;
            static std::atomic<bool> is_running;
            static std::thread context_thread;
            static asio::io_context context;
            static void ensure_context_started();

            using work_guard_t =
                asio::executor_work_guard<asio::io_context::executor_type>;
            static std::optional<work_guard_t> work_guard;
    };

    class janus_context {
        public:

            /**
             * @brief Constructs the janus_context for sending and receiving requests to the janus server
             */
            janus_context(std::string host, uint16_t port);

            ~janus_context();

            asio::awaitable<std::optional<bool>> start();
            bool is_connected();

            asio::awaitable<std::optional<janus_response_message>> send_request(const http_request& request, const std::string& host, const uint16_t port);

        private:
            
            /**
             * @brief ASYNC
             * 
             * Read the immediate response sent from Janus. In some cases, the Janus server will send
             * an ACK message, instead of the actual response to the operation directly. This will
             * instead be sent to the `janus_response_buffer`.
             * 
             * @return Either a `janus_response_message` containing relevant information or `std::nullopt` if failed.
             */
            asio::awaitable<std::optional<janus_response_message>> read_response();

            /**
             * @brief ASYNC
             * 
             * Reads from the response buffer instead when waiting for long poll messaging.
             */
            asio::awaitable<std::optional<janus_response_message>> read_from_response_buffer();

            /**
             * @brief ASYNC
             * 
             * Connects to the Janus Server.
             * 
             * @return `true` if connected, `false` if not.
             */
            asio::awaitable<bool> connect();

            /**
             * @brief ASYNC
             * 
             * Continuosly polls the Janus Server for the long polling messages
             * and adds them to the `janus_response_buffer`.
             */
            void long_poll_task();

        private:
            asio::io_context context;
            std::thread context_thread;
            using work_guard_t = asio::executor_work_guard<asio::io_context::executor_type>;
            std::optional<work_guard_t> work_guard;

            std::string host;
            uint16_t port;

            asio::ip::tcp::socket long_poll_socket;
            janus_response_buffer long_poll_buffer;
    };

    // CHECK
    struct janus_response_message {
        std::string event_type;
        std::string transaction;
        std::string body;
    };

    class janus_response_buffer {
        public:
            janus_response_buffer(asio::any_io_executor executor)
            : strand(asio::make_strand(executor)) {}

        public:
        asio::awaitable<void> push(janus_response_message message) {
            asio::dispatch(strand,[this, msg = std::move(message)]() mutable {
                auto t_number = msg.transaction;
                
                auto it = waiters.find(t_number);

                if (it != waiters.end()) {
                    it->second->result = std::move(msg);
                    it->second->signal.cancel();

                    waiters.erase(it);
                    return;
                }

                messages.push_back(std::move(msg));
            });
        }

        asio::awaitable<std::optional<janus_response_message>> wait_for_transaction(
            const std::string& transaction_number,
            std::chrono::steady_clock::duration timeout = 30s   
        ) {
            co_await asio::dispatch(strand, asio::use_awaitable);

            // see if transaction already exists
            if (auto found = try_pop_locked(transaction_number)) {
                co_return found;
            }

            auto new_waiter = std::make_shared<janus_waiter>(strand);
            auto [it, inserted] = waiters.emplace(transaction_number, new_waiter);

            if (!inserted) {
                std::cerr << "[JANUS] already waiting for transaction: " << transaction_number << std::endl;
                co_return std::nullopt;
            }
            
            new_waiter->signal.expires_at(std::chrono::steady_clock::time_point::max());

            asio::steady_timer timeout_timer(strand);
            timeout_timer.expires_after(timeout);

            using namespace asio::experimental::awaitable_operators;

            auto signal_await = new_waiter->signal.async_wait(asio::redirect_error(asio::use_awaitable, new_waiter->signal_ec));
            auto timeout_await = timeout_timer.async_wait(asio::redirect_error(asio::use_awaitable, new_waiter->timeout_ec));

            auto which = co_await (std::move(signal_await) || std::move(timeout_await));

            waiters.erase(transaction_number);

            if (which.index() == 0) {
                co_return new_waiter->result;
            } else {
                co_return std::nullopt;
            }
        }

        private:
            struct janus_waiter {
                explicit janus_waiter(asio::any_io_executor executor) : signal(executor) {}
                asio::steady_timer signal;
                std::optional<janus_response_message> result;
                boost::system::error_code signal_ec;
                boost::system::error_code timeout_ec;
            };

            std::optional<janus_response_message> try_pop_locked(const std::string& transaction) {
                for (auto it = messages.begin(); it != messages.end(); ++it) {
                    if (it->transaction == transaction) {
                        janus_response_message out = std::move(*it);
                        messages.erase(it);
                        return out;
                    }
                }

                return std::nullopt;
            }

        private:
            asio::strand<asio::any_io_executor> strand;
            std::deque<janus_response_message> messages;
            std::unordered_map<std::string, std::shared_ptr<janus_waiter>> waiters;
    };
}

#endif