/*
 * DIFactory.hxx
 *
 *  Created on: Oct 23, 2021
 *  Author: <a href="mailto:damirlj@yahoo.com">Damir Ljubic</a>
 */

#ifndef DI_DIFACTORY_HXX_
#define DI_DIFACTORY_HXX_

#include <tuple>
#include <iostream>
#include <type_traits>

#include "Factory.hxx"

namespace utils::di
{
    template <typename T>
    struct IFactory
    {
            virtual ~IFactory() = default;
            virtual std::unique_ptr<T> create() = 0;
        protected:
            IFactory() = default;
    };



    template <typename DIServiceInterface, typename DIService, typename...Args>
    class DIFactory final : IFactory<DIServiceInterface>
    {
        static_assert(std::is_base_of_v<DIServiceInterface, DIService>, "DI: Invalid service implementation!");

        public:

            static decltype(auto) createFactory(Args&&...args)
            {
                return std::unique_ptr<IFactory<DIServiceInterface>>(new (std::nothrow)
                        DIFactory(std::forward<Args>(args)...));
            }

            /**
             * Factory method: creates the dependency object, a
             * Service that will be injected at client side.
             *
             * @return Reference to the service concrete implementation upcasted
             * to the matching interface
             *
             * @note template <class T> class A{ public: A(T* t);};
             * There is no hierarchy relationship between A<Base> and A<Derived> instances
             * (only between template parameters)
             */
            std::unique_ptr<DIServiceInterface> create() override
            {
                   return std::apply(
                        [](auto&&...args)
                        {
                            return factory<DIService>(std::forward<decltype(args)>(args)...);
                        }
                       , m_args);
            }

            ~DIFactory() override = default;

            // Copy-operations forbidden

            DIFactory(const DIFactory&) = delete;
            DIFactory& operator=(const DIFactory&) = delete;


        private:

            /**
             * For binding the arguments of dependency object - service creation,
             * with factory method
             *
             * <p>
             * The service factory will be placed into IOC container.
             * In case that expectation is that each call of the @see DIFactory#create
             * creates the new instance of the service implementation, to accomplish that,
             * the arguments for the factory function need to be stored into tuple
             *
             * @param args  Service implementation construction arguments
             */
             explicit DIFactory(Args&&...args) noexcept :
                m_args(std::make_tuple(std::forward<Args>(args)...))
            {}

        private:
            std::tuple<std::decay_t<Args>...> m_args; // store the arguments
    };

    template <typename DIServiceInterface, typename DIService, typename...Args>
    decltype(auto) make_factory(Args&&...args) noexcept
    {
        return DIFactory<DIServiceInterface, DIService, Args...>::createFactory(
                std::forward<Args>(args)...);
    }
}


#endif /* DI_DIFACTORY_HXX_ */
