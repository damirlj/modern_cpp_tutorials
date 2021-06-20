/*
 * LoggingMerge.h
 *
 *  Created on: Jan 31, 2021
 *      Author: <a href="mailto:damirlj@yahoo.com">Damir Ljubic</a>
 */

#ifndef LOGGING_MERGE_H_
#define LOGGING_MERGE_H_

#include <string>
#include <tuple>

#include "Logger.h"
#include "LoggingHelper.h"


namespace utils::log
{

	/**
	 * For parallel logging on different logging mediums	
	*/
    template<class...Policies>
    class LoggingMerge final : public Policies...
    {
        public:

            using policy_t = std::tuple<Policies...>;

            LoggingMerge(): m_policies{Policies{}...}//default, parameterless c-tors
            {}

			template <typename T, string_t<T>>
            void log(log_verbosity_t verbosity, T&& msg)
            {
                for (auto i = 0; i < nPolicies; ++i)
                {
                    std::get<i>(m_policies).log(verbosity, std::forward<T>(msg));
                }
            }

			template <typename T, string_t<T>>
            void log(std::size_t policy, log_verbosity_t verbosity, T&& msg)
            {
                if (policy >= nPolicies) throw std::out_of_range("Policy index out of range!");
				
                std::get<policy>(m_policies).log(verbosity, std::forward<T>(msg));
            }

        private:

            inline static constexpr std::size_t nPolicies = sizeof...(Policies);
            policy_t m_policies;
    };
}


#endif /*LOGGING_MERGE_H_*/
