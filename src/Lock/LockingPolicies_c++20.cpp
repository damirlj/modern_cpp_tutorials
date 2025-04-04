// Author: damir ljubic
// email: damirlj@yahoo.com
// All rights reserved.


#include <mutex>
#include <concepts>
#include <thread>
#include <iostream>
#include <array>
#include <chrono>

// https://godbolt.org/z/1bfcKfb83

template <typename Lock>
concept is_lockable = requires(std::remove_cvref_t<Lock>& lock) 
{
    lock.lock();
    lock.unlock();
};

template <typename Host, typename Lock>
requires is_lockable<Lock>
struct ObjectLock 
{
    private: // data
        mutable Lock lock_ {};

    private: // methods
        inline void lock() const { lock_.lock(); }
        inline void unlock() const { lock_.unlock(); }
    public: 
        /**
        * This is requirement on enclosing class - lock level, to have the same (name) inner type that 
        * implements the actual locking strategy
        */
        struct ScopeLock final 
        {
            explicit ScopeLock(const ObjectLock& obj) noexcept : obj_{obj} { obj_.lock(); }
            ~ScopeLock() { obj_.unlock(); }
            private:
                const ObjectLock& obj_;
        };
};

namespace locking 
{
    template <typename Lock>
    using scope_lock = std::lock_guard<Lock>;

    template <typename Host, typename Lock>
    requires is_lockable<Lock>
    struct ObjectLock 
    {
        private: // data
            mutable Lock lock_ {};

        public: 
            /**
             * Instead of having constraint on the inner (name) type, we 
             * can impose constraint on the behavioral aspect - that all lock-levels in our 
             * Locking policy have the same call-operator signature
            */
            [[nodiscard]] inline auto operator()() const
            {
                return scope_lock<Lock> {lock_};
            }
    };
}

template <typename Host , typename Lock>
requires is_lockable<Lock>
struct ClassLock 
{
   private: // data
    static inline Lock lock_{};
    private: //methods
    
        static void lock()   { lock_.lock(); }
        static void unlock() { lock_.unlock(); }
    public:
        struct ScopeLock final 
        {
            explicit ScopeLock(const ClassLock& ) noexcept { lock(); }
            ~ScopeLock() { unlock(); }
        };
};


namespace locking {
    
    template <typename Host , typename Lock>
    requires is_lockable<Lock>
    struct ClassLock 
    {
        private: // data
            static inline Lock lock_{};
        
        public:
            [[nodiscard]] inline auto operator()() const
            {
                return scope_lock<Lock> {lock_};
            } 
    };
}


template <typename Locker>
concept has_scope_lock = requires {
    typename Locker::ScopeLock;
};

template <template <class, class> class Locker, typename Lock = std::mutex>
struct Counter 
{
    static_assert(has_scope_lock<Locker<Counter, Lock>>, "Invalid locking policy");

  private: 
      int count_ = 0;
      using lock_type = Locker<Counter, Lock>;
      lock_type lock_ {};
  public:
    int operator()() 
    {
        typename lock_type::ScopeLock lock {lock_}; 
        std::cout << '[' << std::this_thread::get_id() << "] counter: " << ++count_ << '\n';
        return count_;
    }
    void reset() 
    {
        typename lock_type::ScopeLock lock {lock_};
        count_ = 0;
    }
};


namespace locking {
    template <template <class, class> class Locker, typename Lock = std::mutex>
    struct Counter 
    {
        
    private: 
        int count_ = 0;
        using lock_type = Locker<Counter, Lock>;
        lock_type lock_ {};
    public:
        int operator()() 
        {
            auto locker = lock_();
            std::cout << '[' << std::this_thread::get_id() << "] counter: " << ++count_ << '\n';
            return count_;
        }
        
};

}

template <typename Counter>
void test_objectLevelLock() 
{
    std::cout << __func__ << '\n';

    Counter counter;
    
    auto thread_func = [&counter](int N) mutable 
    {
        using namespace std::chrono_literals;

        for (int i = 0; i < N; ++i ) 
        {
            counter();  
            std::this_thread::sleep_for(10ms);
        }
    };

    std::array<std::jthread, 3> threads = 
        {
            std::jthread{thread_func, 100}, 
            std::jthread{thread_func, 50}, 
            std::jthread{thread_func, 30}
        };
}

template <typename Counter>
void test_classLevelLock() 
{
    std::cout << __func__ << '\n';
    
    
    auto thread_func = [](int N) 
    {
        using namespace std::chrono_literals;
        Counter counter;
        for (int i = 0; i < N; ++i ) 
        {
            counter();  
            std::this_thread::sleep_for(10ms);
        }
    };

    std::array<std::jthread, 3> threads = 
        {
            std::jthread{thread_func, 100}, 
            std::jthread{thread_func, 50}, 
            std::jthread{thread_func, 30}
        };
}

int main() 
{
    // Object-level lock
    test_objectLevelLock<locking::Counter<locking::ObjectLock>>();
    test_objectLevelLock<Counter<ObjectLock>>();

    // Class-level lock
    test_classLevelLock<Counter<ClassLock>>();
    test_classLevelLock<locking::Counter<locking::ClassLock>>();

    return 0;
}