#pragma once

// Std library
#include <concepts>
#include <memory>
#include <vector>
#include <exception>
#include <algorithm>
#include <functional>


namespace patterns::observer
{
    // Observer concept - interface
    template <typename Update, template <class> class Observer>
    concept is_observer = requires(Observer<Update>& observer, const Update& update, std::exception_ptr e)
    {
        {observer.onNext(update)} -> std::same_as<void>;
        {observer.onError(e)} -> std::same_as<void>;
        {observer.onCompletion()} -> std::same_as<void>;       
    };

    /**
     * Publisher - the Observable that will emit the Updates to the
     * all subscribed Observers 
     * Push model - emits the updates as they arrive to all listeners
     */
    template <typename Update, template <class> typename Observer>
    requires is_observer<Update, Observer>
    class Publisher final
    {
        public:
            using observer_type = Observer<Update>;

            ~Publisher() 
            {
                completion();    
            }
            
            void subscribe(const std::shared_ptr<observer_type>& observer) 
            {
                std::lock_guard lock {lock_};
                observers_.push_back(observer);
            }

            void unsubscribe(const std::shared_ptr<observer_type>& observer) 
            {
                std::lock_guard lock {lock_};
                observers_.erase(std::remove(observers_.begin(), observers_.end(), observer), observers_.end());
            }

            void notify(const Update& update) 
            {
                push([&update](const std::shared_ptr<observer_type>& observer){
                    try 
                    {
                        observer->onNext(update);
                    }catch(...)
                    {
                        std::exception_ptr e = std::current_exception();
                        observer->onError(e);
                    }
                });
            }

            // Publisher seas to emit the updates down the stream
            void completion()
            {
                push([](const std::shared_ptr<observer_type>& observer){
                    try 
                    {
                        observer->onCompletion();
                    }catch(...)
                    {
                        std::exception_ptr e = std::current_exception();
                        observer->onError(e);
                    }
                });
            }

        private: 
            template <typename Func, typename...Args>
            inline void push(Func&& func, Args&&...args)
            {
                std::lock_guard lock {lock_};
                
                std::for_each(observers_.begin(), observers_.end(), 
                [func = std::forward<Func>(func), ...args = std::forward<Args>(args)](const auto& observer) 
                {
                    if (const auto& ptr = observer.lock(); ptr)[[likely]]
                    {
                        std::invoke(func, ptr, args...);
                    }
                });
            }
        private:

            mutable std::mutex lock_{};
            std::vector<std::weak_ptr<observer_type>> observers_;
    };

}