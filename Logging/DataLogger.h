/*
 * DataLogger.h
 *
 *  Created on: Feb 14, 2021
 *      Author: <a href="mailto:damirlj@yahoo.com">Damir Ljubic</a>
 */

#ifndef LOGGING_DATALOGGER_H_
#define LOGGING_DATALOGGER_H_

namespace utils::log
{
    template<class T>
    class ILoggerL
    {
        public:

            virtual ~ILoggerL() = default;
            virtual void log(const T& ) = 0;

        protected:

            ILoggerL() = default;
    };

    template<class T>
    class ILoggerR
    {
        public:

            virtual ~ILoggerR() = default;
            virtual void log(T&& ) = 0;

        protected:

            ILoggerR() = default;
    };
}



#endif /* LOGGING_DATALOGGER_H_ */
