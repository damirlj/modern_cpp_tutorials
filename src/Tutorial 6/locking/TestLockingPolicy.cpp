/*
 * TestLockingPolicy.cpp
 *
 *  Created on: Nov 13, 2021
 *  Author: <a href="mailto:damirlj@yahoo.com">Damir Ljubic</a>
 */

#include <string>
#include <vector>
#include <thread>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <atomic>

// Different locking policies

#include "NonLock.h"
#include "ObjecLevelLock.h"
#include "ClassLevelLock.h"


#include "TestLockingPolicy.h"


/**
 * A basic example of using
 * single policy which is plugged-in into the host class
 *
 * @tparam LockingPolicy
 */
template <class LockingPolicy>
class Message : private LockingPolicy
{
    public:

        Message() = default;
        explicit Message(std::string msg) noexcept : m_msg(std::move(msg))
        {}

        friend std::ostream& operator << (std::ostream& out, const Message& msg)
        {
            {
                typename LockingPolicy::Lock lock {msg};
                out << msg.get() << '\n';
            }

            return out;
        }

        void set(const std::string& msg)
        {
            typename LockingPolicy::Lock lock {*this};
            m_msg = msg;
        }

        std::string get() const { return m_msg;}

    private:
        std::string m_msg;
};



namespace
{
    /**
     * Class-level locking.
     *
     * The all instances of the class shared the same
     * locking mechanism (static mutex)
     *
     */
    [[maybe_unused]]
    void testClassLevelLock()
    {
       using namespace std;
       using namespace utils::locking;
       using namespace std::chrono_literals;


       using message_t = Message<ClassLevelLock>;
       //using message_t = Message<NonLock>; // compare when locking is disabled (single-thread environment)

       vector<thread> threads;
       constexpr size_t numOfThreads = 10;

       std::atomic<bool> done = false;

       // Launch background threads

       for (size_t i = 0; i < numOfThreads; ++i)
       {
           threads.emplace_back(
                    [&done] // thread function
                    {
                       for (;;)
                       {
                           if (done) break;
                           for (auto j = 0; j < 5; ++j )
                           {
                               message_t msg {std::string(100, '=')};
                               std::cout << msg;
                           }

                           std::this_thread::sleep_for(100ms);
                       }
                    }
           );
       }

       // main thread

       for (auto j = 0; j < 10; ++j)
       {
           message_t msg {std::string(100, '8')};
           std::cout << msg;
       }

       std::this_thread::sleep_for(1s);
       done = true;

       std::for_each(threads.begin(), threads.end(), [](auto& thread){
           thread.join();
       });

       std::cout << "\n\nAll threads joined...\n";
    }
}


namespace
{
    /**
     * Object-level locking.
     *
     * Every instance of the class has its own locking mechanism
     * (per-object mutex)
     */
    [[maybe_unused]]
    void testObjectLevelLock()
    {
       using namespace std;
       using namespace utils::locking;
       using namespace std::chrono_literals;

       /*
        * Share the object between threads
        */
       using message_t = Message<ObjectLevelLock>;
       //using message_t = Message<NonLock>; // compare when locking is disabled (single-thread environment)
       auto msg = std::make_shared<message_t>();

       vector<thread> threads;
       constexpr size_t numOfThreads = 10;

       std::atomic<bool> done = false;

       // Launch background threads

       for (size_t i = 0; i < numOfThreads; ++i)
       {
           threads.emplace_back(
                    [&done, &msg]() mutable // thread function
                    {
                       for (;;)
                       {
                           if (done) break;
                           for (auto j = 0; j < 5; ++j )
                           {
                               msg->set(std::string(100, '='));
                               std::cout << *msg;
                           }

                           std::this_thread::sleep_for(100ms);
                       }
                    }
           );
       }

       // main thread

       for (auto j = 0; j < 10; ++j)
       {
           msg->set(std::string(100, '8'));
           std::cout << *msg;
       }

       std::this_thread::sleep_for(1s);
       done = true;

       std::for_each(threads.begin(), threads.end(), [](auto& thread){
           thread.join();
       });

       std::cout << "\n\nAll threads joined...\n";
    }
}


namespace utils::locking
{
    /*
     * test in your main.cpp
     *
     * int main()
     * {
     *    return utils::locking::testLockingPolicy();
     * }
     */
    int testLockingPolicy()
    {
        testClassLevelLock();
        //testObjectLevelLock();

        return 0;
    }
}
