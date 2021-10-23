/*
 * DIContainer.hxx
 *
 *  Created on: Oct 23, 2021
 *  Author: <a href="mailto:damirlj@yahoo.com">Damir Ljubic</a>
 */

#ifndef DI_DICONTAINER_HXX_
#define DI_DICONTAINER_HXX_

// std library
#include <any>
#include <optional>
#include <typeinfo>
#include <typeindex>
#include <unordered_map>

// Application
#include "DIFactory.hxx"

namespace utils::di
{
    /**
     * IOC implementation
     * <p>
     * DI container as injector. It holds:
     * - dependency objects (services) in case that they will be shared between clients
     * - factories, in case that the client will rather holds the new instance of the dependency
     */
    class DIContainer final
    {
        public:

            using factories_map = std::unordered_map<std::type_index, std::any>;
            using instances_map = std::unordered_map<std::type_index, std::any>;

            ~DIContainer() = default;

            DIContainer(const DIContainer&) = delete;
            DIContainer& operator=(const DIContainer&) = delete;

            /**
             * DI container will be created through this gateway
             *
             * <p>
             * @note since the c-tor is private, we can't use std::make_unique
             * @note std::unique_ptr can be easily converted to the std::shared_ptr
             * to express different type of ownership. Other way around doesn't work
             *
             * @return The reference to the DI container
             */
            static std::unique_ptr<DIContainer> make_di_container()
            {
                return std::unique_ptr<DIContainer>(new (std::nothrow) DIContainer());
            }

            /**
             * Adding into container the factory method for creating the matching
             * dependencies
             *
             * @tparam DIService        Service interface
             * @tparam DIServiceImpl    Service concrete implementation
             * @tparam Args             Arbitrary argument types
             * @param args              Arguments to construct the concrete service implementation
             */
            template <typename DIService, typename DIServiceImpl, typename...Args>
            void add(Args&&...args);

            /**
             * Client must not be aware of the service implementation.
             * Therefore, at client side, the concrete service implementation will be retrieved,
             * based on the given template parameter: service interface type - its std::type_index
             *
             * @tparam DIService    Service interface
             * @param shared        Whether to retrieve the shared reference to the service, or
             *                      create a new instance
             * @return              The reference to the concrete service implementation
             */
            template <typename DIService>
            std::optional<std::shared_ptr<DIService>> get(bool shared);

        private:
            DIContainer() = default;

        private:
            factories_map m_diFactories;
            instances_map m_diServices;
    };


    template <typename DIService, typename DIServiceImpl, typename...Args>
    inline void DIContainer::add(Args&&...args)
    {
        using namespace std;

        const auto id = type_index(typeid(DIService));

        const auto it = m_diFactories.find(id);
        if (it != m_diFactories.end()) throw std::runtime_error("DI: Service factory already specified!");

        auto factory = make_factory<DIService, DIServiceImpl>(std::forward<Args>(args)...);
        if (!factory) throw std::runtime_error("DI: Service factory not created!");

        // Save instance - when shared instance is required

        m_diServices[id] = std::any(std::shared_ptr<DIService>(factory->create()));

        // Save service factory - when new instance is required

        m_diFactories[id] = std::any(std::shared_ptr<IFactory<DIService>>(std::move(factory)));
    }


    template <typename DIService>
    inline std::optional<std::shared_ptr<DIService>> DIContainer::get(bool shared)
    {
        using namespace std;

        // C++ "reflection": Service interface as key to find the associated implementation
        const auto id = type_index(typeid(DIService));

        /*
         * If the shared instance of the service implementation is required,
         * grab from the container the matching one
         */
        if (shared)
        {
            if (const auto it = m_diServices.find(id); it != m_diServices.end())
            {
                return std::any_cast<std::shared_ptr<DIService>>(it->second);
            }
            else
            {
                cerr << "DI: Service implementation not found!\n";
                return nullopt;
            }
        }

        // Otherwise, return the new instance of the service implementation

        if (const auto it = m_diFactories.find(id); it != m_diFactories.end())
        {
            const auto& factory = std::any_cast<const std::shared_ptr<IFactory<DIService>>&>(it->second);
            return factory->create();
        }
        else
        {
            cerr << "DI: Service factory not found!\n";
            return nullopt;
        }
    }
}


#endif /* DI_DICONTAINER_HXX_ */
