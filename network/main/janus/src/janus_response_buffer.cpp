#include "janus_response_buffer.hpp"

namespace janus {
    response_buffer::response_buffer(asio::any_io_executor executor)
    : strand(asio::make_strand(executor)) {}

    asio::awaitable<void> response_buffer::push(response_message message) {
        asio::dispatch(strand,[this, msg = std::move(message)]() mutable {
            auto t_number = msg.get_transaction();
            
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

    
    asio::awaitable<std::optional<response_message>> response_buffer::wait_for_transaction(
        const std::string& transaction_number,
        std::chrono::steady_clock::duration timeout  
    ) {
        co_await asio::dispatch(strand, asio::use_awaitable);

        // see if transaction already exists
        if (auto found = try_pop_locked(transaction_number)) {
            co_return found;
        }

        auto new_waiter = std::make_shared<waiter>(strand);
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


    std::optional<response_message> response_buffer::try_pop_locked(const std::string& transaction) {
        for (auto it = messages.begin(); it != messages.end(); ++it) {
            if (it->get_transaction() == transaction) {
                response_message out = std::move(*it);
                messages.erase(it);
                return out;
            }
        }

        return std::nullopt;
    }
}