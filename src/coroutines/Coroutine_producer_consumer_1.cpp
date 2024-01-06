#include <iostream>
#include <vector>
#include <coroutine>
#include <chrono>
#include <thread>
#include <utility>
#include <functional>
#include <memory>
#include <algorithm>
#include <iterator>
#include <atomic>


//https://godbolt.org/z/zc1dh8f1z

#define FUNC() std::cout << __func__ << '\n'


namespace details
{
    template <typename InputIterator>
    void printIterable(InputIterator first, InputIterator last)
    {
        using value_type = std::decay_t<decltype(*first)>;
        std::cout << '[';
        if constexpr (std::is_same_v<std::uint8_t, value_type>) {
            std::copy(first, std::prev(last), std::ostream_iterator<std::uint16_t>(std::cout, ", "));
            std::cout << static_cast<std::uint16_t>(*std::prev(last)) << "]\n";
        }
        else
        {
            std::copy(first, std::prev(last), std::ostream_iterator<value_type>(std::cout, ", "));
            std::cout << *std::prev(last) << "]\n";
        }
    }

    template <typename Container>
    void printContainer(const Container& container)
    {
        printIterable(std::cbegin(container), std::cend(container));
    }
}


namespace utils 
{
     template <typename T, typename U = std::remove_cvref_t<T>>
     void exchange(std::atomic<U>& val, const U& expected, T&& newVal, std::memory_order rxOrder, std::memory_order txOrder) 
     {
        while (const auto rv = val.load(rxOrder) != expected);
        val.store(std::forward<T>(newVal), txOrder);// this can be iterrupted by another thread – when this is important, use CAS idiom
     }
}

class [[nodiscard]] AudioDataResult final
{
    public:
        class promise_type;
        using handle_type = std::coroutine_handle<promise_type>;

                
        // Predefined interface that has to be specify in order to implement
        // coroutine's state-machine transitions
        class promise_type 
        {
            
            public:
                
                using value_type = std::vector<int>;

                AudioDataResult get_return_object() 
                {
                    return AudioDataResult{handle_type::from_promise(*this)};
                }
                std::suspend_never initial_suspend() noexcept { return {}; }
                std::suspend_always final_suspend() noexcept { return {}; }
                void return_void() {}
                void unhandled_exception() 
                {
                    std::rethrow_exception(std::current_exception());
                }

                // Generates the value and suspend the "producer"
                template <typename Data>
                requires std::convertible_to<std::decay_t<Data>, value_type>
                std::suspend_always yield_value(Data&& value) 
                {
                    data_ = std::forward<Data>(value);
                    data_ready_.store(true, std::memory_order_release);
                    return {};
                }

                // Awaiter interface: for consumer waiting on data being ready
                struct AudioDataAwaiter 
                {
                    explicit AudioDataAwaiter(promise_type& promise) noexcept: promise_(promise) {}

                    bool await_ready() const { return promise_.data_ready_.load(std::memory_order_relaxed);}
                    
                    void await_suspend(handle_type)
                    {
                        static constexpr bool expected = true;
                        static constexpr bool replace_with = false;
                        
                        utils::exchange(promise_.data_ready_, 
                            expected, 
                            replace_with,
                            std::memory_order_acquire, 
                            std::memory_order_relaxed);
                    }
                    // Move assignment at client invocation side: const auto data = co_await audioDataResult;
                    // This requires that coroutine's result type provides the co_await unary operator
                    value_type&& await_resume()
                    {
                        return std::move(promise_.data_);
                    }

                    private: 
                        promise_type& promise_;
                };//Awaiter interface

        
            private:
                value_type data_;
                std::atomic<bool> data_ready_;
        }; //promise_type interface

        
        auto operator co_await() noexcept   
        {
            return promise_type::AudioDataAwaiter{handle_.promise()};
        }

        // Make the result type move-only, due to ownership over the handle
        AudioDataResult(const AudioDataResult&) = delete;
        AudioDataResult& operator=(const AudioDataResult&) = delete;

        AudioDataResult(AudioDataResult&& other) noexcept: handle_(std::exchange(other.handle_, nullptr)){}
        AudioDataResult& operator=(AudioDataResult&& other) noexcept 
        {
            using namespace std;
            AudioDataResult tmp = std::move(other);
            swap(*this, tmp);
            return *this;
        }

        // d-tor: RAII
        ~AudioDataResult() { if (handle_) {FUNC(); handle_.destroy();}}

        // For resuming the producer - at the point when the data are consumed
        void resume() {if (not handle_.done()) { FUNC(); handle_.resume();}}
    
    private:
        AudioDataResult(handle_type handle) noexcept : handle_(handle) {}

    private:
        handle_type handle_ = nullptr;
};


using data_type = std::vector<int>;
AudioDataResult producer(const data_type& data) 
{
    for (std::size_t i = 0; i < 5; ++i) {
        FUNC();
        co_yield data;
    }
    co_yield data_type{}; // exit criteria

    co_return;
}

AudioDataResult consumer(AudioDataResult& audioDataResult) 
{
    for(;;)
    {
        FUNC();
        const auto data = co_await audioDataResult;
        if (data.empty()) {std::cout << "No data - exit!\n"; break;}
        std::cout << "Data received:";
        details::printContainer(data);

        audioDataResult.resume(); // resume producer
    }
    co_return;
}

int main() 
{
    {
        static const data_type data = {1, 2, 3, 4};
        auto audioDataProducer = producer(data);

        std::thread t_consumer ([&]{auto audioRecorded = consumer(audioDataProducer);});
        t_consumer.join();
        
    }

    std::cout << "bye-bye!\n";
    return 0;
}