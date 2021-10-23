/*
 * ClientWithVariant.hxx
 *
 *  Created on: Oct 23, 2021
 *  Author: <a href="mailto:damirlj@yahoo.com">Damir Ljubic</a>
 */

#ifndef DI_CLIENTWITHVARIANT_HXX_
#define DI_CLIENTWITHVARIANT_HXX_

#include <vector>
#include <variant>
#include <algorithm>
#include <stdexcept>

namespace test::di::visitor
{
    /**
     * For the complex clients that may depend on multiple dependents - services,
     * we can use type-safe union (std::variant) to hold dependencies: client as
     * dependency container.
     * Advantage: no polymorphism required - the services don't need to derived the
     * same base class (interface)
     * Disadvantage: you can't extend it, with the new services at runtime.
     *
     * <p>
     * We can use internally visitor pattern, calling overloaded function object,
     * with overloading resolution set that covers the invocation of all injected
     * service implementations.
     * We can also provide the function object that will be specific for the
     * particular dependency type
     *
     *
     * @tparam Services List of dependents
     */
    template <typename...Services>
    class Client final
    {
        public:
            using service_type = std::variant<Services...>;
            /*
             * For example:
             *
             * Client<Service1, Service2>::service_type would be deduced to
             * std::vector<std::variant<Service1, Service2>, std::variant<Service1, Service2>>
             * where first std::variant<Service1, Service2> is instantiated with Service1, and
             * the second one with Service2
             * This way we can use the homogenous container, to holds different types (no polymorphism
             * is required)
             *
             */
            using services_type = std::vector<service_type>;

            explicit Client(services_type services) noexcept :
                    m_services(std::move(services))
            {}

            /**
             * Provide for the particular service type
             * the callable object that will be invoked
             * on the injected service implementation
             *
             * @param func  Function object that will be invoked on the injected
             *              service implementation
             * @return      The return value of invocation, if any
             */
            template <typename Service, typename Function>
            decltype(auto) call(Function func)
            {
                if (auto it = std::find_if(m_services.begin(), m_services.end()
                        , [](const auto& service)
                        {
                            return std::holds_alternative<Service>(service);
                        }
                        ); it != m_services.end())
                {
                    return func(std::get<Service>(*it));
                }
                else
                {
                    throw std::logic_error("<DI> Non-existing service required!");
                }
            }

            template <typename Function>
            decltype(auto) call(Function func, std::size_t index)
            {
                if (index >= m_services.size()) throw std::out_of_range("Index out of range!");
                return std::visit(func, m_services[index]);
            }

            /**
             * Visitor pattern: visit all dependency by applying the
             * overloaded function object, with resolutions set that covers all
             * possible dependencies
             *
             * @tparam Function
             * @param func  Overloaded function object
             */
            template <typename Function>
            void callAll(Function func)
            {
                std::for_each(m_services.begin(), m_services.end(), [func](auto& service)
                        {
                            std::visit(func, service);
                        });
            }

        private:
            services_type m_services;
    };
}


#endif /* DI_CLIENTWITHVARIANT_HXX_ */
