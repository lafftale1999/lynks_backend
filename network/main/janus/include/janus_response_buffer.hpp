#ifndef JANUS_RESPONSE_BUFFER_HPP_
#define JANUS_RESPONSE_BUFFER_HPP_

#include "janus_common.hpp"
#include "janus_response_message.hpp"

#include <boost/asio/experimental/awaitable_operators.hpp>
#include <unordered_map>

using namespace std::chrono_literals;

namespace janus {
    
    /**
     * @brief Buffer for incoming messages from the long polling logic for
     * the Janus REST API server.
     */
    class response_buffer {
        public:
            response_buffer(asio::any_io_executor executor);

            /**
             * @brief ASYNC
             * 
             * Serialized method for pushing incoming messages into the
             * `buffer`. This will also implicitly see if any `waiters`
             * are waiting for the incoming `message`.
             * 
             * @param `message` the incoming message
             */
            asio::awaitable<void> push(response_message message);

            /**
             * @brief ASYNC
             * 
             * Serialized method for waiting until the message related to the passed
             * `transaction_number` has been pushed into the buffer.
             * 
             * @param transaction_number& identifying the message we are waiting for
             * @param timeout deciding the time before timeout
             * 
             * @return `response_message` if successful, `std::nullopt` if not.
             * 
             * @attention The Janus long polling loop is on 30 seconds.
             */
            asio::awaitable<std::optional<response_message>> wait_for_transaction(
                const std::string& transaction_number,
                std::chrono::steady_clock::duration timeout = 30s   
            );

        private:
            /**
             * @brief Representation of the object waiting for a response
             * over the long polling logic.
             */
            struct waiter {
                explicit waiter(asio::any_io_executor executor) : signal(executor) {}
                
                asio::steady_timer                  signal;     /**< The signal used for returning to the coroutine */
                std::optional<response_message>     result;     /**< The result for the operations */
                boost::system::error_code           signal_ec;  /**< Error code for errors in signal */
                boost::system::error_code           timeout_ec; /**< Error code for errors when timed out */
            };

            /**
             * @brief This function in itself is not serialized, and assumes that
             * the caller already has aqcuired synchronized mechanisms.
             * 
             * @param transaction& the transaction target
             * 
             * @return `response_message` if found, else `std::nullopt`
             */
            std::optional<response_message> try_pop_locked(const std::string& transaction);

        private:
            asio::strand<asio::any_io_executor>                             strand;     /*< ensures serialization in functions using the strand*/
            std::deque<response_message>                                    messages;   /*< buffer for incoming messages*/
            std::unordered_map<std::string, std::shared_ptr<waiter>>        waiters;    /*< map containing all the resources waiting for responses*/
    };
}



#endif