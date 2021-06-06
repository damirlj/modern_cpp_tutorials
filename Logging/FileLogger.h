/*
 * FileLogger.h
 *
 *  Created on: Feb 14, 2021
 *      Author: <a href="mailto:damirlj@yahoo.com">Damir Ljubic</a>
 */

#ifndef LOGGING_FILELOGGER_H_
#define LOGGING_FILELOGGER_H_


#include <cstdint>
#include <vector>
#include <type_traits>
#include <memory>

#include "DataLogger.h"

#include "File.h"
#include "AOThread.h"


namespace utils::log
{

    template <typename Data>
    using cache_t = std::vector<Data>;//uint8_t, std::string

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
                    , std::unique_ptr<utils::file::OutputFile<cache_t<Data>>> file
                    , std::string name
                    , utils::thread::ThreadWrapper::schedule_t scheduling
                    , utils::thread::ThreadWrapper::priority_t priority
                    );

            ~FileLogger() = default;

            /**
             * The logging task will be serialize into background
             * thread
             * @param data  The logging data
             */
            void log(cache_t<Data>&& data) override;

        private:

            bool checkAvailableCache(std::size_t required);
            void writeCache2File();
            void write2Cache(cache_t<Data>&& data);
            void clearCache();

        private:


            cache_t<Data> m_logBuffer;
            std::unique_ptr<utils::file::OutputFile<cache_t<Data>>> m_pLogFile;

            using task_t = utils::aot::job_t<void>;
            using loggin_thread_t = utils::aot::AOThread<void>;

            std::unique_ptr<loggin_thread_t> m_plogThread;
   };

} /* namespace utils::log */

#endif /* LOGGING_FILELOGGER_H_ */
