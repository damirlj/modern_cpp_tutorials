/*
 * ConsoleLogger.h
 *
 *  Created on: Jan 31, 2021
 *      Author: <a href="mailto:damirlj@yahoo.com">Damir Ljubic</a>
 */

#ifndef LOGGING_CONSOLELOGGER_H_
#define LOGGING_CONSOLELOGGER_H_

#include <iostream>
#include <string>

// Application
#include "ClassLevelMutex.h"

namespace utils::log
{
    /**
     * Simplified console logger.
     *
     * Uses class-level lock for ensuring the proper console logging
     * in concurrent environment
     */
    class ConsoleLogger final
    {
        public:

            using lock_t = utils::lock::CLMutex<ConsoleLogger>;

            template <typename ...Args>
            void logAll(Args&&...args) const noexcept
            {
                std::lock_guard<lock_t> lock {lock_t::instance()};

                (std::cout << ... << std::forward<Args>(args));
            }

            template <typename...Args>
            void log(const std::string& fmt, Args&&...args) const
            {
                std::lock_guard<lock_t> lock {lock_t::instance()};

                const auto msg = utils::string_format(fmt, std::forward<Args>(args)...);//this can throw!
                std::cout << msg << std::endl;
            }

            void log(const std::string& msg) const noexcept
            {
                std::lock_guard<lock_t> lock {lock_t::instance()};

                std::cout << msg << std::endl;
            }
    };
}


#endif /* LOGGING_CONSOLELOGGER_H_ */
