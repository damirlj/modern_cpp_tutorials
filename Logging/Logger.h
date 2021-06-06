/*
 * Logger.h
 *
 *  Created on: Jan 31, 2021
 *      Author: <a href="mailto:damirlj@yahoo.com">Damir Ljubic</a>
 */

#ifndef LOGGING_LOGGER_H_
#define LOGGING_LOGGER_H_

#include <string>
#include <tuple>


namespace utils::log
{

	/**
	 * For logging parallel on a different logging mediums	
	*/
    template<class...Policies>
    class Logger final : public Policies...
    {
        public:

            using policy_t = std::tuple<Policies...>;

            Logger(): m_policies{Policies{}...}//default, parameterless c-tors
            {}

            void log(const std::string& msg)
            {
                for (auto i = 0; i < nPolicies; ++i)
                {
                    std::get<i>(m_policies).log(msg);
                }
            }

            void log(std::size_t policy, const std::string& msg)
            {
                if (policy >= nPolicies) throw std::out_of_range("Policy index out of range!");
                std::get<policy>(m_policies).log(msg);
            }

        private:

            inline static constexpr std::size_t nPolicies = sizeof...(Policies);
            policy_t m_policies;
    };
}


#endif /* LOGGING_LOGGER_H_ */
