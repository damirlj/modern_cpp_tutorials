/*
 * ElapsedTime.h
 *
 *  Created on: Jan 7, 2021
 *      Author: <a href="mailto:damirlj@yahoo.com">Damir Ljubic</a>
 */

#ifndef MEASURING_ELAPSEDTIME_H_
#define MEASURING_ELAPSEDTIME_H_

#include <chrono>
#include <utility>

namespace utils::measure
{
    /**
     * Helper class for benchmarking: measuring the elapsed time
     *
     * @tparam Clock    Type of the clock
     * @tparam Duration Type of the clock duration
     */
    template <class Clock=std::chrono::steady_clock
            , class Duration = std::chrono::milliseconds>
    class ElapsedTime final
    {
        public:

            void start() noexcept
            {
                m_tp = Clock::now();
            }

            auto stop() const noexcept
            {
                return std::chrono::duration_cast<Duration>(Clock::now() - m_tp).count();
            }

        private:
            std::chrono::time_point<Clock> m_tp;
    };


    template <class Clock = std::chrono::steady_clock
            , class Duration = std::chrono::milliseconds>
    class Measure final
    {
            class ScopeElapsedTime final
            {
                public:

                    explicit ScopeElapsedTime(Duration& duration) : m_duration(duration)
                    {}

                    ~ScopeElapsedTime()
                    {
                        using namespace std::chrono;
                        m_duration = duration_cast<Duration>(Clock::now() - m_tp);
                    }

                private:

                    Duration& m_duration;
                    const std::chrono::time_point<Clock> m_tp = Clock::now();
            };

        public:


            template <typename Func, typename...Args>
            static decltype(auto) elapsedTime(Duration& duration, Func&& func, Args&&...args)
            {
                ScopeElapsedTime time {duration};
                //return std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);
                return std::forward<Func>(func)(std::forward<Args>(args)...);
            }


            template <class T, typename R, typename...Args>
            static decltype(auto) elapsedTime(Duration& duration
                    , T&& obj
                    , R T::* func
                    , Args&&...args)
            {
                ScopeElapsedTime time {duration};
                //return std::invoke(func, std::forward<T>(obj), std::forward<Args>(args)...);
                return (std::forward<T>(obj).*func)(std::forward<Args>(args)...);
            }

            template <class T, typename R, typename...Args>
            static decltype(auto) elapsedTime(Duration& duration
                    , T* ptr
                    , R T::* func
                    , Args&&...args)
            {
                ScopeElapsedTime time {duration};
                //return std::invoke(func, ptr, std::forward<Args>(args)...);
                return (ptr->*func)(std::forward<Args>(args)...);
            }
    };

}//namespace utils::measure




#endif /* MEASURING_ELAPSEDTIME_H_ */
