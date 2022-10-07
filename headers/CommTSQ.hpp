#pragma once

#include "commons.hpp"
// #include "CommConnection.hpp"

namespace comm {
    template<typename T>
    class tsq {
        public:
            tsq() = default;
            tsq(const tsq<T> &) = delete;
            virtual ~tsq() {clear();}


            const T &front() {
                std::scoped_lock lock(muxQueue);
                return deqQueue.front();
            }

            const T &back() {
                std::scoped_lock lock(muxQueue);
                return deqQueue.back();
            }


            T pop_front() {
                std::scoped_lock lock(muxQueue);

                auto q = std::move(deqQueue.front());
                deqQueue.pop_front();
                
                return q;
            }

            T pop_back() {
                std::scoped_lock lock(muxQueue);

                auto q = std::move(deqQueue.back());
                deqQueue.pop_back();

                return q;
            }


            void push_front(const T &item) {
                std::scoped_lock lock(muxQueue);
                deqQueue.emplace_front(std::move(item));

                std::unique_lock<std::mutex> ul(muxBlock);
                blocking.notify_one();
            }

            void push_back(const T &item) {
                std::scoped_lock lock(muxQueue);
                deqQueue.emplace_back(std::move(item));

                std::unique_lock<std::mutex> ul(muxBlock);
                blocking.notify_one();
            }


            bool empty() {
                std::scoped_lock lock(muxQueue);
                return deqQueue.empty();
            }

            size_t count() {
                std::scoped_lock lock(muxQueue);
                return deqQueue.size();
            }

            void clear() {
                std::scoped_lock lock(muxQueue);
                deqQueue.clear();
            }

            void wait() {
                while(empty()) {
                    std::unique_lock<std::mutex> ul(muxBlock);
                    blocking.wait(ul);
                }
            }

        protected:
            std::mutex muxQueue;
            std::deque<T> deqQueue;

            std::condition_variable blocking;
			std::mutex muxBlock;
    };
}