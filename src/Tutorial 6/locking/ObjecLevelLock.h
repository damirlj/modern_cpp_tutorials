/*
 * ObjectLevelLock.h
 *
 *  Created on: Nov 13, 2021
 *  Author: <a href="mailto:damirlj@yahoo.com">Damir Ljubic</a>
*/

#ifndef AIRPLAYSERVICE_OBJECLEVELLOCK_H
#define AIRPLAYSERVICE_OBJECLEVELLOCK_H

#include <mutex>

namespace utils::locking
{
    /**
     * Policy-based design.
     * Object level locking: each instance has it's on dedicated
     * locking mechanism (mutex)
     *
     * usage:
     * template <typename LockingPolicy>
     * class Host : private LockingPolicy
     * {
     *      public:
     *             void f()
     *             {
     *                  typename LockingPolicy::Lock lock{*this};
     *                  ...
     *             }
     * };
     */
    class ObjectLevelLock
    {
        public:
            using lock_type = std::mutex;

            ObjectLevelLock() = default;
            ~ObjectLevelLock() = default;

            ObjectLevelLock(const ObjectLevelLock&) = delete;
            ObjectLevelLock& operator=(const ObjectLevelLock& ) = delete;

            class Lock;
            friend class Lock;

            /**
             * Idea with the inner class Lock is for implementing
             * the policy-based design, referring at the client side to the same
             * (name) inner class
             *      'typename T::Lock lock{*this};'
             *  independent of the locking policy implementation behind
             *
             */
            class Lock final
            {
                public:

                    /**
                     * C-tor
                     * @param lock  We will pass actually the reference to the "Host" class!
                     * @note        This will work only if we impose parameterized inheritance
                     *              template <typename LockingPolicy>
                     *              class Host : private LockingPolicy
                     *              {};
                     */
                    explicit Lock(const ObjectLevelLock& lock) noexcept: m_objectLock(lock)
                    {
                        m_objectLock.m_lock.lock();
                    }
                    ~Lock()
                    {
                        m_objectLock.m_lock.unlock();
                    }
            private:
                const ObjectLevelLock& m_objectLock;
        };
        private:
            mutable lock_type m_lock;

   };
}
#endif //AIRPLAYSERVICE_OBJECLEVELLOCK_H
