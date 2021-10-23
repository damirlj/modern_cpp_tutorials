/*
 * TestDependencyInjection.cxx
 *
 *  Created on: Oct 23, 2021
 *  Author: <a href="mailto:damirlj@yahoo.com">Damir Ljubic</a>
 */

#include "DIContainer.hxx"
#include "TestCases.hxx"
#include "ClientWithVariant.hxx"
#include "TestDependencyInjection.hxx"




namespace test::di
{
    int testDI()
    {
        using namespace std;
        using namespace utils::di;

        auto diContainer = DIContainer::make_di_container();
        if (!diContainer) return 1;

        try
        {
            diContainer->add<Service1, ConsoleService>();
            diContainer->add<Service2, LocatorService>(std::string("GNSS"));
        }
        catch(const exception& e)
        {
            cerr << e.what() << '\n'; // unit test
            return 2;
        }


        // Client is only aware of the interfaces on which it depends - not the implementation behind

        // 1. Inject dependency at client side: through the <b>constructor</b>

        //const auto service = diContainer->get<Service1>(true); // shared instance
        const auto service1 = diContainer->get<Service1>(false); // new instance
        Client1 client1(service1.has_value() ? *service1 : nullptr);
        client1("Alex");

        // 2. Inject dependency at client side: through the <b>setter method</b>

        const auto service2 = diContainer->get<Service2>(false);
        Client2 client2;
        client2.setService(service2.has_value() ? *service2 : nullptr);
        client2(283.37f, 112.11f);

        /*
         * Example of injected the dependency into client
         * that stores them into typed-safe union(std::variant) and can
         * use the visitor pattern along with overloaded function object, or
         * can specify for each DO matching handler
         */
        test::di::visitor::Client<std::shared_ptr<Service1>, std::shared_ptr<Service2>> client3 ({*service1, *service2});
        client3.call<std::shared_ptr<Service2>>([](std::shared_ptr<Service2>& service){
            service->coordinate(106.34f, 89.21f);
        });

        return 0;
    }
}
