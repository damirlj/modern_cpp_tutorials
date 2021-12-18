/*
 * ConsoleLogger.hxx
 *
 *  Created on: Dec 15, 2021
 *      Author: <a href="mailto:damirlj@yahoo.com">Damir Ljubic</a>
 */

#ifndef TUTORIAL_TYPE_ERASURE_CONSOLELOGGER_HXX_
#define TUTORIAL_TYPE_ERASURE_CONSOLELOGGER_HXX_

#include <string>
#include <iostream>
#include <mutex>

namespace test::ep
{
    class ConsoleLogger
    {
        public:

            explicit ConsoleLogger(std::mutex& lock) : m_lock(lock)
            {}
            void log(const std::string& msg) const
            {
                std::lock_guard<std::mutex> lock {m_lock};
                std::cout << msg << '\n';
            }
        private:
            std::mutex& m_lock;
    };
}


#endif /* TUTORIAL_TYPE_ERASURE_CONSOLELOGGER_HXX_ */
