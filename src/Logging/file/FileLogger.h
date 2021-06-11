//
// <author> damirlj@yahoo.com
// Copyright (c) 2021. All rights reserved!
//

#ifndef LOGGING_FILELOGGER_H_
#define LOGGING_FILELOGGER_H_


#include <cstdint>
#include <vector>
#include <type_traits>
#include <memory>

#include "DataLogger.h"

#include "FileStream.h"
#include "AOThread.h"


namespace utils::log
{


        template <typename Data>
        using cache_t = std::vector<Data>;//uint8_t(std::byte), std::string

        /**
         * Logger class
         * Logging the data into file medium.
         * Designed having in mind logging binary data (uint8_t), or
         * std::string messages.
         * Should work for an arbitrary data type.
         */

        template <typename Data>
        class FileLogger final : public ILoggerR<cache_t<Data>>
        {

            public:

                FileLogger(std::size_t cache
                        , std::unique_ptr<utils::file::OutputFileStream<Data>> file
                        , std::string name
                        , utils::ThreadWrapper::schedule_policy_t scheduling
                        , utils::ThreadWrapper::priority_t priority
                );

                ~FileLogger() override;

                /**
                 * The logging task will be serialize into background
                 * thread
                 * @param data  The logging data
                 */
                void log(cache_t<Data>&& data) override;

            private:

                // Helper methods

                bool checkAvailableCache(std::size_t required);
                std::future<void> write2Cache(cache_t<Data>&& data);
                std::future<void> flushCacheAndWrite(cache_t<Data>&& data);
                std::future<void> flushCache();
                void flushCacheAndStop();

            private:


                cache_t<Data> m_logBuffer;
                std::unique_ptr<utils::file::OutputFileStream<Data>> m_pLogFile;

                using task_t = utils::aot::job_t<void>;
                using loggin_thread_t = utils::aot::AOThread<void>;

                std::unique_ptr<loggin_thread_t> m_plogThread;
        };

}//namespace: uitls::log

#endif /* LOGGING_FILELOGGER_H_ */
