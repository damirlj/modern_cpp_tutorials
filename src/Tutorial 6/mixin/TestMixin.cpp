/*
 * TestMixin.cpp
 *
 *  Created on: Nov 6, 2021
 *  Author: <a href="mailto:damirlj@yahoo.com">Damir Ljubic</a>
 */

#include <thread>
#include <chrono>
#include <iostream>
#include <type_traits>
#include <string>

// Application includes
#include "../commons/Commons.h"
#include "TestMixin.h"


template <typename T, typename void_t=void>
struct has_to_string : std::false_type {};

/*
 * We check the two feature of the "java-like" class
 * 1) has non-static public method toString
 * 2) the required signature-return value of toString() method: std::string
 */
template <typename T>
struct has_to_string<T, std::void_t<decltype(T::toString)>> : std::is_same<std::string,
    decltype(std::declval<T>().toString())>
{};


template <typename T>
static constexpr bool has_to_string_v = has_to_string<T>::value;

#define CHECK_ENUM(x) case (x) : return #x


/**
 * ConsoleLogger as a Mixin class.
 * It adds additional value to the base class: logging to console.
 *
 * @tparam Super Base class type - parameterized inheritance.
 * There is no "is a" relationship, but rather it's the way to
 * extend the existing class with the additional-ortogonal features
 */
template <typename Super>
class ConsoleLogger : public Super
{
    public:
        /*
         * This is the way to inherit the base class c-tor overload resolution, especially
         * when mixin class has no data members, but rather enhance the
         * base class with additional functionality.
         * This is especially important when combining the more mixin classes (its features) into
         * single one
         */
        using Super::Super;

        template <typename...Args>
        void log(Args&&...args)
        {
            if constexpr (sizeof...(args) > 0) // print additional arguments, if any
            {
                std::cout << "[";
                (std::cout << ... << args); // binary left fold expression (Iop...opE)
                std::cout << "]\n";
            }
            /*
             * Print the base "java-like" class
             *
             * @note The use of "this" pointer is necessary since compiler
             * doesn't make any assumption about the names (methods) of the parameterized base
             * class.
             * The other way around is to use the full name scope - Super::toString()
             */
            if constexpr (has_to_string_v<Super>)
            {
                std::cout << this->toString() << '\n';
                //std::cout << Super::toString() << '\n';
            }
        }
};


/**
 * Mixin class that is non-default constructible
 * Enhanced the base class with timestamp feature
 *
 * @tparam Super Base class
 */
template <typename Super>
class TimeStamp : public Super
{
    public:

        /**
         * Mixin class which is not default-constructible
         *
         * @param format    The desirable timestamp format
         * @param args      Arbitrary argument list for constructing the base class
         */
        template <typename...Args>
        explicit TimeStamp(const std::string& format, Args&&...args) noexcept :
            Super(std::forward<Args>(args)...)
            , m_format(format)
        {}

        std::string operator()() const
        {
            using namespace std::chrono;
            const auto tp = system_clock::now();
            const auto t = system_clock::to_time_t(tp);

            constexpr auto length = 100;
            char timestamp [length] = {0};
            if (const auto size = std::strftime(timestamp
                        , length
                        , m_format.c_str()
                        , std::localtime(&t)
            ); size > 0)
            {
                timestamp[size] = '\0';
            }

            return timestamp;
        }

    private:
        std::string m_format;
};

/**
 * Mixin class.
 * Enhanced the base - super class with
 * benchmark feature, ability to measure elapsed time
 *
 * @tparam Super        Parameterized base class
 * @tparam Clock        The clock (monotonic by default)
 * @tparam Resolution   Clock resolution (milliseconds by default)
 */
template <typename Super
        , typename Clock = std::chrono::steady_clock
        , typename Resolution = std::chrono::milliseconds
        >
class ElapsedTime : public Super
{
    public:
        using Super::Super;

        void start()
        {
            m_startPoint = Clock::now();
        }

        Resolution stop() const
        {
            using namespace std::chrono;
            return duration_cast<Resolution>(Clock::now() - m_startPoint);
        }

    private:
        std::chrono::time_point<Clock> m_startPoint;
};

using gender_t = enum class Gender
{
    male,
    female
};

namespace
{
    std::string printGender(gender_t gender)
    {
        switch (gender)
        {
            CHECK_ENUM(gender_t::male);
            CHECK_ENUM(gender_t::female);
        }
        return "<n/a>";
    }
}


class Person
{
    public:
        Person(std::string name, int age, gender_t gender) noexcept :
            m_name(std::move(name))
            , m_age(age)
            , m_gender(gender)
        {}

        std::string toString() const
        {
            return utils::string_format("Name=%s, age=%d, gender=%s"
                    , m_name.c_str()
                    , m_age
                    , printGender(m_gender).c_str()
                    );
        }

    private:
        std::string m_name;
        int m_age;
        gender_t m_gender;
};

namespace tutorial::mixin
{
    template <typename T>
    using elapsed_time_logger_t = ElapsedTime<ConsoleLogger<T>>;

    void defaultConstructibleMixins()
    {
        using namespace std::chrono_literals;

        /*
        * The outermost mixin (ElapsedTime) is inherited the
        * c-tor overload resolution of the non-mixin: the innermost class - Person
        */

        elapsed_time_logger_t<Person> personLoggerWithElapsedTime {"Alex", 7, gender_t::male};

        personLoggerWithElapsedTime.start();
        std::this_thread::sleep_for(1.5s);
        const auto duration = personLoggerWithElapsedTime.stop();
        personLoggerWithElapsedTime.log("duration=", duration.count(), "[ms]");
    }


    template <typename T>
    using timestamp_logger_t = TimeStamp<ConsoleLogger<T>>;

    void nonDefaultConstructibleMixins()
    {
        timestamp_logger_t<Person> personLoggerWithTimestamp {"[%Y-%m-%d, %X]", "Divna", 41, gender_t::female};
        const auto timestamp = personLoggerWithTimestamp();
        personLoggerWithTimestamp.log(timestamp);
    }

    int testMixin()
    {

        defaultConstructibleMixins();
        nonDefaultConstructibleMixins();

        return 0;
    }
}
