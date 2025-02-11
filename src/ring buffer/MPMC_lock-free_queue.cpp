/**
* @author: Damir Ljubic
* @email: damirlj@yahoo.com

* All rights reserved!
*/

#include <atomic>
#include <array>
#include <vector>
#include <optional>
#include <concepts>
#include <functional>
#include <thread>
#include <chrono>

// Testing
#include <iostream>
#include <memory>
#include <syncstream>
#include <cassert>


// <Compile Explorer> https://godbolt.org/z/KEEvavfxd
namespace utils::mpmc
{
    template <std::size_t N>
    constexpr bool is_power_of_2 = (N & (N-1)) == 0;

    /**
     * @brief Multiple-Producers Multiple-Consumers queue
     * 
    */
    template <typename T, std::size_t N>
    requires is_power_of_2<N>
    class queue final
    {

        public:

            using value_type = std::remove_cvref_t<T>;

            std::optional<value_type> pop(const std::atomic_flag& stop) noexcept (std::is_nothrow_move_constructible_v<value_type>)
            {
                for (;;)
                {
                    
                    auto head = head_.load(std::memory_order_relaxed); 
                    if (not is_empty() && head_.compare_exchange_strong(head, (head + 1) & (N - 1), std::memory_order_release))
                    {
                        auto data = std::optional<value_type>(std::move(data_[head]));
                        return data;
                    }

                    if (stop.test(std::memory_order_relaxed)) break;
                    std::this_thread::yield();
                }
                
                return {};
                
            }

            std::optional<value_type> pop_wait_for(const std::atomic_flag& stop, std::chrono::milliseconds timeout) noexcept (std::is_nothrow_move_constructible_v<value_type>)
            {
                using namespace std::chrono;

                auto start = steady_clock::now();

                for (;;)
                {
                    
                    auto head = head_.load(std::memory_order_relaxed); 
                    if (not is_empty() && head_.compare_exchange_strong(head, (head + 1) & (N - 1), std::memory_order_release))
                    {
                        auto data = std::optional<value_type>(std::move(data_[head]));
                        return data;
                    }

                    if (stop.test(std::memory_order_relaxed)) break;
                    if (duration_cast<milliseconds>(steady_clock::now() - start) > timeout) break;

                    std::this_thread::yield();
                }
                
                return {};
            }

            
                       
            template <typename U>
            requires std::convertible_to<U, value_type>
            bool push(U&& u) noexcept (std::is_nothrow_constructible_v<U>)
            {
                auto tail = tail_.load(std::memory_order_relaxed); // expected value - otherwise, another producer modifies it
                while (is_full() || not tail_.compare_exchange_weak(tail, (tail + 1) & (N - 1), std::memory_order_release))
                {
                    std::this_thread::yield();
                }

                data_[tail] = std::forward<U>(u);
                
                return true;
            }

            template <typename U>
            requires std::convertible_to<U, value_type>
            bool push_wait_for(U&& u, std::chrono::milliseconds timeout) noexcept (std::is_nothrow_constructible_v<U>)
            {
                using namespace std::chrono;

                auto start = steady_clock::now();

                for (;;)
                {
                    auto tail = tail_.load(std::memory_order_relaxed);
                    if (not is_full() && tail_.compare_exchange_strong(tail, (tail + 1) & (N - 1), std::memory_order_release))
                    {
                        data_[tail] = std::forward<U>(u);
                        return true;   
                    }

                    if (duration_cast<milliseconds>(steady_clock::now() - start) > timeout) break;
                    std::this_thread::yield();  
                }

                return false;
                
            }


        private:

            inline bool is_full() const 
            {
                const auto full = [=, this] 
                { 
                    const auto tail = tail_.load(std::memory_order_relaxed);
                    return ((tail + 1) & (N - 1)) == head_.load(std::memory_order_acquire); 
                };

                return full();
            }

            inline bool is_empty(size_t head) const 
            {
                const auto empty = [head, this] 
                {
                    return head == tail_.load(std::memory_order_acquire); 
                };

                return empty();
            }

            inline bool is_empty() const 
            {
                const auto head = head_.load(std::memory_order_relaxed);
                return is_empty(head);
            }

        
        private:
            alignas(64) std::array<value_type, N> data_;
            alignas(64) std::atomic<std::size_t> head_ {0};
            alignas(64) std::atomic<std::size_t> tail_ {0};
    };
}


// Unit-test

// https://en.cppreference.com/w/cpp/error/exception_ptr
inline void handle_eptr(std::exception_ptr eptr)
{
    try
    {
        if (eptr) std::rethrow_exception(eptr);
    }
    catch(const std::exception& e)
    {
        std::cout << "Caught exception: '" << e.what() << "'\n";
    }
}

template <typename ...Args>
void oss(Args&&...args)
{
    std::osyncstream out {std::cout};
    ((out << std::forward<Args>(args)), ...);
    out << '\n';
}

template <typename T, std::size_t N>
requires utils::mpmc::is_power_of_2<N>
void producer(std::shared_ptr<utils::mpmc::queue<T, N>> queue) {
    const auto tid = std::this_thread::get_id();
    oss(__func__, ": tid= ", tid);
    queue->push([tid]{oss(" <consumer>: job= ", tid);});
}

template <typename T, std::size_t N>
requires utils::mpmc::is_power_of_2<N>
void consumer(std::shared_ptr<utils::mpmc::queue<T, N>> queue, const std::atomic_flag& stop) 
{
    using namespace std::chrono;
    
    for (; ;)
    {
        std::exception_ptr e;

        try
        {
            auto job = queue->pop(stop);
            if (job.has_value()) 
            {
                oss(__func__, ": tid= ", std::this_thread::get_id());
                std::invoke(*job);
            }
            else if (stop.test(std::memory_order_relaxed)) break;

            std::this_thread::sleep_for(1ms);
            std::this_thread::yield();
        }
        catch(...)
        {
            e = std::current_exception();
        }

        handle_eptr(e);
    }
}


int main()
{
    std::atomic_flag stop {false};

    using job_queue = utils::mpmc::queue<std::function<void()>, 8>; // set the low queue depth - less than producer threads that concurently access it: for rigid test
    auto q = std::make_shared<job_queue>();
  

    // Multiple-Consumers 
    constexpr int Consumers = 3;
    std::vector<std::thread> t_consumers;
    t_consumers.reserve(Consumers);
    for (int i = 0; i < Consumers; ++i) {
        t_consumers.emplace_back([q, &stop]{consumer(q, stop);});
    }

    // Multiple-Producers
    constexpr int Producers = 10;
    std::vector<std::thread> t_producers;
    t_producers.reserve(Producers);
    for (int i = 0; i < Producers; ++i)
    {
        t_producers.emplace_back([q] { producer(q); });
    }
    for (auto& producer : t_producers) producer.join();

    // Signaling stop
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(2s);
    stop.test_and_set(std::memory_order_relaxed);

    for (auto& consumers : t_consumers) consumers.join();

    return 0;
}

