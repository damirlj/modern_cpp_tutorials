//
// Created by Damir Ljubic (damirlj@yahoo.com) on 05.03.2021.
// Copyright (c) 2021. All rights reserved.
//

#ifndef LOGGING_CONSOLELOGGER_H_
#define LOGGING_CONSOLELOGGER_H_


#include <android/log.h>

// Class-level lock
#include "ClassMutex.h"

// Logging
#include "Logger.h"
#include "LoggingHelper.h"


namespace utils::log
{

    template<typename...Args>
    void logging(android_LogPriority level, const std::string& tag, Args&&...args)
    {
        __android_log_print(level, tag.c_str(), std::forward<Args>(args)...);
    }



    /**
     * @brief Android specific logging to terminal (Logcat) implementation
     */
    class ConsoleLogger final : public LoggerBase<ConsoleLogger>
    {
        public:

            using super = LoggerBase<ConsoleLogger>;
            using super::super; // Using base class c-tor(s) - to provide the logging tag


            template <typename T>
            void logImplWithTag(log_verbosity_t verbosity, string_t<T>&& msg) const
            {
                using namespace std;
                {
                    typename utils::locking::ClassLevelLock<ConsoleLogger>::Lock lock{};//class level lock

                    logging(
                            [](log_verbosity_t v)
                            {
                                switch(v)
                                {
                                    case log_verbosity_t::LOG_LEVEL_TRACE:   return ANDROID_LOG_VERBOSE;
                                    case log_verbosity_t::LOG_LEVEL_DEBUG:   return ANDROID_LOG_DEBUG;
                                    case log_verbosity_t::LOG_LEVEL_INFO:    return ANDROID_LOG_INFO;
                                    case log_verbosity_t::LOG_LEVEL_WARNING: return ANDROID_LOG_WARN;
                                    case log_verbosity_t::LOG_LEVEL_ERROR:   return ANDROID_LOG_ERROR;
                                }
                            }(verbosity)
                            , tag().c_str()
                            , "%s\n"
                            , std::forward<T>(msg).c_str()
                    );
                }
            }
    };//class:ConsoleLogger

}//namespace utils::log

#endif /* LOGGING_CONSOLELOGGER_H_ */
