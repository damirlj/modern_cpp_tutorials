/*
 * AOThread.h
 *
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
     * For wrapping the only copy-constructible callable objects
     * into movable object, since std::packaged_task is move-only
     * The solution is taken from "Concurrency in action" Anthony Williams
     */
    class FunctionWrapper final
    {
        struct FunctionWrapperBase
        {
            virtual ~FunctionWrapperBase() = default;
            virtual void call() = 0;
        };

        template <typename Func>
        struct FunctionWrapperBaseImpl : FunctionWrapperBase
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
                m_pFunctionWrapper(std::make_unique<FunctionWrapperBaseImpl<Func>>(std::move(func)))
            {}

            FunctionWrapper() = default;
            ~FunctionWrapper() = default;

            FunctionWrapper(FunctionWrapper&& other):m_pFunctionWrapper(std::move(other.m_pFunctionWrapper))
            {}
            FunctionWrapper& operator=(FunctionWrapper&& other)
            {
                m_pFunctionWrapper = std::move(other.m_pFunctionWrapper);
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
            ~AOThread() = default;

            AOThread(const AOThread& ) = delete;
            AOThread& operator = (const AOThread&) = delete;

            template <typename Func>
            auto enqueue(Func&& func)
            {
                using namespace std;

                using result_t = invoke_result_t<Func>;
                using task_t = packaged_task<result_t()>;

                task_t task {FunctionWrapper(move(func))};
                auto f = task.get_future();

                {
                    lock_guard<std::mutex> lock {m_lock};
                    m_jobs.push(move(task));
                }

                m_condition.notify_one();

                return f;
            }

            template <typename Func, typename...Args>
            auto emplace_enqueue(Func&& func, Args&&...args)
            {
                using namespace std;

                using result_t =invoke_result_t<Func&&, Args&&...>;
                using task_t = packaged_task<result_t()>;


                task_t task { bind(std::forward<Func>(func), std::forward<Args>(args)...)};//implicit conversion to FunctionWrapper
                auto f =  task.get_future();

                {
                    lock_guard<mutex> lock {m_lock};
                    m_jobs.push(move(task));
                }

                m_condition.notify_one();

                return f;

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
                using namespace std;
                {
                    lock_guard<std::mutex> lock {m_lock};
                    m_stopThread = true;
                }

                m_condition.notify_one();
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

            thread_with_deleter_t<std::thread> m_pThread = nullptr;
            std::queue<FunctionWrapper> m_jobs;


            std::mutex m_lock;
            std::condition_variable m_condition;
    };
}



#endif /* DS_AOT_AOTHREAD_H_ */
