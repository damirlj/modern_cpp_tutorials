/*
 * ThreadWrapper.h
 *
 *  Created on: Dec 29, 2020
 *      Author: <a href="mailto:damirlj@yahoo.com">Damir Ljubic</a>
 */

#ifndef THREAD_THREADWRAPPER_H_
#define THREAD_THREADWRAPPER_H_

#include <thread>
#include <string>
#include <memory>
#include <optional>

#include <pthread.h>

#include "Commons.h"

namespace utils::thread
{

    /**
     *  Wrapper around the std::thread implementation.
     *
     *  Extended with ability to additionally specify:
     *      - thread priority (along with the scheduling policy)
     *      - name
     *      - thread affinity
     */
    class ThreadWrapper final : public std::thread
    {

        public:

            inline static constexpr std::size_t MAX_SIZE_BYTES = 16;//@note linux limitation!

            using handle_t = pthread_t;
            using priority_t = int;

            using schedule_t = enum class ESchedule : int
            {
                  normal = SCHED_OTHER
                , rr = SCHED_RR
                , fifo [[maybe_unused]] = SCHED_FIFO
            };


            using std::thread::thread; //use base class c-tors

            /**
             * Setting the thread priority, along with the
             * scheduling policy
             *
             * @param policy    Scheduling policy
             * @param priority  Thread priority
             * @return          Indication of the operation outcome: true on success
             */
            bool setPriority(schedule_t policy, priority_t priority)
            {
                return 0 == setPriority(native_handle()
                        , utils::toUType(policy)
                        , priority);
            }


            void wait()
            {
                if (joinable())
                {
                    join();
                }
            }


            bool setName(std::string name)
            {

                if (name.size() >= MAX_SIZE_BYTES)//std::string::size() returns number of chars, not including null-terminated string
                {
                    name.resize(MAX_SIZE_BYTES);
                    name[MAX_SIZE_BYTES - 1] = '\0';
                }

                return 0 == setName(native_handle(), name);
            }

            std::optional<std::string> getName()
            {
                return getName(native_handle());
            }

            bool setAffinity(std::optional<int> core)
            {
                const auto num_cpus = std::thread::hardware_concurrency();

                if (core)//value set
                {
                    if (*core < 0 || * core > static_cast<int>(num_cpus)) return false;
                    return 0 == setAffinity(native_handle(), *core);
                }

                const auto core_id = sched_getcpu();//set the current CPU as designated one: prevents thread migration

                return 0 == setAffinity(native_handle(), core_id);
            }

        private:

                int setAffinity(handle_t handle, int core)
                {
                   cpu_set_t cpuset;

                   CPU_ZERO(&cpuset);
                   CPU_SET(core, &cpuset);

                   return pthread_setaffinity_np(handle, sizeof(cpu_set_t), &cpuset);
                }

                int setPriority(handle_t handle, int policy, int priority)
                {
                    struct sched_param param;
                    param.sched_priority = priority;

                    return pthread_setschedparam(handle, policy, &param);
                }


                int setName(handle_t handle, const std::string& name)
                {
                    return pthread_setname_np(handle, name.c_str());
                }

                std::optional<std::string> getName(handle_t handle)
                {
                    char name[MAX_SIZE_BYTES] = {'\0'};
                    if ( 0 == pthread_getname_np(handle, name, sizeof(name) ))
                    {
                        return std::string(name);
                    }

                    return {};//error occurred: 'errno' is set
                }


    };//class ThreadWrapper

    /**
     * Helper type.
     * Custom thread deleter, in case that thread needs to be joined.
     *
     * @note the std::unique_ptr can't be in this case created with
     * std::make_unique, since this assumes default deallocation policy (operator ::delete)
     *
     * template<typename T, typename...Args>
     * auto make_unique(Args&&...args)
     * {
     *    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
     * }
     *
     * Instead, the
     * std::unique_ptr<ThreadWrapper>(new ThreadWrapper(Args...), ThreadDeleter{})
     * should be called
     * @see make_thread_ptr
     */
    struct ThreadDeleter
    {
        void operator()(ThreadWrapper * p) const
        {
            if (p)
            {
                p->wait();
                delete p;
            }
        }
    };
    using thread_deleter_t = ThreadDeleter;

    using thread_ptr_t = std::unique_ptr<ThreadWrapper, thread_deleter_t>;

    template<typename...Args>
    auto make_thread_ptr(Args&&...args) noexcept
    {
        return thread_ptr_t(new (std::nothrow) ThreadWrapper(std::forward<Args>(args)...)
                , thread_deleter_t{});
    }

}//namespace utils::thread



#endif /* THREAD_THREADWRAPPER_H_ */
