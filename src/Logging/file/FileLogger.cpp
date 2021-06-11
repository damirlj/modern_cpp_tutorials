//
// <author> damirlj@yahoo.com
// Copyright (c) 2021. All rights reserved!
//

#include <future>

#include "FileLogger.h"

using namespace utils::log;



template <typename Data>
FileLogger<Data>::FileLogger(std::size_t cache
        , std::unique_ptr<utils::file::OutputFileStream<Data>> file
        , std::string name
        , utils::ThreadWrapper::schedule_policy_t scheduling
        , utils::ThreadWrapper::priority_t priority
        ): m_pLogFile(std::move(file))
    , m_plogThread(std::make_unique<loggin_thread_t>(name, scheduling, priority))
{
    m_logBuffer.reserve(cache);
}


template <typename Data>
FileLogger<Data>::~FileLogger()
{
    flushCacheAndStop();
}


template <typename Data>
bool FileLogger<Data>::checkAvailableCache(std::size_t required)
{
    const auto availableCache = m_logBuffer.capacity() - m_logBuffer.size();
    return availableCache >= required;
}


template <typename Data>
std::future<void> FileLogger<Data>::flushCacheAndWrite(cache_t<Data>&& data)
{
    task_t job { [this, d=std::move(data)] {
        // Flush cache to file
        m_pLogFile->write(m_logBuffer);

        // Preserve data to cache
        m_logBuffer.clear();
        m_logBuffer.insert(m_logBuffer.end(), d.begin(), d.end());

    }};

    return m_plogThread->enqueue(std::move(job));
}


template <typename Data>
std::future<void> FileLogger<Data>::write2Cache(cache_t<Data>&& data)
{
    task_t job { [this, d = std::move(data)] {

        m_logBuffer.insert(m_logBuffer.end()
                , d.begin()
                , d.end()
                );
    }};

    return m_plogThread->enqueue(std::move(job));
}


template <typename Data>
std::future<void> FileLogger<Data>::flushCache()
{
    task_t job { [this] {
        // Flush cache to file
        m_pLogFile->write(m_logBuffer);
    }};

    return m_plogThread->enqueue(std::move(job));
}


template <typename Data>
void FileLogger<Data>::flushCacheAndStop()
{
    auto flushed = flushCache();

    flushed.get();//wait until the cache is flushed to file

    // Signal logging thread exit, and wait

    m_plogThread.reset(nullptr);

    // Close the output file medium

    m_pLogFile.reset(nullptr);
}


template <typename Data>
void FileLogger<Data>::log(cache_t<Data>&& data)
{
    if (!checkAvailableCache(data.size()))//don't allow the reallocation of the cache
    {
        (void)flushCacheAndWrite(std::move(data));//write cache to file and preserve the data afterwards
        return;
    }

    // Enough slots - write into cache

    (void)write2Cache(std::move(data));
}


namespace utils::log
{
    /*
     * Template specialization
     *
     * use
     *    extern template class utils::log::FileLogger<X>;
     *
     * at the client side, to instruct the compiler to search for template specialization
     * in this translation unit, and avoid creation object file with template class for each
     * translation unit where it's included
     */

    template class FileLogger<uint8_t>;//use utils::file::BinaryOutputFileStream for the output file - to log BLOB data
    template class FileLogger<std::string>;//use utils::file::CharOutputFileStream for the output file - to log traces
}



