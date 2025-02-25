/*
* Author: Damir Ljubic
* email: damirlj@yahoo.com
* @2025
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

namespace utils::mpsc
{
    template <std::size_t N>
    constexpr bool is_power_of_2 = (N > 0) && (N & (N-1)) == 0;

    /**
     * @brief Multiple-Producers Single-Consumer bounded queue
     * This (single consumer) relaxes the requirements on the interface of this thread-safe queue
     * implemented in the lock-free manner
     *
     * Designed to be used with Active Object concurrent pattern
    */
    template <typename T, std::size_t N>
    requires is_power_of_2<N>
    class queue final
    {

        static constexpr auto MASK = N - 1;
        
        inline std::size_t inc(std::size_t val)
        {
            return (val + 1) & MASK;
        }

        public:

            using value_type = std::remove_cvref_t<T>;
            
            auto try_pop() -> std::optional<value_type>
            {
                return pop([this]() { return not is_empty(); });
            }

            auto pop_wait(const std::atomic_flag& stop) -> std::optional<value_type>
            {
                return pop([&stop, this]() 
                    {
                        while (is_empty()) // wait until is non-empty, or stop is signaled
                        {
                            if (stop.test(std::memory_order_relaxed)) return false;
                            std::this_thread::yield();
                        }

                        return true;
                    });
            }

            auto pop_wait_for(const std::atomic_flag& stop, std::chrono::milliseconds timeout) -> std::optional<value_type>
            {
                return pop([&stop, timeout, this]() 
                    {
                        using namespace std::chrono;

                        auto start = steady_clock::now();

                        while (is_empty()) // wait until is non-empty, stop is signaled, or timeout expired
                        {
                            if (stop.test(std::memory_order_relaxed)) return false;
                            if (duration_cast<milliseconds>(steady_clock::now() - start) > timeout) return false;
                            
                            std::this_thread::yield();
                        }

                        return true;
                    });
            }

            /**
             * This can be invoked by the multiple producers - running on different 
             * thread contexts 
            */
                       
            template <typename U>
            requires std::convertible_to<U, value_type>
            void push(U&& u) noexcept (std::is_nothrow_constructible_v<U>)
            {
                auto tail = tail_.load(std::memory_order_relaxed); // expected value - otherwise, another producer modifies it
                while (is_full() || not tail_.compare_exchange_weak(tail, inc(tail), std::memory_order_acq_rel, std::memory_order_relaxed));
             
                data_[tail] = std::forward<U>(u);
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
                    if (not is_full(tail) && tail_.compare_exchange_weak(tail, inc(tail), std::memory_order_acq_rel, std::memory_order_relaxed))
                    {
                        data_[tail] = std::forward<U>(u);
                        break;
                    }

                    if (duration_cast<milliseconds>(steady_clock::now() - start) > timeout) return false;
                    std::this_thread::yield();
                }
                
                return true;
            }


        private:

            inline bool is_full(std::size_t tail) const
            {
                const auto full = [tail, this] 
                { 
                    return ((tail + 1) & MASK) == head_.load(std::memory_order_acquire); 
                };

                return full();    
            }
            
            inline bool is_full() const 
            {
                const auto tail = tail_.load(std::memory_order_relaxed);
                return is_full(tail);
            }

            inline bool is_empty(size_t head) const 
            {
                const auto empty = [head, this] 
                {
                    return head == tail_.load(std::memory_order_acquire); // maintain by the producers
                };

                return empty();
            }

            inline bool is_empty() const 
            {
                const auto head = head_.load(std::memory_order_relaxed); // maintain by the single consumer
                return is_empty(head);
            }

            
        private:

            template <typename Func, typename...Args>
            requires std::invocable<Func, Args...> && std::is_same_v<bool, std::invoke_result_t<Func, Args...>>
            inline std::optional<value_type> pop(Func&& func, Args&&...args) noexcept (std::is_nothrow_move_constructible_v<value_type>)
            {
                if (not std::invoke(std::forward<Func>(func), std::forward<Args>(args)...)) return {};

                const auto head = head_.load(std::memory_order_relaxed);
                auto data = std::optional<value_type>(std::move(data_[head]));
                
                head_.store((head + 1) & MASK, std::memory_order_release);
                
                return data;
            }

                       
        private:
            
            alignas(64) std::atomic<std::size_t> head_ {0};
            alignas(64) std::atomic<std::size_t> tail_ {0};

            alignas(64) std::array<T, N> data_;
    };
}




// Unit-test

// https://en.cppreference.com/w/cpp/error/exception_ptr
void handle_eptr(std::exception_ptr eptr)
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
requires utils::mpsc::is_power_of_2<N>
void producer(std::shared_ptr<utils::mpsc::queue<T, N>> queue) 
{
    using namespace std::chrono_literals;

    const auto tid = std::this_thread::get_id();
    oss(__func__, ": tid= ", tid);
    queue->push([tid]{oss(" <consumer>: job= ", tid);});

    std::this_thread::sleep_for(1ms);
}

template <typename T, std::size_t N>
requires utils::mpsc::is_power_of_2<N>
void consumer(std::shared_ptr<utils::mpsc::queue<T, N>> queue, const std::atomic_flag& stop) 
{
    /*
     * Simulates the AOT: draining the queue, providing the single thread-context for
     * executing jobs in order of their reception: FIFO
     * 
     * stop simulates the std::jthread stop token
     */
    for(; ;)
    {
        std::exception_ptr e;
        try
        {
            auto job = queue->pop_wait(stop);
            if (job.has_value()) std::invoke(*job);
            else if (stop.test(std::memory_order_relaxed)) break;
            
        }catch(...)
        {
            e = std::current_exception();
        
        }

        handle_eptr(e);
    }
}


int main()
{
    std::atomic_flag stop {false};

    using job_queue = utils::mpsc::queue<std::function<void()>, 8>; // set the low queue depth - less than producer threads that concurently access it: for rigid test
    auto q = std::make_shared<job_queue>();
    assert(q->try_pop() == std::nullopt);

    // Single-Consumer thread
    std::thread t_consumer([q, &stop]{consumer(q, stop);});

    // Multiple-Producers threads
    constexpr int N = 10;
    std::vector<std::thread> t_producers;
    t_producers.reserve(N);
    for (int i = 0; i < N; ++i)
    {
        t_producers.emplace_back(std::thread([q] { producer(q); }));       
    }
    for (auto& thread : t_producers) thread.join();

    // Signaling stop
    // One can reimplement this - using the stop token: https://en.cppreference.com/w/cpp/thread/jthread
    // @note - On the other hand: this is internally implemented with std::condition_variable/std::mutex
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(2s);
    stop.test_and_set(std::memory_order_relaxed);

    t_consumer.join();

    return 0;
}
