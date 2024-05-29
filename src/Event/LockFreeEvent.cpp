// Author: Damir Ljubic
// mail: damirlj@yahoo.com


// Linux platform
#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>

// Std library
#include <atomic>
#include <chrono>
#include <thread>
#include <functional>
#include <concepts>
#include <time.h>

#include <iostream>


// Example: https://godbolt.org/z/f8dj4zbjv

namespace details 
{

    template <typename Clock, typename Duration>
    constexpr auto remainedTime(std::chrono::time_point<Clock, Duration> t1, 
                                std::chrono::time_point<Clock, Duration> t2) noexcept 
    {
        using namespace std::chrono;
        constexpr auto GIGA = static_cast<long>(1e+9);
        
        const auto diff = duration_cast<nanoseconds>(t2 -t1).count();
        //std::cout << diff << '\n';
        struct timespec ts {.tv_sec = static_cast<time_t>(diff/GIGA), .tv_nsec = diff % GIGA};
        return ts;
    }

    /**
    * Event signalization for the single producer -single consumer scenario. <br>
    * Internally, it relies on the native (linux) futex mechanism (similar to std::mutex/std::condition_variable), <br>
    * to suspend the calling thread, rather than to employ the busy loop with sleep/yield <br>
    * https://www.man7.org/linux/man-pages/man2/futex.2.html
    */
    class Event final
    {
    
        public:
            
            enum class State { waiting = 1, signaled};

            /**
            * c-tor
            * @param autoReset Indication whether to reset the event after 
            * being received: for the next succesive wait
            */
            explicit Event(bool autoReset) noexcept : autoReset_(autoReset) {}
            ~Event() = default;

            // copy operations forbidden
            Event(const Event& ) = delete;
            Event& operator = (const Event& ) = delete;
            
            // move semantic allowed
            Event(Event&& ) = default;
            Event& operator = (Event&& ) = default;

            /**
            * Signal the event
            */
            void notify() 
            {
                state_.store(State::signaled, std::memory_order_release);
                syscall(SYS_futex, &state_, FUTEX_WAKE_PRIVATE, State::signaled, nullptr, nullptr, 0);
            }

            /**
            * Wait until the event is signaled, or timeout expired, whatever 
            * comes first.
            *
            * @param time The timeout to wait for, in milliseconds
            * @return True if the event is signaled. False - in case that 
                      timeout is expired before.
            */
           [[nodiscard]] bool waitFor(std::chrono::milliseconds timeout) 
           {
                using enum State;
                using namespace std::chrono;
                using clock = steady_clock;
                
                const auto start = clock::now();
                const auto end = start + timeout;

                auto expected = signaled;
                while(not state_.compare_exchange_weak(expected, 
                                                       autoReset_? waiting : signaled, 
                                                       std::memory_order_acq_rel)) 
                {
                    expected = signaled;
                    
                    if (clock::now() >= end) return false; // timeout expired
                    
                    const auto ts = remainedTime(clock::now(), end);
                    // Suspend the calling thread until the value is equal to expected, or timeout is expired
                    syscall(SYS_futex, &state_, FUTEX_WAIT_PRIVATE, State::waiting, &ts, nullptr, 0);
                }

                return true;
            }

            [[nodiscard]] bool waitUntil(std::chrono::time_point<std::chrono::steady_clock, std::chrono::milliseconds> till) 
            {
                using namespace std::chrono;
                if (till <= steady_clock::now()) [[unlikely]] return false;
                return waitFor(duration_cast<milliseconds>(till - steady_clock::now()));
            }

            /**
            *   Wait infinitelly, on the event being signaled
            */
            
            void wait() 
            {
                using namespace std::chrono;
                using enum State;
        
                auto expected = signaled;
                while(not state_.compare_exchange_weak(expected, 
                                                       autoReset_ ? waiting : signaled, 
                                                       std::memory_order::acq_rel)) 
                {
                    expected = signaled;
                    syscall(SYS_futex, &state_, FUTEX_WAIT_PRIVATE, State::waiting, nullptr, nullptr, 0);
                }
            }

        
            template <typename Func, typename ...Args>
            requires std::is_invocable_v<Func, Args...>
            auto waitAndThen(Func&& func, Args&&...args) 
            {
                wait();
                
                if constexpr (not std::is_void_v<std::invoke_result_t<Func, Args...>>) 
                {
                    return std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);
                }
                std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);
            }

        private:
            bool autoReset_;
            std::atomic<State> state_ {State::waiting};
    }; // class Event
} // namespace details


int main()
{
    using namespace details;
    Event event {true};

    
    std::thread producer {[&event]
    {
        using namespace std::chrono_literals;
        constexpr auto timeout = 1s;
        std::this_thread::sleep_for(timeout);
        std::cout << "Signal event, after: " << timeout << '\n';
        
        event.notify();
    }};

    std::thread consumer([&event]
    {
        using namespace std::chrono;
        using namespace std::chrono_literals;

        auto start = steady_clock::now();
        #define TEST_WAIT (0)
        #if (TEST_WAIT == 1)
            event.wait();
        #else
            constexpr auto timeout = 700ms;
            // constexpr auto timeout = 1150ms;
            const auto rv = event.waitFor(timeout);
            std::cout << "waitFor(): " << std::boolalpha << rv << '\n';
        #endif    
        std::cout << "Waited for: " << duration_cast<milliseconds>(steady_clock::now() - start) << '\n';
        
    });

    consumer.join();
    producer.join();

    return 0;
}