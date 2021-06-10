//
// LoggerHelper.h
// Author: <a href="mailto:damirlj@yahoo.com">Damir Ljubic</a>
//

#ifndef AIRPLAYSERVICE_LOGGINGHELPER_H
#define AIRPLAYSERVICE_LOGGINGHELPER_H

#include <string>
#include <type_traits>
#include <optional>
#include <utility>

// Application
#include "../src/commons/Commons.h"

namespace utils::log
{
    template <typename T>
    constexpr bool is_string = std::is_same_v<std::string, std::decay_t<T>> ||
                               std::is_constructible_v<std::string, T> ||
                               std::is_convertible_v<T, std::string>;


    template <typename T, typename = std::enable_if_t<is_string<T>>>
    using string_t = T;

    /*
     * For java-like objects with public toString() method
     */
    template <class T, class=void>
    struct has_to_string : std::false_type {};
    template <class T>
    struct has_to_string<T, std::void_t<decltype(&T::toString)>> : std::is_same<std::string
            , decltype(std::declval<T&>().toString())> {};

    template <typename T>
    inline constexpr bool has_to_string_v = has_to_string<T>::value;

    /**
     * String conversion
     *
     * it will return the value, if the value itself is std::string convertible.
     * If it's numeric type, or enum class - std::to_string() conversion function will be called.
     * Otherwise, the empty string will be returned
     *
     */
    template <typename T>
    std::optional<std::string> str(T &&value)
    {
        if constexpr (is_string<T>) // for string or string-like types
        {
            return value;
        }
        else
        {
            if constexpr (std::is_arithmetic_v<std::decay_t<T>>) // for numerics: integral and floating-point arithmetic
            {
                return std::to_string(std::forward<T>(value));
            }
            else
            {
                if constexpr (std::is_enum_v<std::decay_t<T>>) // for enum (scoped) classes
                {
                    return std::to_string(utils::convertEnum(std::forward<T>(value)));
                }
                else
                {
                    if constexpr (has_to_string_v<std::decay_t<T>>) // for java-like types with toString() method
                    {
                        return std::forward<T>(value).toString();
                    }
                }
            }
        }
        return {};
    }
}
#endif //AIRPLAYSERVICE_LOGGINGHELPER_H
