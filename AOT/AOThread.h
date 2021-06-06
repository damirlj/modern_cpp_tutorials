/*
 * AOThread.h
 *
 *  Created on: Dec 30, 2020
 *      Author: <a href="mailto:damirlj@yahoo.com">Damir Ljubic</a>
 */

#ifndef AOT_AOTHREAD_H_
#define AOT_AOTHREAD_H_

#include <memory>
#include <string>
#include <iostream>

#include "JobQueue.h"
#include "../Thread/ThreadWrapper.h"

#include "../commons/Commons.h"



namespace utils::aot
{

    /**
     *  AOT - Active Object Thread design pattern.
     *
     *  Provides the thread context for executing asynchronous tasks.
     *  The interaction with the client code is through restrictive interface which exposes single
     *  method for tasks to be serialized - stored into the jobs queue.
     *  Thread drains the queue providing the background context in which tasks will be executed
     *  sequentially, one-by-one.
     *
     * @tparam R        Return value type of task
     *
     */
    template <typename R = void>
    class AOThread final
    {

        public:

            using task_queue_t = JobQueue<R>;

            AOThread(std::string name
                    , utils::thread::ThreadWrapper::schedule_t policy
                    , utils::thread::ThreadWrapper::priority_t priority):
                m_pJobQueue (std::make_unique<task_queue_t>()),
                m_pJobThread(utils::thread::make_thread_ptr(&AOThread::threadFunc, this))

            {
               start(name, policy, priority);
            }

            ~AOThread()//user-defined c-tor: prevents generating default (memberwise) move-operations
            {
                stop();
            }

            // Copy-operations explicit forbidden

            AOThread(const AOThread&) = delete;
            AOThread& operator = (const AOThread&) = delete;

            /**
             * Enqueue the task
             *
             * @note If the job should have an additional arguments,
             * use std::bind - to bind the task with the arguments.
             *
             * @param job   The task to be enqueued
             * @return      The future, for waiting on result, if any
             */
            auto enqueue(utils::aot::job_t<R>&& job) noexcept
            {
               return m_pJobQueue->enqueue(std::move(job));//thread-safe task queue
            }

        private:

            void start(std::string name
                    , utils::thread::ThreadWrapper::schedule_t policy
                    , int priority) noexcept
            {
                if (m_pJobThread)
                {
                   (void)m_pJobThread->setName(name);
                   (void)m_pJobThread->setPriority(policy, priority);
                }
            }

            void stop()
            {
                m_pJobQueue->stop();//stop dequeuing: signal thread exit

                if (m_pJobThread)
                {
                    m_pJobThread.reset(nullptr);//wait on jobs thread to join
                }
            }

            /**
             * Serialized all tasks to be executed sequentially,
             * within the single background thread
             */
            void threadFunc() noexcept;


        private:

            std::unique_ptr<task_queue_t> m_pJobQueue = nullptr;
            utils::thread::thread_ptr_t m_pJobThread = nullptr;
    };

    template <typename R>
    void AOThread<R>::threadFunc() noexcept
    {
        using namespace std;

        for(;;)
        {

            // Suspend thread, until the queue is empty or exit is not signaled
            auto job = m_pJobQueue->dequeue();

            if (!job) //exit signaled
            {
                break;
            }


            try
            {
                (*job)();
            }
            catch(const std::bad_function_call& e)
            {
                //todo: add logging policy
                break;
            }

        }//for(;;)
    }


} /* namespace utils::aot */

#endif /* AOT_AOTHREAD_H_ */
