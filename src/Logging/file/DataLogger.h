//
// <author> damirlj@yahoo.com
// Copyright (c) 2021. All rights reserved!
//

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

}//namespace: utils::log



#endif /* LOGGING_DATALOGGER_H_ */
