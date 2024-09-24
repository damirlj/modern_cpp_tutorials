#ifndef EVENT20_HPP
#define EVENT20_HPP

// std library
#include <chrono>
#include <vector>
#include <thread>

// Application
#include "Monitor.hpp"

namespace utils
{
    class Event final
    {
        using threads_ids_type = std::vector<std::thread::id>;

      public:
        explicit Event(bool autoReset) noexcept
            : autoReset_{autoReset}
        {}

        ~Event() = default;
        
        // Copy functions forbidden

        Event(const Event&) = delete;
        Event& operator=(const Event&) = delete;

        // Move operations forbidden

        Event(Event&&) = delete;
        Event& operator=(Event&&) = delete;

        inline void wait() noexcept
        {
            {
                auto lock = sync_.getLock();
                if (flag_) return;  // premature signalization
                waitingThreads_.push_back(std::this_thread::get_id());
            }
            auto lock = sync_.wait([this] { return flag_; });
            update_waiting_threads(waitingThreads_);
            if (autoReset_ && waitingThreads_.empty()) flag_ = false;
        }

        inline bool wait_for(std::chrono::milliseconds timeout) noexcept
        {
            {
                auto lock = sync_.getLock();
                if (flag_) return true;  // premature signalization
                waitingThreads_.push_back(std::this_thread::get_id());
            }
            auto [result, lock] = sync_.wait_for(timeout, [this] { return flag_; });
            update_waiting_threads(waitingThreads_);
            if (autoReset_ && waitingThreads_.empty())
                flag_ = false;  // for broadcast to work with auto reset flag set to true

            return result;
        }

        inline void signal() noexcept
        {
            sync_.notify_one([this] { flag_ = true; });
        }

        inline void broadcast() noexcept
        {
            sync_.notify_all([this] { flag_ = true; });
        }

      private:
        inline void update_waiting_threads(threads_ids_type& waitingThreads) noexcept
        {
            waitingThreads_.erase(
                std::remove(waitingThreads.begin(), waitingThreads.end(), std::this_thread::get_id()),
                waitingThreads.end());
        }

      private:
        Monitor<> sync_{};

        const bool autoReset_;
        bool flag_{false};
        threads_ids_type waitingThreads_;
    };
}  // namespace utils
#endif  // EVENT20_HPP
