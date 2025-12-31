/**
 * @author lafftale1999
 * 
 * @brief Defines a simple, templated, thread-safe queue used for passing messages 
 * between asynchronous components of the backend.
 */

#ifndef NETWORK_QUEUE_HPP_
#define NETWORK_QUEUE_HPP_

#include "network_common.hpp"

using lock = std::lock_guard<std::mutex>;

namespace lynks {
    namespace network {

        /* 
        Templated and Thread safe queue with synchronized methods. Method wait() provides a non-blocking wait using condition_variable.
        */
        template <typename T>
        class queue {
        private:
            std::deque<T> q;
            std::mutex mtx;

            std::condition_variable cv;
            std::mutex cv_mutex;

        public:
            queue() = default;
            queue(const queue<T>&) = delete;

            virtual ~queue() { clear(); }

            const T& front() {
                lock l(mtx);
                return q.front();
            }

            const T& back() {
                lock l(mtx);
                return q.back();
            }

            void push_back(const T& item) {
                lock l(mtx);
                q.push_back(std::move(item));

                std::unique_lock<std::mutex> ul(cv_mutex);
                cv.notify_one();
            }

            void emplace_back(const T& item) {
                lock l(mtx);
                q.emplace_back(std::move(item));

                std::unique_lock<std::mutex> ul(cv_mutex);
                cv.notify_one();
            }

            bool is_empty() {
                lock l(mtx);
                return q.empty();
            }

            size_t size() {
                lock l(mtx);
                return q.size();
            }

            void clear() {
                lock l(mtx);
                q.clear();
            }

            T pop_front() {
                lock l(mtx);
                auto item = std::move(q.front());
                q.pop_front();
                return item;
            }

            T pop_back() {
                lock l(mtx);
                auto item = std::move(q.back());
                q.pop_back();
                return item;
            }

            void wait() {
                while(is_empty()) {
                    std::unique_lock<std::mutex> ul(cv_mutex);
                    cv.wait(ul);
                }
            }
        };
    } // network
} // lynks

#endif