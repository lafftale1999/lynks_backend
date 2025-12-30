#include "janus_context.hpp"
#include "janus_temporary_connection.hpp"
#include "janus_request_mapper.hpp"
#include "janus_messages.hpp"

namespace janus {
    /**
     * @brief Constructs the janus_context for sending and receiving requests to the janus server
     */
    janus::janus(std::string host, uint16_t port) 
    :   host(std::move(host)), 
        port(std::move(port)), 
        long_poll_socket(context), 
        long_poll_buffer(context.get_executor()),
        work_guard(asio::make_work_guard(context)),
        rnd(0, -1)
    {
        context_thread = std::thread([this]{
            context.run();
        });

        asio::co_spawn(
            context,
            start(),
            asio::detached
        );
    }


    /**
     * @brief Gracefully closes the connectiong and stops the context.
     */
    janus::~janus() {
        stop();
    }

    /*
    -------------------------------------------------------------------------------------------------------------------------------------------------
    ----------------------------------------------- PUBLIC MEMBER FUNCTIONS -------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------
    */
    
    /**
     * @brief ASYNC 
     * 
     * Starts the Janus context by running the `context`, connecting to the
     * server and then starting the `long_poll_task()`.
     * 
     * @return `true` if succesful, `false` if failed.
     */
    asio::awaitable<void> janus::start() {
        auto connected = co_await connect();
        if (!connected) {
            std::cerr << "[JANUS] failed to connect\n";
            stop();
            co_return;
        }

        auto inited = co_await init();
        if (!inited) {
            std::cerr << "[JANUS] failed to init\n";
            stop();
            co_return;
        }

        asio::co_spawn(
            context,
            long_poll_task(),
            asio::detached
        );
        
        std::cout << "[JANUS] successfully initialized!\n";
    }

    /**
     * @brief 
     * 
     * Gracefully shuts down the Janus context.
     */
    void janus::stop() {
        if (long_poll_socket.is_open()) long_poll_socket.close();
        work_guard.reset();
        if (!context.stopped()) context.stop();
        if (context_thread.joinable()) context_thread.join();
    }

    /**
     * @brief Checks if the socket is still open for the
     * Janus server long polling logic.
     * 
     * @return `true` if connected, `false` if not.
     */
    bool janus::is_connected() {
        return long_poll_socket.is_open();
    }


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
    asio::awaitable<std::optional<response_message>> janus::send_request(const http_request& request, const std::string& host, const uint16_t port) {
        auto result = co_await temporary_connection::send_request(context, request, host, port);
        if (!result) co_return std::nullopt;

        std::cout << "\n-------- OUTGOING REQUEST --------\n";
        std::cout << request;
        std::cout << "\n-------- END --------\n";
        
        response_message msg;
        std::cout << *result;

        try {
            response_message tmp(result->body());
            msg = std::move(tmp);

        } catch (const std::exception& e) {
            std::cerr << "[JANUS] failed to parse message\n";
            co_return std::nullopt;
        }
        
        /**
         * If the event type is `ack`, this means the server has received our request - but the result of
         * our request will be sent to the long_poll_buffer instead.
         */
        if (msg.get_event_type() == "ack") {
            std::cout << "[JANUS] ack receiver\n";
            co_return co_await long_poll_buffer.wait_for_transaction(msg.get_transaction());
        }

        co_return msg;
    }

    std::string janus::get_path() const {
        return "/janus/" + session_path + "/" + videoroom_path;
    }


    /*
    -------------------------------------------------------------------------------------------------------------------------------------------------
    ----------------------------------------------- PRIVATE MEMBER FUNCTIONS ------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------
    */
    asio::awaitable<bool> janus::init() {
        co_return co_await init_session();
    }

    asio::awaitable<bool> janus::init_session() {
        messages::session::create_session_request msg_request(generate_string_id());
        auto request = request_mapper::get_request(
            request_type::CREATE_SESSION,
            msg_request.to_json(),
            host, "/janus"
        );
        std::cout << "----------- MESSAGE-------------\n";
        std::cout << *request << std::endl;
        std::cout << "----------- MESSAGE-------------\n";

        auto response = co_await send_request(*request, host, port);
        if (!response) {
            std::cerr << "[JANUS] failed to init session\n";
            co_return false; 
        }
        std::cout << "[JANUS] create session response: " << *response;

        messages::session::create_session_response msg_response(response->get_body());
        session_path = msg_response.get_session_id();

        co_return co_await init_videoroom();
    }

    asio::awaitable<bool> janus::init_videoroom() {
        messages::session::attach_plugin_request msg_request(generate_string_id(), "janus.plugin.videoroom");
        auto request = request_mapper::get_request(
            request_type::ACTIVATE_VIDROOM_PLUGIN,
            msg_request.to_json(),
            host, "/janus/" + session_path
        );

        auto response = co_await send_request(*request, host, port);
        if (!response) {
            std::cerr << "[JANUS] failed to init videoroom\n";
            co_return false;
        }

        std::cout << "[JANUS] init videoroom response: " << *response;
        messages::session::attach_plugin_response msg_response(response->get_body());
        videoroom_path = msg_response.get_plugin_handle();
        std::cout << "VIDEOPATH: " << videoroom_path << std::endl;

        co_return true;
    }

    /**
     * @brief ASYNC
     * 
     * Connects to the Janus Server.
     * 
     * @return `true` if connected, `false` if not.
     */
    asio::awaitable<bool> janus::connect() {
        boost::system::error_code ec;
        auto token = asio::cancel_after(
            std::chrono::seconds(5),
            asio::redirect_error(asio::use_awaitable, ec)
        );

        asio::ip::tcp::resolver resolver(context);
        auto endpoints = co_await resolver.async_resolve(host, std::to_string(port), token);
        if (ec) {
            std::cerr << "[JANUS] resolve failed: " << ec.message() << std::endl;
            co_return false;
        }

        auto connected = co_await asio::async_connect(long_poll_socket, endpoints, token);
        if (ec) {
            std::cerr << "[JANUS] failed to connect: " << ec.message() << std::endl;
            co_return false;
        }

        co_return true;
    }


    /**
     * @brief ASYNC
     * 
     * Continuosly polls the Janus Server for the long polling messages
     * and adds them to the `long_poll_buffer`.
     */
    asio::awaitable<void> janus::long_poll_task() {
        while(1) {
            co_await send_long_poll_request();
            co_await read_long_poll_response();
        }
    }


    /**
     * @brief ASYNC
     * 
     * Sends the long poll GET request to the server and calls the
     * `read_long_poll_response()`.
     */
    asio::awaitable<void> janus::send_long_poll_request() {
        if (session_path.empty()) {
            std::cout << "[JANUS] session_path not initialized properly\n";
            co_return;
        }
        auto request = request_mapper::get_request(
            request_type::LONG_POLL_EVENTS,
            std::nullopt,
            host, "/janus/"+session_path
        );

        boost::system::error_code ec;
        auto token = asio::cancel_after(
            std::chrono::seconds(30),
            asio::redirect_error(asio::use_awaitable, ec)
        );
        
        auto sent = co_await boost::beast::http::async_write(
            long_poll_socket,
            *request,
            token
        );

        if (ec) {
            std::cerr << "[JANUS] failed to write long poll request: " << ec.message() << std::endl;
        }

        co_return;
    }


    /**
     * @brief ASYNC
     * 
     * Reads the incoming responses from the long poll GET request
     * and adds them to the `long_poll_buffer`
     */
    asio::awaitable<void> janus::read_long_poll_response() {
        boost::beast::error_code ec;
        auto token = asio::cancel_after(
            std::chrono::seconds(30),
            asio::redirect_error(asio::use_awaitable, ec)
        );

        while(1) {
            long_temp_response = {};

            co_await boost::beast::http::async_read(
                long_poll_socket,
                long_flat_buffer,
                long_temp_response,
                token
            );

            if (ec) {
                std::cerr << "[JANUS] long poll read failed: " << ec.message() << std::endl;
                co_return;
            }

            try {
                messages::session::keep_alive_message is_keep_alive(long_temp_response.body());
                std::cout << "[JANUS] keep alive received\n";
                co_return;
            } catch (const std::exception& e) {}

            std::cout << "[JANUS] response received:\n";
            std::cout << long_temp_response;

            response_message msg(long_temp_response.body());
            co_await long_poll_buffer.push(msg);

            co_return;
        }
    }

    /**
     * @brief
     * 
     * Used for generating unique ids.
     * 
     * @return 64-char hash string
     */
    std::string janus::generate_string_id() {
        return rnd.generate_token();
    }
}