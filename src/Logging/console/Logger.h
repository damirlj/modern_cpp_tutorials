//
// Created by Damir Ljubic (damirlj@yahoo.com) on 05.03.2021.
// Copyright (c) 2021. All rights reserved.
//

#ifndef LOGGING_LOGGER_H_
#define LOGGING_LOGGER_H_

#include <string>
#include <memory>
#include <stdexcept>

#include "commons.h"


namespace utils::log
{

    using log_verbosity_t = enum class LogVerbosity: uint8_t
    {
             LOG_LEVEL_TRACE
            ,LOG_LEVEL_DEBUG
            ,LOG_LEVEL_INFO
            ,LOG_LEVEL_WARNING
            ,LOG_LEVEL_ERROR
    };

    class Logger
    {
        public:

            virtual ~Logger() = default;

            virtual void log(log_verbosity_t verbosity, const std::string& msg) = 0;
            virtual void log(log_verbosity_t verbosity, std::string&& msg) = 0;

        protected:

            Logger() = default;
    };

    /**
     * @brief Extends the Logger base interface with the tag
     * This is android specific tracing, where tag can be defined
     * as a trace channel/subchannels - specifying the entire channel hierarchical tree
     *
     * @see LoggingMetadata : specifies the HCP3 specific logging format (tag + embedded into message
     * metadata)
     */
    class LoggerWithTag : public Logger
    {
        public:

            explicit LoggerWithTag(std::string tag) noexcept : Logger()
                , m_tag(std::move(tag))
            {}

            const std::string& tag() const
            {
                return m_tag;
            }

        private:

            const std::string m_tag;
    };



    /*
     * CRTP (Curiously Recurring Template Pattern)
     * https://www.fluentcpp.com/2017/05/12/curiously-recurring-template-pattern/
     *
     * Use the derived class implementation invoked at client side
     * through the base class interface
     */
    template<class LoggerImpl>
    class LoggerBase : public LoggerWithTag
    {
            /*
             * To prevent wrong template instantiation
             * class Derived1 : public Base<Derived1>{};//OK
             * class Derived2 : public Base<Derived1>{};//NOK: Derived2 is not the friend class (but rather Derived1)
             */
            friend LoggerImpl;
            explicit LoggerBase(std::string tag) noexcept : LoggerWithTag(tag)
            {}

        public:

            using impl_t = LoggerImpl;

            /**
             * @brief Since the LoggerBase c-tor is private, the
             * std::make_unique call will fail
             * Therefore, use factory class as substitution for std::make_unique.
             * It will rather return nullptr, than throw
             *
             * @param tag The tag that will be prepended to the message (like channel tree)
             * @return The std::unique_ptr on the base class interface, without exposing the derived
             * class implementation: it will be encapsulated into base class interface
             */
            template<typename...Args>
            static auto createLogger(std::string tag, Args&&...args) noexcept
            {
                return std::unique_ptr<LoggerBase<LoggerImpl>>(new (std::nothrow)
                            LoggerImpl{tag, std::forward<Args>(args)...}
                        );
            }

            // Logger interface implementation: lvalue reference
            void log(log_verbosity_t verbosity, const std::string& msg) override
            {
                impl().logImplWithTag(verbosity, msg);//Derived class specific implementation
            }

            // Logger interface implementation: rvalue reference
            void log(log_verbosity_t verbosity, std::string&& msg) override
            {
                impl().logImplWithTag(verbosity, std::move(msg));//Derived class specific implementation
            }

            /**
             * @brief Helper method for logging the message as a
             * formatted string
             *
             * @param verbosity The verbosity level
             * @param format The logging message format
             * @param args The parameter pack
             */
            template<typename...Args>
            void logFormatted(log_verbosity_t verbosity
                    , const std::string& format
                    , Args&&...args) const
            {
                try
                {
                    //In case of the wrong formatting, the string_format can throw an exception
                    auto msg = utils::string_format(format, std::forward<Args>(args)...);
                    impl().logImplWithTag(verbosity, std::move(msg));//Derived class specific log implementation
                }
                catch(const std::exception& e)
                {
                    (void)fprintf(stderr, "<Error> %s", e.what());//unit testing
                }
            }



        private:

            impl_t& impl() const//reference to the derived class specific implementation
            {
                return *static_cast<impl_t *>(const_cast<LoggerBase<impl_t>*>(this));
            }
    };//class LoggerBase

} /* namespace utils::log */

#endif /* LOGGING_LOGGER_H_ */
