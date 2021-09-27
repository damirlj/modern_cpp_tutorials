/*
 * Tuples.hxx
 *
 *  Created on: Sep 25, 2021
 *      Author: <a href="mailto:damirlj@yahoo.com">Damir Ljubic</a>
 */

#ifndef TUPLES_TUPLES_HXX_
#define TUPLES_TUPLES_HXX_

#include <tuple>
#include <iostream>
#include <type_traits>
#include <iterator>

namespace test::tuples
{
    template <class Collection, class=void>
    struct is_iterable : std::false_type {};

    template <class Collection>
    struct is_iterable<Collection,
        std::void_t<decltype(std::declval<Collection&>().begin()),
                    decltype(std::declval<Collection&>().end())> > : std::true_type {};

    template <class T>
    static constexpr bool is_string_v = std::is_same_v<std::decay_t<T>, std::string> ||
                                        std::is_constructible_v<std::string, T> ||
                                        std::is_convertible_v<T, std::string>;


    template <typename T>
    void print(T&& el)
    {
        if constexpr (is_iterable<T>::value && !is_string_v<T>)
        {
            using value_t = std::decay_t<decltype(*el.begin())>;
            std::copy(el.cbegin(), el.cend(), std::ostream_iterator<value_t>(std::cout, "\n"));  
        }
        else
        {
            std::cout << el << '\n';
        }
    }



    template <class T, class = void>
    struct is_optional_t : std::false_type {};

    template <class T>
    struct is_optional_t<T
                         , std::void_t<decltype(T::valid),
                                       decltype(T::value)>> : std::true_type {};

    /**
     * The class wraps the parameters pack captured by references into std::tuple,
     * in order to optimize setting the values, by preventing repeating
     * the non-trivial assignment operations.
     */
    template <typename...Args>
    class Setter
    {
        public:

            explicit Setter(Args&...args): m_values(std::tie(args...))
            {}
            explicit Setter(std::tuple<Args&...>& values) : m_values(values)
            {}

            template <std::size_t I, typename Value>
            Setter& set(Value&& value)
            {

                if constexpr (I < N &&
                        std::is_constructible_v<
                        std::decay_t<decltype(std::get<I>(m_values))>, Value>)
                {
                    /*
                     * This is firstly meant for a non-trivial assignments, that are tedious to
                     * write by hand
                     *
                     */
                    auto& opt = std::get<I>(m_values);
                    if constexpr (is_optional_t<Value>::value)
                    {
                        // In case that there is no proper c-tor
                        opt.valid = true;
                        opt.value = std::forward<Value>(value).value;
                    }
                    else
                    {
                        opt = std::forward<Value>(value);
                    }
                }

                return *this;
            }

            // For tracing only
            void print2Console()
            {
                print2Console(std::make_index_sequence<N>{});
            }

        private:

            template <std::size_t...I>
            void print2Console(std::index_sequence<I...>)
            {
                ( print(std::get<I>(m_values)),...); //fold expression
            }


        private:

            std::tuple<Args&...> m_values;
            static constexpr size_t N = sizeof...(Args);

    };
}



#endif /* TUPLES_TUPLES_HXX_ */
