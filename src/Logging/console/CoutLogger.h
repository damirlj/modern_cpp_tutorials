//
// Created by Damir Ljubic (damirlj@yahoo.com) on 06.03.2021.
// Copyright (c) 2021. All rights reserved.
//

#ifndef AIRPLAYSERVICE_COUTLOGGER_H
#define AIRPLAYSERVICE_COUTLOGGER_H

#include <iostream>

//Class-level lock
#include "ClassMutex.h"

// Logging
#include "Logger.h"
#include "LoggingHelper.h"



namespace utils::log
{
    static inline constexpr auto LOG_LEVEL = log_verbosity_t::LOG_LEVEL_INFO; //tracing filter

    /**
     * @brief Platform independent Logging to the std::cout.
     * Meant to be used with unit tests that needs to run
     * on the local machine
     */
    class CoutLogger final : public LoggerBase<CoutLogger>
    {
        public:
            using super = LoggerBase<CoutLogger>;
            using super::super; // Using base class c-tor(s): to provide the logging tag


            template <typename T>
            void logImplWithTag(log_verbosity_t verbosity, string_t<T>&& msg) const
            {
                using namespace std;

                typename utils::locking::ClassLevelLock<CoutLogger>::Lock lock{};//class level lock

                auto s = utils::string_format("<%s>: %s\n", tag().c_str(), std::forward<T>(msg).c_str());

                switch(verbosity)
                {
                    case log_verbosity_t::LOG_LEVEL_ERROR:
                        cerr << s;
                    break;
                    default:
                        if (verbosity >= LOG_LEVEL)
                        {
                            cout << s;
                        }
                    break;
                }
            }
    };// CoutLogger

}//namespace utils::log

#endif //AIRPLAYSERVICE_COUTLOGGER_H
