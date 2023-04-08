//
// <author> damirlj@yahoo.com
// Copyright (c) 2023. All rights reserved!
//

#ifndef EVENT_H
#define EVENT_H

// std library
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <functional> // std::invoke


namespace utils
{

    /**
     *  Implementation of the event synchronization primitive.
     *  Will synchronize threads, one producer: signaling the event to single, or
     *  all consumer threads waiting on the same condition
     */
    class Event final
    {
      public:
        using event_wait_t = enum class EEvent : std::uint8_t { timeout = 0, signaled };

        /**
         * C-tor
         *
         * @param autoReset In case that is set to true, will reset the event after being signaled
         * at consumer point, so that it can wait - block on the same event on the next recall
         */
        explicit Event(bool autoReset) noexcept;
        ~Event();

        // Copy functions forbidden

        Event(const Event&) = delete;
        Event& operator=(const Event&) = delete;

        // Move operations forbidden

        Event(Event&&) = delete;
        Event& operator=(Event&&) = delete;

        /**
         * Wait on the event being signaled, or timeout expired
         *
         * @param timeout Timeout in milliseconds to wait
         * @return Indication of the operation outcome {@link Event#event_wait_t}
         */
        event_wait_t wait_for(std::chrono::milliseconds timeout);

        /**
         * Wait infinitely on event being signaled
         */
        void wait();

        /**
         * Notify - wake up the single thread
         */
        void notify();

        /**
         * Notify - wake up the all waiting threads
         *
         * @note If the event is auto reset - this will notify only the first
         * woken thread
         */
        void broadcast();

        /**
         * Manually reset event - in case of the auto reset is false.
         */
        [[maybe_unused]] void reset();

      private:

        using notify_f = void (std::condition_variable::*)(void);
        void setEvent(notify_f notifier)
        {
            {
                std::scoped_lock lock{m_lock};
                m_predicate = true;
            }
            std::invoke(notifier, m_event);
        }

      private:
        std::condition_variable m_event;  // not copyable nor movable
        std::mutex m_lock;  // not copyable nor movable

        bool m_autoReset;
        bool m_predicate = false;
    };
}  // namespace utils


#endif  // EVENT_H
