//
// Created by Damir Ljubic (damirlj@yahoo.com) on 05.03.2021.
// Copyright (c) 2021. All rights reserved.
//

#ifndef LOGGERWRAPPER_H
#define LOGGERWRAPPER_H

#include <utility>
#include <memory>
#include <sstream>

#include "Logger.h"
#include "LoggingHelper.h"



namespace utils::log
{

    /**
     * @brief Wrapper around the logger implementation
     * Provides the helper methods for different verbosity levels
     *
     * @tparam LoggerImpl The concrete logger implementation type
     */
    template <class LoggerImpl>
    class LoggerWrapper final
    {
            static_assert(std::is_base_of_v<LoggerBase<LoggerImpl>, LoggerImpl>, "Invalid logger type!");

        public:

            using logger_t = LoggerImpl;


            /**
             * @brief C-tor
             *
             * @param tag The message android-like tag
             * @param args The variadic list of arguments for the logger factory method
             */
            template <typename...Args>
            explicit LoggerWrapper(std::string tag, Args&&...args) noexcept:
                m_pLogger(LoggerBase<logger_t>::createLogger(tag, std::forward<Args>(args)...))
            {}


            ~LoggerWrapper() = default;

            // Move operations allowed (member-wise)

            LoggerWrapper(LoggerWrapper&& ) noexcept = default;
            LoggerWrapper& operator = (LoggerWrapper&& ) noexcept = default;

            // Copy operations forbidden

            LoggerWrapper(const LoggerWrapper&) = delete;
            LoggerWrapper& operator = (const LoggerWrapper&) = delete;


            // Trace level

            template <typename T>
            void logTrace(string_t<T>&& msg) const
            {
                log(log_verbosity_t::LOG_LEVEL_TRACE, std::forward<T>(msg));
            }

            template <typename T>
            void logTraceWithFunc(const char* func, string_t<T>&& msg) const
            {
                logWithFunc(log_verbosity_t::LOG_LEVEL_TRACE, func, std::forward<T>(msg));
            }

            template <typename...Args>
            void logTraceArgsWithFunc(const char* func, Args&&...args) const
            {
                logArgsWithFunc(log_verbosity_t::LOG_LEVEL_TRACE, func, std::forward<Args>(args)...);
            }

            template <typename...Args>
            void logTraceFormatted(const std::string& format, Args&&...args) const
            {
                logFormatted(log_verbosity_t::LOG_LEVEL_TRACE, format, std::forward<Args>(args)...);
            }

            template <typename...Args>
            void logTraceFormattedWithFunc(const char* func, const std::string& format, Args&&...args) const
            {
                logFormattedWithFunc(log_verbosity_t::LOG_LEVEL_TRACE
                        , func
                        , format
                        , std::forward<Args>(args)...);
            }


            // Debug level

            template <typename T>
            void logDebug(string_t<T>&& msg) const
            {
                log(log_verbosity_t::LOG_LEVEL_DEBUG, std::forward<T>(msg));
            }

            template <typename T>
            void logDebugWithFunc(const char* func, string_t<T>&& msg) const
            {
                logWithFunc(log_verbosity_t::LOG_LEVEL_DEBUG, func, std::forward<T>(msg));
            }

            template <typename...Args>
            void logDebugArgsWithFunc(const char* func, Args&&...args) const
            {
                logArgsWithFunc(log_verbosity_t::LOG_LEVEL_DEBUG, func, std::forward<Args>(args)...);
            }

            template <typename...Args>
            void logDebugFormatted(const std::string& format, Args&&...args) const
            {
                logFormatted(log_verbosity_t::LOG_LEVEL_DEBUG, format, std::forward<Args>(args)...);
            }

            template <typename...Args>
            void logDebugFormattedWithFunc(const char* func
                    , const std::string& format, Args&&...args) const
            {
                logFormattedWithFunc(log_verbosity_t::LOG_LEVEL_DEBUG
                        , func
                        , format
                        , std::forward<Args>(args)...);
            }



            // Info level

            template <typename T>
            void logInfo(string_t<T>&& msg) const
            {
                log(log_verbosity_t::LOG_LEVEL_INFO, std::forward<T>(msg));
            }

            template <typename T>
            void logInfoWithFunc(const char* func, string_t<T>&& msg) const
            {
                logWithFunc(log_verbosity_t::LOG_LEVEL_INFO, func, std::forward<T>(msg));
            }

            template <typename...Args>
            void logInfoArgsWithFunc(const char* func, Args&&...args) const
            {
                logArgsWithFunc(log_verbosity_t::LOG_LEVEL_INFO, func, std::forward<Args>(args)...);
            }

            template <typename...Args>
            void logInfoFormatted(const std::string& format, Args&&...args) const
            {
                logFormatted(log_verbosity_t::LOG_LEVEL_INFO, format, std::forward<Args>(args)...);
            }

            template <typename...Args>
            void logInfoFormattedWithFunc(const char* func, const std::string& format, Args&&...args) const
            {
                logFormattedWithFunc(log_verbosity_t::LOG_LEVEL_INFO
                        , func
                        , format
                        , std::forward<Args>(args)...);
            }



            // Warning level

            template <typename T>
            void logWarning(string_t<T>&& msg) const
            {
                log(log_verbosity_t::LOG_LEVEL_WARNING, std::forward<T>(msg));
            }

            template <typename T>
            void logWarningWithFunc(const char* func, string_t<T>&& msg) const
            {
                logWithFunc(log_verbosity_t::LOG_LEVEL_WARNING, func, std::forward<T>(msg));
            }

            template <typename...Args>
            void logWarningArgsWithFunc(const char* func, Args&&...args) const
            {
                logArgsWithFunc(log_verbosity_t::LOG_LEVEL_WARNING
                        , func
                        , std::forward<Args>(args)...);
            }

            template <typename...Args>
            void logWarningFormatted(const std::string& format, Args&&...args) const
            {
                logFormatted(log_verbosity_t::LOG_LEVEL_WARNING, format, std::forward<Args>(args)...);
            }

            template <typename...Args>
            void logWarningFormattedWithFunc(const char* func, const std::string& format, Args&&...args) const
            {
                logFormattedWithFunc(log_verbosity_t::LOG_LEVEL_WARNING
                        , func
                        , format
                        , std::forward<Args>(args)...);
            }


            // Error level

            template <typename T>
            void logError(string_t<T>&& msg) const
            {
                log(log_verbosity_t::LOG_LEVEL_ERROR, std::forward<T>(msg));
            }

            template <typename T>
            void logErrorWithFunc(const char* func, string_t<T>&& msg) const
            {
                logWithFunc(log_verbosity_t::LOG_LEVEL_ERROR, func, std::forward<T>(msg));
            }

            template <typename...Args>
            void logErrorArgsWithFunc(const char* func, Args&&...args) const
            {
                logArgsWithFunc(log_verbosity_t::LOG_LEVEL_ERROR, func, std::forward<Args>(args)...);
            }

            template <typename...Args>
            void logErrorFormatted(const std::string& format, Args&&...args) const
            {
                logFormatted(log_verbosity_t::LOG_LEVEL_ERROR, format, std::forward<Args>(args)...);
            }

            template <typename...Args>
            void logErrorFormattedWithFunc(const char* func, const std::string& format, Args&&...args) const
            {
                logFormattedWithFunc(log_verbosity_t::LOG_LEVEL_ERROR
                        , func
                        , format
                        , std::forward<Args>(args)...);
            }


        private:

            /**
             * @brief Logging the given message
             *
             * @tparam T stringify type
             * @param verbosity The message verbosity level
             * @param msg The logging message
             */
            template <typename T>
            void log(log_verbosity_t verbosity, string_t<T>&& msg) const
            {
                if (m_pLogger)
                {
                    m_pLogger->log(verbosity, std::forward<T>(msg));
                }
            }

            /**
             * @brief Logging the message, prepend with the caller name
             *
             *
             * @tparam T The stringify type
             * @param verbosity The logging verbosity level
             * @param func The name of the caller function (__file__)
             * @param msg The logging message
             */
            template <typename T>
            void logWithFunc(log_verbosity_t verbosity, const char* func, string_t<T>&& msg) const
            {
                if (m_pLogger)
                {
                    std::ostringstream s;
                    s << "[" << func << "] " << std::forward<T>(msg);

                    m_pLogger->log(verbosity, s.str());
                }
            }

            struct GetOptional
            {
                std::string operator ()(const std::optional <std::string> &s) const
                {
                    if (!s)
                    { return std::string("<n/a>"); }
                    return *s;
                }
            };

            /**
             * @brief Logging the message arguments, prepend with
             * the caller name.
             * All arguments need to be string-like, or string convertable.
             * User-defined types need to provide java-like "toString()" public non-static method
             * @see str() function
             *
             * @note This is intended to use with appropriate macro
             *
             * @tparam Args The variadic arguments types
             * @param verbosity The logging verbosity level
             * @param func The caller name (__func__)
             * @param args The variadic arguments list, to be logged
             */
            template <typename...Args>
            void logArgsWithFunc(log_verbosity_t verbosity, const char* func, Args&&...args) const
            {
                if (m_pLogger)
                {
                    std::ostringstream s;
                    s << "[" << func << "] ";

                    if constexpr (sizeof...(args))
                    {
                        (s << ... << GetOptional{}(str(std::forward<Args>(args))));
                    }

                    m_pLogger->log(verbosity, s.str());
                }
            }

            /**
             * @brief Logging the formatted message
             *
             * @tparam Args The variadic arguments types
             * @param verbosity The logging verbosity level
             * @param format The message format
             * @param args The message arbitrary number of arguments
             */
            template <typename...Args>
            void logFormatted(log_verbosity_t verbosity, const std::string& format, Args&&...args) const
            {
                if (m_pLogger)
                {
                    m_pLogger->logFormatted(verbosity, format, std::forward<Args>(args)...);
                }
            }

            template <typename...Args>
            void logFormattedWithFunc(log_verbosity_t verbosity
                    , const char * func
                    , const std::string& format
                    , Args&&...args) const
            {
                if (m_pLogger)
                {
                    std::ostringstream s;
                    s << "[" << func << "] " << format;

                    m_pLogger->logFormatted(verbosity, s.str(), std::forward<Args>(args)...);
                }
            }

        private:

            std::unique_ptr<LoggerBase<logger_t>> m_pLogger;

    };//LoggerWrapper


/*
 * Helper macros, to capture the caller name (__func__)
 *
 * Client code needs to provide the internal reference to the logger,
 * implementing the getter method
 *      private:
 *          const logger_t & getLogger() const { return m_logger; }
 *
 */

#define TRACE_MSG(msg)                  getLogger().logTraceWithFunc(__func__, msg)
#define TRACE_FUNC()                    getLogger().logTraceArgsWithFunc(__func__)
#define TRACE_ARGS(...)                 getLogger().logTraceArgsWithFunc(__func__, __VA_ARGS__)
#define TRACE_FMT(format, ...)          getLogger().logTraceFormattedWithFunc(__func__, format, __VA_ARGS__)

#define DEBUG_MSG(msg)                  getLogger().logDebugWithFunc(__func__, msg)
#define DEBUG_FUNC()                    getLogger().logDebugArgsWithFunc(__func__)
#define DEBUG_ARGS(...)                 getLogger().logDebugArgsWithFunc(__func__, __VA_ARGS__)
#define DEBUG_FMT(format, ...)          getLogger().logDebugFormattedWithFunc(__func__, format, __VA_ARGS__)

#define INFO_MSG(msg)                   getLogger().logInfoWithFunc(__func__, msg)
#define INFO_FUNC()                     getLogger().logInfoArgsWithFunc(__func__)
#define INFO_ARGS(...)                  getLogger().logInfoArgsWithFunc(__func__, __VA_ARGS__)
#define INFO_FMT(format, ...)           getLogger().logInfoFormattedWithFunc(__func__, format, __VA_ARGS__)

#define WARN_MSG(msg)                   getLogger().logWarningWithFunc(__func__, msg)
#define WARN_FUNC()                     getLogger().logWarningArgsWithFunc(__func__)
#define WARN_ARGS(...)                  getLogger().logWarningArgsWithFunc(__func__, __VA_ARGS__)
#define WARN_FMT(format, ...)           getLogger().logWarningFormattedWithFunc(__func__, format, __VA_ARGS__)

#define ERROR_MSG(msg)                  getLogger().logErrorWithFunc(__func__, msg)
#define ERROR_FUNC()                    getLogger().logErrorArgsWithFunc(__func__)
#define ERROR_ARGS(...)                 getLogger().logErrorArgsWithFunc(__func__, __VA_ARGS__)
#define ERROR_FMT(format, ...)          getLogger().logErrorFormattedWithFunc(__func__, format, __VA_ARGS__)

}//namespace utils::log

#endif //LOGGERWRAPPER_H
