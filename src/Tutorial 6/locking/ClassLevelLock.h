/*
 * ClassLevelLock.h
 *
 *  Created on: Nov 13, 2021
 *  Author: <a href="mailto:damirlj@yahoo.com">Damir Ljubic</a>
*/

#ifndef AIRPLAYSERVICE_CLASSLEVELLOCK_H
#define AIRPLAYSERVICE_CLASSLEVELLOCK_H

#include <mutex>

namespace utils::locking
{
    /**
     *
     * Policy-based design.
     * For a different type of the host provides different type of class-level lock.
     * All instances of the class shared the same locking mechanism (mutex).
     *
     * usage:
     *
     * template <typename LockingPolicy>
     * class Host : private LockingPolicy
     * {
     *      public:
     *              void f()
     *              {
     *                 typename LockingPolicy::Lock lock{*this};
     *                 ...
     *              }
     * };
     *
     */

    class ClassLevelLock
    {
        private:

            class Initializer
            {
                public:

                    using lock_type = std::mutex;

                    static Initializer& get()
                    {
                        static Initializer initializer;
                        return initializer;
                    }

                    lock_type& locker() { return m_mutex; }

                    Initializer(const Initializer&) = delete;
                    Initializer& operator=(const Initializer&) = delete;

                    Initializer(Initializer&& ) = delete;
                    Initializer& operator=(Initializer&&) = delete;

                private:
                    Initializer() = default;
                private:
                    mutable lock_type m_mutex;
            };

        public:
            class Lock;
            friend class Lock;

            /**
             *  All policy implementations refer the same inner Lock class (T::Lock).
             *  Internal class which implements java-like the common interface for all
             *  locking policies, so that can be instantiated as
             *      `typename T::Lock lock{};`, and used as scope lock - RAII idiom, or
             *  in case used with other locking policy
             *      `typename T::Lock lock{*this};`
             *
             */
            class Lock final
            {
                public:
                    Lock()
                    {
                        auto& initializer = Initializer::get();
                        initializer.locker().lock();
                    }

                    /**
                     * For being consistent with other
                     * locking policies
                     */
                    explicit Lock(const ClassLevelLock& ) : Lock()
                    {
                    }

                    ~Lock()
                    {
                        auto& initializer = Initializer::get();
                        initializer.locker().unlock();
                    }
            };//Lock
    };//
}//namespace

#endif //AIRPLAYSERVICE_CLASSLEVELLOCK_H
