/*
 * ClassLevelMutex.h
 *
 *  Created on: Jan 31, 2021
 *      Author: <a href="mailto:damirlj@yahoo.com">Damir Ljubic</a>
 */

#ifndef LOCK_CLASSLEVELMUTEX_H_
#define LOCK_CLASSLEVELMUTEX_H_

#include <mutex>

namespace utils::lock
{
    /**
     * Class-level mutex
     *
     * @tparam      Host The host class for which the mutex will provide class-level
     *              mutex protection: for all instances of the Host class
     */
    template <class Host>
    class CLMutex final : public std::mutex
    {
        public:

            static CLMutex& instance()
            {
                static CLMutex clMutex;//Scott Meyers singleton pattern
                return clMutex;
            }

        private:

            CLMutex() = default;
    };
}



#endif /* LOCK_CLASSLEVELMUTEX_H_ */
