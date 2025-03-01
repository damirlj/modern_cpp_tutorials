/*
 * AOThread.h
 * C++17 based solution
 *  Created on: Aug 22, 2021
 *      Author: <a href="mailto:damirlj@yahoo.com">Damir Ljubic</a>
 */

#ifndef DS_AOT_AOTHREAD_H_
#define DS_AOT_AOTHREAD_H_

#include <thread>
#include <future>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <memory>
#include <iostream>
#include <type_traits>

namespace utils::aot
{
    template <typename Thread,
    typename = std::enable_if_t<std::is_base_of_v<std::thread, Thread>>>
    struct ThreadDeleter
    {
        void operator()(Thread * ptr) const
        {
            using namespace std;

            if (ptr)
            {
                if (ptr->joinable()) ptr->join();
                cout << "\nLeaving thread gracefully...\n";
                delete ptr;

            }
        }
    };

    template <typename Thread>
    using thread_with_deleter_t = std::unique_ptr<Thread, ThreadDeleter<Thread>>;

    template <typename Thread, typename Func, typename...Args>
    auto make_thread(Func&& func, Args&&...args)
    {
        return thread_with_deleter_t<Thread>(new (std::nothrow) Thread{std::forward<Func>(func)
            , std::forward<Args>(args)...}
            , ThreadDeleter<Thread>{}
        );
    }

    /**
     * Type Erasure
     * For wrapping the only copy-constructible callable objects
     * into movable object, since std::packaged_task is move-only
     */
    class FunctionWrapper final
    {
        struct FunctionWrapperBase
        {
            virtual ~FunctionWrapperBase() = default;
            virtual void call() = 0;
        };

        template <typename Func>
        struct FunctionWrapperBaseImpl final : FunctionWrapperBase
        {
                FunctionWrapperBaseImpl(Func&& func):
                    m_func(std::move(func))
                {}
                ~FunctionWrapperBaseImpl() override = default;

                void call() override
                {
                    m_func();
                }

            private:
                Func m_func;
        };

        public:
            template <typename Func>
            FunctionWrapper(Func&& func):
                m_pFunctionWrapper(std::make_unique<FunctionWrapperBaseImpl<Func>>(std::forward(func)))
            {}

            FunctionWrapper() = default;
            ~FunctionWrapper() = default;

            // Move operations
            FunctionWrapper(FunctionWrapper&& other) noexcept : m_pFunctionWrapper(std::exchange(other.m_pFunctionWrapper, nullptr))
            {}

            FunctionWrapper& operator=(FunctionWrapper&& other) noexcept
            {
                using namespace std;
                FunctionWrapper tmp {std::move(other)};
                swap(*this, tmp);
                
                return *this;
            }

            // Copy-functions are forbidden

            FunctionWrapper(const FunctionWrapper&) = delete;
            FunctionWrapper& operator=(const FunctionWrapper&) = delete;

            void operator ()()
            {
                m_pFunctionWrapper->call();
            }

        private:
            std::unique_ptr<FunctionWrapperBase> m_pFunctionWrapper = nullptr;

    };


    /**
     * AOT thread design pattern - heterogeneous version
     * More flexible version which allows handling the heterogeneous jobs
     * (of a different signature - return type)
     */
    class AOThread final
    {
        public:


            AOThread() = default;
            ~AOThread()
            {
                /*
                 * It's important to declare the mutex/conditional_variable first, then the job queue,
                 * and finally the thread itself to ensure the proper destruction in 
                 * reversable order.
                 * Otherwise, we would need to force joining the thread explicitly, to prevent
                 * any undesirable effects (crashes)
                 */
                stop();
            }

            AOThread(const AOThread& ) = delete;
            AOThread& operator = (const AOThread&) = delete;

            template <typename Func>
            auto enqueue(Func&& func)
            {
                using namespace std;

                using result_t = invoke_result_t<Func>;
                using task_t = packaged_task<result_t()>;

                auto task = task_t{std::forward<Func>(func)};
                auto result = task.get_future();

                {
                    lock_guard<std::mutex> lock {m_lock};
                    // Call explicitly the task - so that the std::future is properly set
                    m_jobs.push(FunctionWrapper([task = std::move(task)]() mutable { task(); }));
                }

                m_condition.notify_one();

                return result;
            }

            template <typename Func, typename...Args>
            auto emplace_enqueue(Func&& func, Args&&...args)
            {
                using namespace std;

                using result_t =invoke_result_t<Func&&, Args&&...>;
                using task_t = packaged_task<result_t()>;


                // Bind the data - additional arguments with the job: to satisfy the task (task_t) signature
                auto task = task_t{std::bind(std::forward<Func>(func), std::forward<Args>(args)...)};
                auto result = task.get_future();

                {
                    lock_guard<mutex> lock {m_lock};
                    // Call explicitly the task - so that the std::future is properly set
                    m_jobs.push(FunctionWrapper{[task = std::move(task)]() mutable { task(); }});
                }

                m_condition.notify_one();

                return result;

            }

            bool start()
            {
                try
                {
                    m_pThread = make_thread<std::thread>(&AOThread::dequeue, this);
                    return true;
                }
                catch(...)
                {
                    return false;
                }
            }


            void stop()
            {
                if (!m_pThread) return;

                {
                    std::lock_guard<std::mutex> lock {m_lock};
                    m_stopThread = true;
                }
                m_condition.notify_one();

                m_pThread.reset(nullptr); // wait on thread to join
            }

        private:

            void dequeue()
            {
                using namespace std;

                for(;;)
                {
                    FunctionWrapper job;

                    {
                        unique_lock<std::mutex> lock {m_lock};
                        m_condition.wait(lock, [this]{ return m_stopThread || !m_jobs.empty();});

                        if (m_stopThread) break;

                        job = move(m_jobs.front());
                        m_jobs.pop();
                    }

                    try
                    {
                        job();
                    }
                    catch (const bad_function_call& e)
                    {
                        cerr << e.what() << '\n';
                        //throw; // rethrow
                    }
                }
            }

        private:

            bool m_stopThread = false;
            
           // @note: Order of declaration is important!
        
            std::mutex m_lock;
            std::condition_variable m_condition;
        
            std::queue<FunctionWrapper> m_jobs;

            thread_with_deleter_t<std::thread> m_pThread = nullptr;
           
    };
}



#endif /* DS_AOT_AOTHREAD_H_ */
