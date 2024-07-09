//
// Created by dalj8690 on 29.04.2020.
//

#ifndef AIRPLAYSERVICE_THREADWRAPPER_H
#define AIRPLAYSERVICE_THREADWRAPPER_H


#include <pthread.h>

#include <sys/time.h>
#include <sys/resource.h>

// linux
#ifdef __linux__
    #include <unistd.h>
#else
    #include <strstream>
#endif


// Std library
#include <thread>
#include <string>
#include <optional>
#include <cstring>
#include <functional>

// JNIEnv
#if __has_include(<jni.h>)
    #include <jni.h>
    #define JNI_INCLUDED 1
#else
    #define JNI_INCLUDED 0
#endif


namespace utils
{

    namespace pthread
    {
        #define throw_runtime_with_err(msg) throw std::runtime_error((msg) + std::string(strerror(err)))

        typedef void* (*thread_f)(void*);

        /**
         *  Create realtime thread with scheduling/priority
         *
         * @param handle The thread handle
         * @param func The thread function
         * @param context The thread function argument
         * @param policy The realtime thread scheduling policy (supported SCHED_RR/SCHED_FIFO)
         * @param priority The realtime thread priority: [1, 99]
         * @return Indication of the operation outcome: 0 on success
         */
        static int createThreadWithPrio(pthread_t* handle, thread_f func, void* context, int policy, int priority)
        {
            using namespace std::string_literals;

            pthread_attr_t attr;
            int err = pthread_attr_init(&attr);
            if (err) [[unlikely]] { throw_runtime_with_err("<Thread> Failed: 'pthread_attr_init()': "s); }

            // Set the realtime schedule policy

            err = pthread_attr_setschedpolicy(&attr, policy);
            if (err) [[unlikely]] { throw_runtime_with_err("<Thread> Failed: 'pthread_attr_setschedpolicy(): '"s); }

            // Set the priority

            struct sched_param param;
            param.sched_priority = priority;
            err = pthread_attr_setschedparam(&attr, &param);
            if (err) [[unlikely]] { throw_runtime_with_err("<Thread> Failed: 'pthread_attr_setschedparam()': "s); }

            // For this to take into account, the explicit scheduling needs to be specified.
            // Otherwise, the attributes will not be applied - the thread will inherit the process/parent thread
            // scheduling policy.
            // @note This fails, if the user is unprivileged one (without CAP_SYS_NICE capability)
            err = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
            if (err) [[unlikely]] { throw_runtime_with_err("<Thread> Failed: 'pthread_attr_setinheritsched()': "s); }

            // Create thread with a given attribute
            err = pthread_create(handle, &attr, func, context);
            if (err) [[unlikely]] { throw_runtime_with_err("<Thread> Failed: 'pthread_create()': "s); }

            pthread_attr_destroy(&attr);

            return err;
        }
    }  // namespace pthread

#if (JNI_INCLUDED == 1)
    namespace jni
    {
        /**
         * Android way of setting the (Java/Kotlin) threads priority
         * https://developer.android.com/reference/android/os/Process
         *
         * The fact is, the Android at app level supports only CFS (Completely Fair Scheduler), with
         * static priority 0. The way to apply the "weight factor" on the CPU time slices, is through
         * the "niceness", also known as dynamic priority: [-20, 19].
         * The more thread is nicer, the less time slot it will get
         */
        using priority_t = enum class ThreadPriority :int
        {
            THREAD_PRIORITY_AUDIO = -16,
            THREAD_PRIORITY_BACKGROUND = 10,
            THREAD_PRIORITY_DEFAULT = 0,
            THREAD_PRIORITY_DISPLAY = -4,
            THREAD_PRIORITY_FOREGROUND = -2,
            THREAD_PRIORITY_LESS_FAVORABLE = 1,
            THREAD_PRIORITY_LOWEST = 19,
            THREAD_PRIORITY_MORE_FAVORABLE = -1,
            THREAD_PRIORITY_URGENT_AUDIO = -19,
            THREAD_PRIORITY_URGENT_DISPLAY = -8,
            THREAD_PRIORITY_VIDEO = -10
        };

        /**
         * Setting the thread priority (niceness) within Java thread context,
         * by calling the Process.setThreadPriority
         *
         * @param env       Pointer to the JNI function table
         * @param priority  Priority (niceness) to set
         */
        static void setThreadPriority(JNIEnv* env, int priority)
        {
            try
            {
                if (env == nullptr) [[unlikely]]
                    throw std::runtime_error("<Thread> Invalid env argument");

                jclass cls = env->FindClass("android/os/Process");
                if (nullptr == cls) [[unlikely]]
                    throw std::runtime_error("<Thread> Invalid cls name.");
                jmethodID id = env->GetStaticMethodID(cls, "setThreadPriority", "(I)V");
                if (nullptr == id) [[unlikely]]
                    throw std::runtime_error("<Thread> Invalid method id.");

                env->CallStaticVoidMethod(cls, id, static_cast<jint>(priority));
            }
            catch (const std::runtime_error& e)
            {
                if (env->ExceptionCheck())
                {
                    env->ExceptionDescribe();
                    env->ExceptionClear();
                }
            }
        }
    }  // namespace jni
#endif

    /**
     * Wrapper around the std::thread implementation.
     *
     * Extended with ability to specify the thread priority (along with
     * the scheduling policy), name and thread affinity
     */
    class ThreadWrapper final : public std::thread
    {

      public:
        using handle_t = pthread_t;
        inline static constexpr std::size_t MAX_SIZE_BYTES = 16;  //@note linux limitation!

        using base = std::thread;
        using base::base;  // use base class c-tors

        // clang-format off
        using schedule_policy_t = enum class ESchedule : int
        {
            sh_policy_normal = SCHED_OTHER,
            sh_policy_rr = SCHED_RR,
            sh_policy_fifo  [[maybe_unused]] = SCHED_FIFO
        };
        // clang-format on
        using priority_t = int;

#if (JNI_INCLUDED == 1)
        /**
         * For creating the native thread by attaching it
         * to the Java thread, from which context the thread priority - niceness
         * will be set
         *
         * @param jvm The reference to the Java Virtual Machine
         * @param priority The niceness of a thread : [-20,19]
         * @param name The name of a thread
         * @param func The thread function
         * @param args The thread function arguments
         *
         * @note May throw!
         */
        template <typename Func, typename... Args>
        ThreadWrapper(JavaVM* jvm, priority_t priority, std::string name, Func&& func, Args&&... args);
#endif
        /**
         * For creating realtime thread
         *
         * @param policy The realtime thread schedule policy (SCHED_RR/SCHED_FIFO)
         * @param priority The realtime thread priority
         * @param func Thread function
         * @param args Thread function arguments
         *
         * @note May throw!
         */
        template <typename Func, typename... Args>
        ThreadWrapper(schedule_policy_t policy, priority_t priority, std::string name, Func&& func, Args&&... args);


        ThreadWrapper(const base&) = delete;
        ThreadWrapper& operator=(const base&) = delete;

        /*
         * std::thread supports only move semantic - because by moving the ownership, thread that
         * loses ownership over the thread function is to be considered unjoinable:
         * calling destructor on unjoinable thread will not yield the exception - terminates the program
         */
        ThreadWrapper(ThreadWrapper&&) noexcept = default;
        inline ThreadWrapper& operator=(ThreadWrapper&& threadWrapper) noexcept
        {
            wait();  // wait on joinable thread: the one that is started or detached

            ThreadWrapper tw{std::move(threadWrapper)};
            std::swap(*this, tw);

            return *this;
        }

        /**
         * Destructor
         *
         * like std::jthread RAII approach
         * https://github.com/josuttis/jthread
         *
         */
        inline ~ThreadWrapper() { wait(); }

        /**
         * Setting the thread priority, along with the scheduling policy.
         *
         * @param policy    Scheduling policy
         * @param priority  Thread priority (niceness - for CFS)
         * @return Indication of the operation outcome: true on success
         */
        inline bool setPriority(schedule_policy_t policy, priority_t priority)
        {
            return 0 == setPriority(native_handle(), std::underlying_type_t<schedule_policy_t>(policy), priority);
        }

        [[nodiscard]] inline auto tid()
        {
#ifdef __ANDROID__
            return static_cast<unsigned long>(gettid());
#elif defined(__POSIX__)
            return static_cast<unsigned long>(pthread_gettid_np(native_handle()));
#else
            std::strstream tid;
            tid << get_id();
            return std::stoul(tid.str());
#endif
        }

#ifdef __ANDROID__
        /**
         * According to Android limitations - we actually can set only niceness [-20, 19] for
         * CFS scheduling policy
         * @param nice Niceness to set.
         * @return Indication of the operation outcome: TRUE on success
         */
        inline bool setPriority(priority_t nice)
        {
            constexpr auto MIN_NICE = -20;
            constexpr auto MAX_NICE = 19;
            if (nice < MIN_NICE || nice > MAX_NICE) return false;

            return 0 == ::setpriority(PRIO_PROCESS, tid(), nice);
        }
        [[nodiscard]] inline int getPriority() { return ::getpriority(PRIO_PROCESS, tid()); }
#endif

        inline void wait()
        {
            if (joinable()) { join(); }
        }


        /**
         * Setting the name for a thread
         *
         * @param name The name of a thread
         * @return Indication of the operation outcome: true on success
         */
        inline bool setName(std::string name)
        {
            // std::string::size() returns number of chars, not including
            // null-terminated string
            if (name.size() >= MAX_SIZE_BYTES)
            {
                name.resize(MAX_SIZE_BYTES);
                name[MAX_SIZE_BYTES - 1] = '\0';
            }

            return 0 == setName(native_handle(), name);
        }

        inline std::optional<std::string> getName() { return getName(native_handle()); }

        inline bool setAffinity(std::optional<int> core)
        {
            const auto num_cpus = std::thread::hardware_concurrency();

            if (core)  // value set
            {
                if (*core < 0 || *core > static_cast<int>(num_cpus)) return false;
                return 0 == setAffinity(native_handle(), *core);
            }
            // core not specified: set the current CPU as designated one: prevents thread migration
            const auto core_id = sched_getcpu();
            return 0 == setAffinity(native_handle(), core_id);
        }

      private:
        inline int setAffinity(handle_t handle, int core)
        {
            cpu_set_t cpuset;

            CPU_ZERO(&cpuset);
            CPU_SET(core, &cpuset);
#ifdef __ANDROID__
            return sched_setaffinity(tid(), sizeof(cpu_set_t), &cpuset);
#else
            return pthread_setaffinity_np(handle, sizeof(cpu_set_t), &cpuset);
#endif
        }

        inline int setPriority(handle_t handle, int policy, int priority)
        {
            struct sched_param param;
            param.sched_priority = priority;

            return pthread_setschedparam(handle, policy, &param);
        }

        inline int setName(handle_t handle, std::string_view name) { return pthread_setname_np(handle, name.data()); }

        inline std::optional<std::string> getName(handle_t handle) const
        {
            char name[MAX_SIZE_BYTES] = {'\0'};
            if (0 == pthread_getname_np(handle, name, sizeof(name))) { return std::string(name); }
            return {};  // error happened: 'errno' is set
        }

    };  // class ThreadWrapper

#if (JNI_INCLUDED == 1)
    namespace jni
    {
        struct JNIThreadAnchor final
        {

            explicit JNIThreadAnchor(JavaVM* jvm) noexcept
                : m_pJavaVM(jvm)
            {
                if (m_pJavaVM) [[likely]]
                    m_pJavaVM->AttachCurrentThread(&m_pEnv, nullptr);
            }
            inline ~JNIThreadAnchor()
            {
                if (m_pJavaVM && m_pEnv) [[likely]]
                    m_pJavaVM->DetachCurrentThread();
            }

            // Ownership over JNIEnv* for attached thread is not sharable, nor movable
            JNIThreadAnchor(const JNIThreadAnchor&) = delete;
            JNIThreadAnchor& operator=(const JNIThreadAnchor&) = delete;
            JNIThreadAnchor(JNIThreadAnchor&&) = delete;
            JNIThreadAnchor& operator=(JNIThreadAnchor&&) = delete;

            /**
             * Boolean operator
             * @return Indicator whether the native thread is successfully attached.
             */
            inline explicit operator bool() const { return nullptr != m_pEnv; }
            inline JNIEnv* get() const { return m_pEnv; }

          private:
            JavaVM* m_pJavaVM;
            JNIEnv* m_pEnv = nullptr;
        };
    }  // namespace jni

    template <typename Func, typename... Args>
    inline ThreadWrapper::ThreadWrapper(JavaVM* jvm, priority_t priority, std::string name, Func&& func, Args&&... args)
        : std::thread(
            [=, func_ = std::forward<Func>(func)](Args&&... args)
            {
                jni::JNIThreadAnchor threadAnchor{jvm};
                if (not threadAnchor) [[unlikely]]
                    throw std::runtime_error("<Thread> Failed to attach native thread!");

                // Set priority (niceness): at Java side, otherwise EPERM will be returned
                jni::setThreadPriority(threadAnchor.get(), priority);

                // Set name: at native side
                std::ignore = setName(name);

                // Native thread function
                std::invoke(func_, std::forward<Args>(args)...);
            },
            std::forward<Args>(args)...)
    {}
#endif

    template <typename Func, typename... Args>
    inline ThreadWrapper::ThreadWrapper(
        utils::ThreadWrapper::schedule_policy_t policy,
        utils::ThreadWrapper::priority_t priority,
        std::string name,
        Func&& func,
        Args&&... args)
        : std::thread(
            [=, func_ = std::forward<Func>(func)](Args&&... args)
            {
                // Set priority
#if __ANDROID__
                if (policy == utils::ThreadWrapper::schedule_policy_t::sh_policy_normal) [[likely]]
                {
                    std::ignore = setPriority(priority);
                }
                else
#endif
                {
                    std::ignore = setPriority(policy, priority);
                }

                // Set name
                std::ignore = setName(name);

                // Native thread function
                std::invoke(func_, std::forward<Args>(args)...);
            },
            std::forward<Args>(args)...)
    {}


    /**
     * Helper type.
     * Custom thread deleter, in case that thread needs to be joined/detach.
     *
     * @note Only for the backward compatibility, since <br>
     * ~ThreadWrapper is refactored to resolve this issue!
     */
    struct [[maybe_unused]] ThreadDeleter
    {
        void operator()(ThreadWrapper* p) const
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

    template <typename... Args>
    [[maybe_unused]] auto make_thread_ptr(Args&&... args) noexcept -> thread_ptr_t
    {
        if constexpr ((std::is_constructible_v<ThreadWrapper, Args&&> && ...))
        {
            return thread_ptr_t(new (std::nothrow) ThreadWrapper(std::forward<Args>(args)...), thread_deleter_t{});
        }
        else
        {
            return nullptr;
        }
    }
}  // namespace utils

#endif  // AIRPLAYSERVICE_THREADWRAPPER_H
