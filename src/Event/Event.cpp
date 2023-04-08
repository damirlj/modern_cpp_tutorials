//
// <author> damirlj@yahoo.com
// Copyright (c) 2023. All rights reserved!
//

#include "Event.h"

using namespace utils;


Event::Event(bool autoReset) noexcept
    : m_autoReset(autoReset)
{}

Event::~Event() = default;

Event::event_wait_t Event::wait_for(std::chrono::milliseconds timeout)
{
    std::unique_lock lock{m_lock};

    const bool signaled = m_event.wait_for(lock, timeout, [this] { return m_predicate; });
    const auto outcome = signaled ? event_wait_t::signaled : event_wait_t::timeout;

    if (m_autoReset) m_predicate = false;

    return outcome;
}

void Event::wait()
{
    std::unique_lock lock{m_lock};

    m_event.wait(lock, [this] { return m_predicate; });

    if (m_autoReset) m_predicate = false;
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
    std::scoped_lock lock{m_lock};
    m_predicate = false;
}
