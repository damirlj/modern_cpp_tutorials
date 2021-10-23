/*
 * Factory.hxx
 *
 *  Created on: Oct 23, 2021
 *  Author: <a href="mailto:damirlj@yahoo.com">Damir Ljubic</a>
 */

#ifndef DI_FACTORY_HXX_
#define DI_FACTORY_HXX_

#include <memory>
#include <type_traits>


namespace utils::di
{
    /**
     * Universal factory
     *
     * @tparam Object: Object to be created
     */
    template <class Object, class...Args>
    std::unique_ptr<Object> factory(Args&&...args)
    {
        if constexpr (std::is_constructible_v<Object, Args...>)
        {
            return std::make_unique<Object>(std::forward<Args>(args)...);
        }
        else
        {
            return nullptr;
        }
    }
}

#endif /* DI_FACTORY_HXX_ */
