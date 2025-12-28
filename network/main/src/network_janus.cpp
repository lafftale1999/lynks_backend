#include "network_janus.hpp"

namespace lynks::network {

    std::atomic<bool> janus_request::is_running = false;
    std::thread janus_request::context_thread{};
    asio::io_context janus_request::context{};
    std::mutex janus_request::mtx{};
    std::optional<janus_request::work_guard_t> janus_request::work_guard{};

    void janus_request::ensure_context_started() {
        std::scoped_lock<std::mutex> lock(mtx);

        if (is_running.load()) return;

        work_guard.emplace(context.get_executor());

        is_running.store(true);
        context_thread = std::thread([]{
            context.run();
        });
    }

    void janus_request::shutdown(bool restartable){
        std::scoped_lock<std::mutex> lock(mtx);

        if (!is_running.load()) return;

        work_guard.reset();
        context.stop();

        if (context_thread.joinable()) context_thread.join();
        if (restartable) context.restart();
        is_running.store(false);
    }

    asio::awaitable<std::optional<http_response>> janus_request::send_request(const http_request& request, const std::string& host, const uint16_t port) {
        ensure_context_started();

        // Handle to the executor that called this coroutine
        auto caller_executor = co_await asio::this_coro::executor;

        // Initiates an asynchronous function which we can call.
        auto operation = [&](auto completion_handler) {
            // Ensure that the completion is delivered to the callers executor
            auto bound_handler = asio::bind_executor(caller_executor, std::move(completion_handler));

            // Spawn a coroutine which runs within the Janus static asio::io_context
            asio::co_spawn(
                context, // <- Janus static io_context

                // TEACHING:
                // 1. `mutable` keyword enables us to mutate captured variables inside the lambda.
                // 2. `-> ReturnType` declares the returntype, which is necessary for coroutines.
                [request, host, port, h = std::move(bound_handler)]() mutable
                    -> asio::awaitable<void>
                {
                    std::optional<http_response> out = std::nullopt;

                    janus_request jr(host, port);
                    try {
                        if (co_await jr.connect_to_server()) {
                            if (co_await jr.send_request_impl(request)) {
                                out = co_await jr.read_response();
                            }
                        }
                    } catch (const std::exception& e) {
                        std::cerr << "[JANUS] send_request failed: " << e.what() << std::endl;
                    }

                    
                    // Calling h(out) completes the async_initiate operation;
                    // the awaiting coroutine resumes and receives out as its result.
                    h(out);
                    co_return;
                },
                asio::detached
            );
        };
        
        // TEACHING:
        // async_initiate<CompletionToken, Signature>(operation, token);
        // 1. decltype - compilation tool to resolve types of objects before compiling the program
        // 2. In this case, will create completion_handler with the token use_awaitable and the return type Signature
        // 3. Calls operation(completion_handler)
        std::optional<http_response> result = co_await asio::async_initiate<decltype(asio::use_awaitable),
            void(std::optional<http_response>)>(
                operation, asio::use_awaitable
            );

        co_return result;
    }

    janus_request::janus_request(std::string host, uint16_t port)
    : socket(context), host(std::move(host)), port(std::move(std::to_string(port))) {}

    janus_request::~janus_request() {
        socket.close();
    }

    asio::awaitable<bool> janus_request::connect_to_server() {

        auto ex = co_await asio::this_coro::executor;

        if (ex != context.get_executor()) {
            std::cerr << "[JANUS] BUG: coroutine is not running on janus context executor\n";
            co_return false;
        }

        // Error handling
        boost::system::error_code ec;
        auto token = asio::cancel_after(
            std::chrono::seconds(5),
            asio::redirect_error(asio::use_awaitable, ec)
        );

        // Resolve endpoint
        asio::ip::tcp::resolver resolver(context);
        auto endpoints = co_await resolver.async_resolve(host, port, token);
        if (ec) {
            std::cerr << "[JANUS] resolve failed: " << ec.message() << std::endl;
            co_return false;
        }

        // Connect to server
        auto result = co_await boost::asio::async_connect(socket, endpoints, token);
        if (ec) {
            std::cerr << "[JANUS] connection failed: " << ec.message() << std::endl;
            co_return false;
        }

        co_return true;
    }

    asio::awaitable<bool> janus_request::send_request_impl(const http_request& request) {
        boost::system::error_code ec;
        auto token = asio::cancel_after(
            std::chrono::seconds(5),
            asio::redirect_error(asio::use_awaitable, ec)
        );
        
        co_await http::async_write(socket, request, token);

        if (!ec) co_return true;
       
        std::cerr << "[JANUS] write failed: " << ec.message() << std::endl;
        co_return false;
    }

    asio::awaitable<std::optional<http_response>> janus_request::read_response() {
        boost::system::error_code ec;
        auto token = asio::cancel_after(
            std::chrono::seconds(5),
            asio::redirect_error(asio::use_awaitable, ec)
        );

        http_response response;

        co_await http::async_read(socket, buffer, response, token);

        if (ec) {
            std::cerr << "[JANUS] read failed: " << ec.message() << std::endl;
            co_return std::nullopt;
        }

        co_return response;
    }
}