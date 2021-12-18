/*
 * TestExternalPolymorphism.cxx
 *
 *  Created on: Dec 18, 2021
 *  Author: <a href="mailto:damirlj@yahoo.com">Damir Ljubic</a>
 */


#include "TestLogger.hxx"

#include "Logger.hxx"
#include "Type1.hxx"
#include "Type2.hxx"
#include "ConsoleLogger.hxx"


#include <initializer_list>
#include <memory>
#include <algorithm>



namespace test::ep
{
    static std::mutex lock{};
    static ConsoleLogger logger {lock};

    // Factory helper function
    template <typename T, typename Logger = ConsoleLogger>
    std::unique_ptr<Logging> create(const T& type, const Logger& logger)
    {
        return std::make_unique<LoggingImpl<T, Logger>>(type, logger);
    }

    /*
     * In your main() call:
     * return test::erasure::test();
     */
    int test()
    {
        Type1 type1 {11};
        Type2 type2 {"Alex", 7};

        /*
         * External Polymorphism:
         * Different - by inheritance unrelated types, treated polymorphically - in
         * the same way
         */
        using logging_type = std::unique_ptr<Logging>;


        /*
         * Quiz: can you call
         *      std::vector<logging_type> typesLogger {create(type1, logger), create(type2, logger)};
         * ?
         * The
         *      vector(std::initializer_list<T>);
         * overloaded version of c-tor will internally call std::uninitialized_copy<T> - the elements in list
         * must be copy-constructible.
         * @note We have move-only elements(std::unique_ptr<T>)!
         *
         * If you need dynamic array - vector, you can do something like:
         *
         * std::vector<logging_type> typesLogger;
         * typesLogger.reserve(2);
         * typesLogger.push_back(create(type1, logger)); // rvalue aware overloaded version
         * typesLogger.push_back(create(type2, logger)); // rvalue aware overloaded version
         */

        // For iterate over static collection, we can use std::initializer_list directly (or std::array)
        std::initializer_list<logging_type> typesLogger {create(type1, logger), create(type2, logger)};

        std::for_each(std::cbegin(typesLogger), std::cend(typesLogger)
                , [](const auto& typeLogger)
                {
                    typeLogger->log(); // unrelated type by inheritance (dynamic polymorphism) treated polymorphically - in the same manner
                });

        return 0;
    }
}

