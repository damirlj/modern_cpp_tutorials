//
// <author> damirlj@yahoo.com
// Copyright (c) 2021. All rights reserved!
//

#ifndef LOGGINGTAG_H
#define LOGGINGTAG_H

#include <sstream>
#include "LoggingHelper.h"



namespace utils::log
{

    /**
     * Adding the subchannels to the logging tag, as "(.subchannel)+" pattern
     * @tparam Ts string or anything from which string is constructible
     * @param subchannel The subchannels
     * @return Generated subchannels pattern
     *
    */
    template<typename...Ts>
    auto appendSubchannels(const string_t<Ts>&...subchannels)
    {
        static const std::string delm = ".";
        std::ostringstream s;

        (s << ... << (delm + subchannels));

        return s.str();
    }

    template<typename...Ts>
    auto generateLogTag(const std::string &app
            , const std::string &channel
            , const string_t<Ts>&...subchannels)
    {
        std::string tag = app + std::string(":") + channel;
        if (sizeof...(subchannels))
        {
            tag += appendSubchannels(subchannels...);
        }
        return tag;
    }
}
#endif //LOGGINGTAG_H
