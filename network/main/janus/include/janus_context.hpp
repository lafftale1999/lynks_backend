/**
 * @author lafftale1999
 * 
 * @brief Defines a high-level C++ client (janus::janus) for interacting with the Janus WebRTC REST API over HTTP.
 * It owns an `asio::io_context` running on its own thread, handles session/plugin initialization (including VideoRoom setup)
 * and implements Janus’s long-polling event loop via a dedicated TCP socket and response buffer. The class exposes async 
 * methods to start/stop the context, send one-off API requests (handling Janus ACK vs. deferred responses)
 *  and generates unique transaction/session identifiers to correlate requests and events.
 */

#ifndef JANUS_CONTEXT_HPP_
#define JANUS_CONTEXT_HPP_

#include "janus_common.hpp"
#include "janus_response_buffer.hpp"
#include "janus_response_message.hpp"
#include "network_crypto.hpp"

using rnd_device = lynks::network::crypto::random_engine<uint64_t>;

namespace janus {
    
    /**
     * @brief High-level implementation of sending requests and receiving responses from
     * the Janus WebRTC REST API server using HTTP.
     */
    class janus {
        public:

            /**
             * @brief Constructs the janus_context for sending and receiving requests to the janus server
             */
            janus(std::string host, uint16_t port);

            /**
             * @brief Gracefully closes the connectiong and stops the context.
             */
            ~janus();

            /**
             * @brief ASYNC 
             * 
             * Starts the Janus context by running the `context`, connecting to the
             * server and then starting the `long_poll_task()`.
             * 
             * @return `true` if succesful, `false` if failed.
             */
            asio::awaitable<void> start();

            /**
             * @brief 
             * 
             * Gracefully shuts down the Janus context.
             */
            void stop();

            /**
             * @brief Checks if the socket is still open for the
             * Janus server long polling logic.
             * 
             * @return `true` if connected, `false` if not.
             */
            bool is_connected();

            /**
             * @brief ASYNC
             * 
             * Opens a temporary connection to the Janus server and sends the https_request. In some cases
             * the janus server will send an ACK message first, instead of the actual response of the operation.
             * 
             * In this case, the method will assume waiting for the response to end up in the `long_poll_buffer` instead.
             * 
             * @param request& finished http_request ready to be sent.
             * 
             * @return `response_message` if succesful, `std::nullopt` if not.
             */
            asio::awaitable<std::optional<response_message>> send_request(const http_request& request, const std::string& host, const uint16_t port);

            std::string get_path() const;

        private:
            asio::awaitable<bool> init_session();
            asio::awaitable<bool> init_videoroom();
            asio::awaitable<bool> init();

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
             * and adds them to the `long_poll_buffer`.
             */
            asio::awaitable<void> long_poll_task();

            /**
             * @brief ASYNC
             * 
             * Sends the long poll GET request to the server and calls the
             * `read_long_poll_response()`.
             */
            asio::awaitable<void> send_long_poll_request();

            /**
             * @brief ASYNC
             * 
             * Reads the incoming responses from the long poll GET request
             * and adds them to the `long_poll_buffer`
             */
            asio::awaitable<void> read_long_poll_response();

            /**
             * @brief
             * 
             * Used for generating unique ids.
             * 
             * @return 64-char hash string
             */
            std::string generate_string_id();

        private:
            using asio_work_guard = asio::executor_work_guard<asio::io_context::executor_type>;

            asio::io_context            context;            /**< Context for the requests sent to Janus */
            std::thread                 context_thread;     /**< Context thread for the `context` */
            asio_work_guard             work_guard;         /**< Prohibits the context from finishing */
            std::string                 host;               /**< IP / DNS for host */
            uint16_t                    port;               /**< Port for host */
            rnd_device                  rnd;                /**< Machine for generating pseudo-random numbers */
            std::string                 session_path;       /**< Path to created session */
            std::string                 videoroom_path;     /**< Path to videoroom plugin when created */
            asio::ip::tcp::socket       long_poll_socket;   /**< Socket for long poll logic */
            boost::beast::flat_buffer   long_flat_buffer;   /**< Used for storing the incoming data on the socket */
            response_buffer             long_poll_buffer;   /**< Response buffer for long poll logic */
            http_response               long_temp_response; /**< Temporary storage for incoming responses */
    };
}

#endif