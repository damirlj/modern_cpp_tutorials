/*
 * Commons.h
 *
 *  Created on: Dec 29, 2020
 *      Author: <a href="mailto:damirlj@yahoo.com">Damir Ljubic</a>
 */

#ifndef COMMONS_COMMONS_H_
#define COMMONS_COMMONS_H_

#include <iostream>
#include <utility>
#include <type_traits>
#include <exception>

namespace utils
{
    /**
     * For converting the stronged-named enumeration value (enum class)
     * into underlying type (enum class <name> : <underlying_type>)
     *
     * @tparam E    stronged-named enumeration type (enum class)
     * @param e     stonged-named enumeration value
     * @return      The underlying type (enum class <name> : <underlying_type>{};)
     */
    template<class E>
    constexpr auto toUType(E e) noexcept
    {
        return static_cast<std::underlying_type_t<E>>(e);
    }


    template<typename ... Args>
    std::string string_format( const std::string& format, Args&&...args )
    {
        const auto rv = std::snprintf( nullptr
                , 0
                , format.c_str()
                , std::forward<Args>(args)... ) + 1; // Extra space for '\0'

        if (rv <= 0 )
        {
            throw std::runtime_error( "Error while obtaining the size." );
        }

        const auto size = static_cast<std::size_t>(rv);

        std::string buffer (size, 0);
        if (std::snprintf(&buffer[0]
                , size
                , format.c_str()
                , std::forward<Args>(args)...) <= 0)
        {
            throw std::runtime_error( "Error while formatting." );
        }

        return buffer.erase(size-1, 1); // We don't want the '\0' inside
    }

}



#endif /* COMMONS_COMMONS_H_ */
