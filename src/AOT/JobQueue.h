/*
 * JobQueue.h
 *
 *  Created on: Dec 29, 2020
 *      Author: <a href="mailto:damirlj@yahoo.com">Damir Ljubic</a>
 */

#ifndef AOT_JOBQUEUE_H_
#define AOT_JOBQUEUE_H_

#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <optional>

namespace utils::aot
{

    template<typename R, typename...Args>
    using job_t = std::packaged_task<R(Args...)>;//universal job signature

    template<typename R, typename...Args>
    using job_queue_t = std::queue<job_t<R, Args...>>;//universal queue of jobs-callable objects

    /**
     *  Specialization of the std::queue<T>.
     *
     *  The queue is designed to hold the jobs - callable objects (tasks).
     *  Helper class for implementing the AOT (Active Object Thread) design pattern.
     *  The queue is implemented in a thread-safe manner.
     *
     *  @see AOThread
     *
     *  @tparam R   Callable object return type
     */

    template <typename R>
    class JobQueue final: public job_queue_t<R>
    {
        public:

            using value_type = job_t<R>;

            using super = job_queue_t<R>;
            using super::super;//using base class (std::queue) c-tors

            ~JobQueue() = default; //user-defined destructor will prevent generating default - memberwise move operations

            // Copy operations discarded

            JobQueue(const JobQueue<R>& ) = delete;
            JobQueue<R>& operator = (const JobQueue<R>&) = delete;



            /**
             * Enqueue the task as rvalue reference, since
             * std::packaged_task is movable only type.
             *
             * @note If the task requires additional arguments (Args...),
             * use std::bind - to bind the callable object with arguments.
             *
             *
             * @param job   The callable object to enqueue
             * @return      The result of the task, if any (std::future<void>).
             *              The return value is in that case used only for synchronization
             */
            template <typename...Args>
            std::future<R> enqueue(job_t<R, Args...>&& job, Args&&...args) noexcept
            {
                std::future<R> result;
                {
                     std::lock_guard<std::mutex> lock {m_mutex};

                     result = job.get_future();
                     value_type task { std::bind(std::move(job), std::forward<Args>(args)...)};
                     this->push(std::move(task));
                }

                m_condition.notify_one();

                return result;
            }

            std::future<R> enqueue(value_type&& job) noexcept
            {
                std::future<R> result;
                {
                     std::lock_guard<std::mutex> lock {m_mutex};

                     result = job.get_future();
                     this->push(std::move(job));
                }

                m_condition.notify_one();

                return result;
            }

            /**
             * Dequeue the job from the queue.
             * This is concurrent operation to enqueue - it will block
             * until either the queue is empty, or stop dequeuing is not signaled
             *
             * @return  The task to be executed, or none-value in case that stop is signaled
             *
             */
            std::optional<value_type> dequeue() noexcept
            {
                std::unique_lock<std::mutex> lock {m_mutex};

                m_condition.wait(lock, [this]{return !this->empty() || m_stopDequeuing;});

                if (m_stopDequeuing) return {};

                auto job = std::move(this->front());
                this->pop();

                return job;
            }

            /**
             * Force stopping dequeuing
             */
            void stop() noexcept
            {
                m_stopDequeuing = true;

                m_condition.notify_one();
            }


        private:

            bool m_stopDequeuing = false;

            std::mutex m_mutex {};//neither copyable, nor movable
            std::condition_variable m_condition {};//neither copyable, nor movable

    };

}


#endif /* AOT_JOBQUEUE_H_ */
