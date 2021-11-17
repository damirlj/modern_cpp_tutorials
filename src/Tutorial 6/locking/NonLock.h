/*
 * NonLock.h
 *
 *  Created on: Nov 13, 2021
 *  Author: <a href="mailto:damirlj@yahoo.com">Damir Ljubic</a>
*/

#ifndef AIRPLAYSERVICE_NONLOCK_H
#define AIRPLAYSERVICE_NONLOCK_H

namespace utils::locking
{
    /**
     * For the single-thread environment,
     * there is no need for the locking mechanism
     *
     */
    class NonLock
    {
        public:
            struct Lock final
            {
                explicit Lock(const NonLock& ){}
                ~Lock() = default;
            };
    };
}
#endif //AIRPLAYSERVICE_NONLOCK_H
