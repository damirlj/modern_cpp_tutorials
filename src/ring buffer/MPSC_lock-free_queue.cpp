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


// <Compiler Explorer>: https://godbolt.org/z/8T4vqrWoE

namespace utils::mpsc
{
    template <std::size_t N>
    constexpr bool is_power_of_2 = (N & (N-1)) == 0;

    /**
     * @brief Multiple-Producers Single-Consumer queue
     * This relax the requirements on the interface of this thread-safe queue
     * implemented in the lock-free manner
     *
     * Designed to be used with Active Object concurrent pattern
    */
    template <typename T, std::size_t N>
    requires is_power_of_2<N>
    class queue final
    {

        public:

            using value_type = std::remove_cvref_t<T>;
            
            auto try_pop() -> std::optional<value_type>
            {
                return pop([this](std::size_t head) { return not is_empty(head); });
            }

            auto pop_wait(const std::atomic_flag& stop) -> std::optional<value_type>
            {
                return pop([&stop, this](std::size_t head) 
                    {
                        while (is_empty(head)) // wait until is non-empty, or stop is signaled
                        {
                            if (stop.test(std::memory_order_relaxed)) return false;
                            std::this_thread::yield();
                        }
                        return true;
                    });
            }

            auto pop_wait_for(const std::atomic_flag& stop, std::chrono::milliseconds timeout) -> std::optional<value_type>
            {
                return pop([&stop, timeout, this](std::size_t head) 
                    {
                        using namespace std::chrono;
                        auto start = steady_clock::now();

                        while (is_empty(head)) // wait until is non-empty, stop is signaled, or timeout expired
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
            bool push(U&& u) noexcept (std::is_nothrow_constructible_v<U>)
            {
                auto tail = tail_.load(std::memory_order_relaxed); // expected value - otherwise, another producer modifies it
                while(is_full() || not tail_.compare_exchange_weak(tail, (tail + 1) & (N - 1), std::memory_order_release));
                
                data_[tail] = std::forward<U>(u);

                return true;
            }

            template <typename U>
            requires std::convertible_to<U, value_type>
            bool push_wait_for(U&& u, std::chrono::milliseconds timeout) noexcept (std::is_nothrow_constructible_v<U>)
            {
                using namespace std::chrono;

                auto start = steady_clock::now();
                auto tail = tail_.load(std::memory_order_relaxed);
                while (is_full() || not tail_.compare_exchange_weak(tail, (tail + 1) & (N - 1), std::memory_order_release))
                {
                    if (duration_cast<milliseconds>(steady_clock::now() - start) > timeout) return false;    
                }
                
                data_[tail] = std::forward<U>(u);

                return true;
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
                    return head == tail_.load(std::memory_order_acquire); // maintain by the producers
                };

                return empty();
            }

            inline bool is_empty() const 
            {
                const auto head = head_.load(std::memory_order_relaxed); // maintain by the single consumer only
                return is_empty(head);
            }

            
        private:

            template <typename Func, typename...Args>
            requires std::invocable<Func, std::size_t, Args...> && std::is_same_v<bool, std::invoke_result_t<Func, std::size_t, Args...>>
            inline std::optional<value_type> pop(Func&& func, Args&&...args) noexcept (std::is_nothrow_move_constructible_v<value_type>)
            {
                const auto head = head_.load(std::memory_order_relaxed); 
                
                if (not std::invoke(std::forward<Func>(func), head, std::forward<Args>(args)...)) return {};

                auto data = std::optional<value_type>(std::move(data_[head]));
                // data_[head] = {}; // default-constructible

                head_.store((head + 1) & (N - 1), std::memory_order_release);
                
                return data;
            }

                       
        private:
            alignas(64) std::array<T, N> data_;
            alignas(64) std::atomic<std::size_t> head_ {0};
            alignas(64) std::atomic<std::size_t> tail_ {0};
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
void producer(std::shared_ptr<utils::mpsc::queue<T, N>> queue) {
    const auto tid = std::this_thread::get_id();
    oss(__func__, ": tid= ", tid);
    queue->push([tid]{oss(" <consumer>: job= ", tid);});
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

