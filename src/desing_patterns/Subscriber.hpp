#pragma once

// Std library
#include <functional>
#include <memory>
#include <exception>

namespace patterns::observer
{

    /**
     * Observer - listener to the Updates.
     * Example of implementation that relies on the CRTP design pattern provided by the 
     * std library: std::enable_shared_from_this<>.
     * This way we prevent that any member function creates the shared_ptr of
     * the enclosing class, will not create a new shared_ptr - with the new control block around the 
     * same underlying pointer "this", but rather references the up front created shared_ptr with the 
     * single control block - calling std::share_from_this()
    */
    template <typename Update>
    class Subscriber : public std::enable_shared_from_this<Subscriber<Update>>
    {
       
        public:
            
            using update_callback = std::function<void(const Update&)>;
            using error_callback = std::function<void(std::exception_ptr e)>;
            using completion_callback = std::function<void()>;

            /**
             * Gateway for factoring the enclosing class.
             * This is the entry point for factoring the instance of the class, as 
             * precondition for std::share_from_this() is a valid call that doesn't throw
            */  
            static std::shared_ptr<Subscriber> create(update_callback updateCallback, 
                error_callback errorCallback, 
                completion_callback completionCallback)
            {
                return std::shared_ptr<Subscriber>(new Subscriber(updateCallback, errorCallback, completionCallback));
            }

            // Common interface
            template <typename Publisher>
            void subsribe(std::shared_ptr<Publisher> publisher) 
            {
                if(publisher)[[likely]] publisher->subsribe(this->share_from_this());
            }

            template <typename Publisher>
            void unsubsribe(std::shared_ptr<Publisher> publisher) 
            {
                if(publisher)[[likely]] publisher->unsubsribe(this->share_from_this());
            }

            // Interface - customization points, aligned with the Observer concept
            void onNext(const Update& update) 
            {
                if (updateCallback_) std::invoke(updateCallback_, update);
            }   
            
            void onError(std::exception_ptr e) 
            {
                if (errorCallback_) std::invoke(errorCallback_, e);
            }

            void onCompletion() 
            {
                if (completionCallback_) std::invoke(completionCallback_);
            }

        private:
            /**
             * Hidden c-tor
            */
            explicit Subscriber(update_callback updateCallback, 
                                error_callback errorCallback, 
                                completion_callback completionCallback) noexcept:
               updateCallback_(updateCallback),
               errorCallback_(errorCallback),
               completionCallback_(completionCallback)
            {}
        private:
            update_callback updateCallback_;
            error_callback errorCallback_;
            completion_callback completionCallback_;

    };
}
