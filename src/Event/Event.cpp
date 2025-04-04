//
// <author> damirlj@yahoo.com
// Copyright (c) 2023. All rights reserved!
//

#include "Event.h"

Event::Event(bool autoReset) noexcept
    : m_autoReset(autoReset)
{}

Event::~Event() = default;

namespace
{
    inline void updateWaitingThreads(Event::waiting_threads_t& waitingThreads)
    {
        waitingThreads.erase(
            std::remove(waitingThreads.begin(), waitingThreads.end(), std::this_thread::get_id()),
            waitingThreads.end());
    }
}  // namespace

Event::event_wait_t Event::wait_for(std::chrono::milliseconds timeout)
{
    std::unique_lock lock{m_lock};

    event_wait_t outcome = event_wait_t::signaled;

    if (not m_predicate)
    {
        m_waitingThreads.push_back(std::this_thread::get_id());

        const bool signaled = m_event.wait_for(lock, timeout, [this] { return m_predicate; });
        outcome = signaled ? event_wait_t::signaled : event_wait_t::timeout;

        updateWaitingThreads(m_waitingThreads);
    }
     
     // Auto reset
     if (m_autoReset && m_waitingThreads.empty()) m_predicate = false;

    return outcome;
}

void Event::wait()
{
    std::unique_lock lock{m_lock};

    if (!m_predicate)
    {
        m_waitingThreads.push_back(std::this_thread::get_id());

        m_event.wait(lock, [this] { return m_predicate; });

        updateWaitingThreads(m_waitingThreads);
    }
    
    // Auto reset
    if (m_autoReset && m_waitingThreads.empty()) m_predicate = false;
}

void Event::notify()
{
    setEvent(&std::condition_variable::notify_one);
}

void Event::broadcast()
{
    setEvent(&std::condition_variable::notify_all);
}

[[maybe_unused]] void Event::reset()
{
    std::lock_guard lock{m_lock};
    m_predicate = false;
}