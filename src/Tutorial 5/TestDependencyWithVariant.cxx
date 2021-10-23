/*
 * TestDependencyWithVariant.cxx
 *
 *  Created on: Oct 23, 2021
 *  Author: <a href="mailto:damirlj@yahoo.com">Damir Ljubic</a>
 */


#include "ClientWithVariant.hxx"
#include "TestCases.hxx"
#include "TestDependencyWithVariant.hxx"



namespace test::di::visitor
{
    /*
     * Function object, with overloaded set
     * that covers all the dependencies, and will be used
     * primarily with the std::visit call - visitor pattern
     */
    struct ServiceVisitor
    {
        void operator()(ConsoleService& s) const
        {
            s.provide(std::string("Divna"));
        }
        void operator()(LocatorService& s) const
        {
            s.coordinate(11.23f, 32.18f);
        }
    };

    int test()
    {

        ConsoleService service1;
        LocatorService service2("GLONASS");

        Client<ConsoleService, LocatorService> client ({service1, service2});

        // Similar as with std::get<std::size_t>: index of service

        client.call(ServiceVisitor{}, 0);
        client.call(ServiceVisitor{}, 1);

        // Similar as with std::get<T>: type of service

        client.call<LocatorService>([](LocatorService& locator)
                {
                    locator.coordinate(157.83f, 46.99f);
                });

        // To visit all services at once

        client.callAll(ServiceVisitor{});

        return 0;
    }
}
