#include <mutex>
#include <condition_variable>
#include <concepts>
#include <functional>
#include <chrono>


/**
* @brief Implementation of the Monitor Object design pattern.
* This will be used to monitor any client's method that requires inter-thread
* synchronization - by accessing/modifying the client's internal state from
* the different thread contexts.
*/
template <typename Lock = std::mutex, typename Condition = std::condition_variable>
class Monitor final
{
    using lock_t = Lock;
    using condition_t = Condition;

  public:

    [[nodiscard]] std::unique_lock<lock_t> getLock() const
    {
        return std::unique_lock<lock_t> {lock_};
    }

    // Inspired by: https://www.modernescpp.com/index.php/thread-safe-queue-two-serious-errors/
    template <bool broadcast>
    [[nodiscard]] auto getLockAndNotifyWhenDone() const
    {
        struct UnLockWithNotify final
        {
            explicit UnLockWithNotify(const Monitor& monitor) noexcept: monitor_{monitor}
            {}
            // Mutex interface
            void lock() { monitor_.lock_.lock();}
            void unlock() {
                if constexpr (not broadcast) monitor_.condition_.notify_one();
                else monitor_.condition_.notify_all();
                monitor_.lock_.unlock();}
            private:
                const Monitor& monitor_;

        };
        
        return UnLockWithNotify{*this};
    }

    /**
     * Wait infinite on the predicate to be signaled, and return the locking 
     * object to the client
    */
    template <typename Predicate, typename... Args>
    requires std::is_same_v<bool, std::invoke_result_t<Predicate, Args...>>
    auto wait(
        Predicate&& predicate,
        Args&&... args) const
    {
        std::unique_lock lock{lock_};

        condition_.wait(
            lock,
            [ p = std::forward<Predicate>(predicate), ...args = std::forward<Args>(args)]() mutable
            {
                return std::invoke(p, std::forward<Args>(args)...);
            });

        return lock;
    }

    /**
     *
     * Wait on the predicate to be signaled, or timeout being expired: whatever comes first
     * and return the locking object to the client
    */
    template <typename Predicate, typename... Args>
    requires std::is_same_v<bool, std::invoke_result_t<Predicate, Args...>>
    std::tuple<bool, std::unique_lock<lock_t>>
    wait_for(std::chrono::milliseconds timeout,
        Predicate&& predicate,
        Args&&... args) const
    {
        std::unique_lock lock{lock_};

        bool result = condition_.wait_for(
            lock,
            timeout,
            [ p = std::forward<Predicate>(predicate), ...args = std::forward<Args>(args)]() mutable
            {
                return std::invoke(p, std::forward<Args>(args)...);
            });

        return {result, std::move(lock)};
    }

    template <typename Func, typename...Args>
    auto notify_one(Func&& func, Args&&...args) const
    {
        return notify<false>(std::forward<Func>(func), std::forward<Args>(args)...);
    }

    template <typename Func, typename...Args>
    auto notify_all(Func&& func, Args&&...args) const
    {
        return notify<true>(std::forward<Func>(func), std::forward<Args>(args)...);
    }

  private:

      /**
       * Execute the given function (callable) under the locking 
       * protection, and notify completion
      */
      template <bool broadcast, typename Func, typename...Args>
      auto notify(Func&& func, Args&&...args) const
      {
        auto lockWithNotify = getLockAndNotifyWhenDone<broadcast>();
        std::unique_lock lock {lockWithNotify};
        
        if constexpr (not std::is_void_v<std::invoke_result_t<Func, Args...>>)
        {
            return std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);

        }
        
        std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);
      }


  private:
    mutable lock_t lock_;
    mutable condition_t condition_;
};
