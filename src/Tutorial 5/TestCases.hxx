/*
 * TestCases.hxx
 *
 *  Created on: Oct 23, 2021
 *  Author: <a href="mailto:damirlj@yahoo.com">Damir Ljubic</a>
 */

#ifndef DI_TESTCASES_HXX_
#define DI_TESTCASES_HXX_

#include <string>
#include <memory>
#include <iostream>

namespace test::di
{
    struct Service1
    {
          virtual ~Service1() = default;
          virtual void provide(const std::string& s) = 0;
        protected:
          Service1() = default;
    };


    struct ConsoleService final : public Service1
    {
       void provide(const std::string& s) override
       {
           std::cout << "Message: " << s << '\n';
       }
    };


    class Client1
    {
        public:
            explicit Client1(std::shared_ptr<Service1> service) : m_service(service)
            {}

            void operator()(const std::string& s) const
            {
                if (m_service) m_service->provide(s);
            }

        private:
            std::shared_ptr<Service1> m_service;
    };



    struct Service2
    {
         virtual ~Service2() = default;
         virtual void coordinate(float x, float y) = 0;
        protected:
         Service2() = default;
    };


    class LocatorService final : public Service2
    {
        public:
            explicit LocatorService(std::string satellite) noexcept :
                m_satellite(std::move(satellite))
            {}

            ~LocatorService() override = default;

            void coordinate(float x, float y) override
            {
                std::cout << "Satellite: " << m_satellite << '\n';
                std::cout << "width= " << x << ", height= " << y << '\n';
            }

        private:
            std::string m_satellite;

    };


    class Client2
    {
        public:
            Client2() = default;

            void setService(std::shared_ptr<Service2> locator)
            {
                m_service = locator;
            }

            void operator()(float x, float y) const
            {
                if (m_service)
                {
                    m_service->coordinate(x, y);
                }

            }
        private:
            std::shared_ptr<Service2> m_service = nullptr;

    };

}




#endif /* DI_TESTCASES_HXX_ */
