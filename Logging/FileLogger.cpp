/*
 * FileLogger.cpp
 *
 *  Created on: Feb 14, 2021
 *      Author: <a href="mailto:damirlj@yahoo.com">Damir Ljubic</a>
 */

#include "FileLogger.h"

using namespace utils::log;



template <typename Data>
FileLogger<Data>::FileLogger(std::size_t cache
        , std::unique_ptr<utils::file::OutputFile<cache_t<Data>>> file
        , std::string name
        , utils::thread::ThreadWrapper::schedule_t scheduling
        , utils::thread::ThreadWrapper::priority_t priority
        ): m_pLogFile(std::move(file))
    , m_plogThread(std::make_unique<loggin_thread_t>(name, scheduling, priority))
{
    m_logBuffer.reserve(cache);
}


template <typename Data>
bool FileLogger<Data>::checkAvailableCache(std::size_t required)
{
    const auto availableCache = m_logBuffer.capacity() - m_logBuffer.size();
    return availableCache >= required;
}


template <typename Data>
void FileLogger<Data>::writeCache2File()
{
    task_t job { [this] {
        m_pLogFile->write(m_logBuffer);
    }};

    m_plogThread->enqueue(std::move(job));
}


template <typename Data>
void FileLogger<Data>::write2Cache(cache_t<Data>&& data)
{
    task_t job { [this, d = std::move(data)] {

        m_logBuffer.insert(m_logBuffer.end()
                , d.begin()
                , d.end()
                );
    }};

    m_plogThread->enqueue(std::move(job));
}

template <typename Data>
void FileLogger<Data>::clearCache()
{
    m_logBuffer.clear();
}


template <typename Data>
void FileLogger<Data>::log(cache_t<Data>&& data)
{
    if (!checkAvailableCache(data.size()))//don't allow the reallocation of the cache
    {
        //Preallocated cache is almost full - write into file
        writeCache2File();

        // Preserve the data - write into cleaned cache
        clearCache();
        write2Cache(std::move(data));

        return;
    }

    // Enough slots - write into cache

    write2Cache(std::move(data));
}


namespace utils::log
{
    /*
     * Template specialization
     * use
     *    extern template FileLogger<utils::log::X>
     *
     * at the client side, to instruct the compiler to search for template specialization
     * in this translation unit
     */

    template class FileLogger<uint8_t>;//use BinaryOutputStream for the output file
    template class FileLogger<std::string>;//use CharOutputStream for the output file
}



