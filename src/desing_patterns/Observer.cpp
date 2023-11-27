#include <functional>
#include <vector>
#include <string>
#include <cstdint>
#include <iostream>
#include <memory>
#include <algorithm>
#include <utility>
#include <exception>


// https://godbolt.org/z/71sacx7Gr

namespace details
{ 
    
    struct DynamicStorage
    {
        template <typename T, typename...Args>
        requires std::is_constructible_v<T, Args...>
        T* allocate(Args&&...args) const {
            return new T(std::forward<Args>(args)...);
        }

        void deallocate(auto* ptr) const {
            return delete(ptr);
        }
    };

    template <std::size_t Capacity, std::size_t Allignment>
    struct StackStorage
    {
        template <typename T, typename...Args>
        T* allocate(Args&&...args)  {
            static_assert(sizeof(T) <= Capacity, "Storage capacity exceeded");
            return std::construct_at(reinterpret_cast<T*>(buffer_.data()), std::forward<Args>(args)...);
        }

        
        void deallocate(auto* ptr) {
            if (ptr) std::destroy_at(ptr);
        }

      private:
        alignas(Allignment) std::array<std::byte, Capacity> buffer_;
    };

    template <class Allocator>
    struct Deleter 
    {
        using allocator_type = std::remove_cvref_t<Allocator>;

        explicit Deleter(allocator_type& allocator) noexcept: allocator_(allocator)
        {}

        void operator()(auto* ptr) {
            allocator_.deallocate(ptr);
        }
       private:
            Allocator& allocator_; // storage needs to outlive deleter
    };

    template<typename T,  class Allocator>
    using unique_ptr = std::unique_ptr<T, Deleter<Allocator>>;

    template <typename T, class Allocator, typename...Args>
    [[nodiscard]] auto make_unique(Allocator& allocator, Args&&...args) {
        if constexpr (std::is_constructible_v<T, Args...>) {
            return details::unique_ptr<T, Allocator>(allocator.template allocate<T, Args...>(std::forward<Args>(args)...), Deleter<Allocator>{allocator});
        }
        return details::unique_ptr<T, Allocator>{nullptr, Deleter<Allocator>{allocator}};
    }
}// namespace details


//For declaring the callable that can be invoked with both, 
//lvalue and rvalue reference argument, something like supporting the 
//both signatures at the same time
// - std::function<void(const auto& )>
// - std::function<void(auto&&)>
template <typename T, typename Func>
requires std::invocable<Func, T>
struct Consumer {

    explicit Consumer(Func&& func) noexcept(std::is_nothrow_copy_constructible_v<Func> || std::is_nothrow_move_constructible_v<Func>)
        : func_(std::forward<Func>(func)) {}

    void apply(const T& arg) {
        applyImpl(arg);
    }

    void apply(T&& arg) {
        applyImpl(std::move(arg));
    }

private:
    template <typename U>
    requires std::convertible_to<U, T>
    void applyImpl(U&& u) { 
       std::invoke(func_, std::forward<U>(u));
    }

    std::decay_t<Func> func_;
};



template <typename T, typename Storage, typename Error = std::exception_ptr>
struct Observer {
    
    using value_type = T;
    using error_type = Error;

    
    template <typename Func>
    using onNextCallback = Consumer<value_type, Func>;
    using onErrorCallback = std::function<void(const error_type&)>;
    using onCompletionCallback = std::function<void()>;

    private:
    // Type erasure
    struct IObserver {
       virtual void onNext(const value_type&) = 0;
       virtual void onNext(value_type&&) = 0;
       virtual void onError(error_type ) = 0;
       virtual void onCompletion() = 0;
       virtual ~IObserver() = default;
      
       using unique_ptr= details::unique_ptr<IObserver, Storage>;
       virtual unique_ptr clone(Storage& storage) const = 0;
    };

    template <typename Func>
    struct ObserverImpl final : IObserver {

      explicit ObserverImpl(Func&& func, onErrorCallback errorCallback, onCompletionCallback completionCallback) noexcept : 
          nextCallback_(std::forward<Func>(func)),
          errorCallback_(errorCallback),
          completionCallback_(completionCallback)
      {}
      explicit ObserverImpl(Func&& func) noexcept: ObserverImpl(std::forward<Func>(func), nullptr, nullptr)
      {}
      explicit ObserverImpl(Func&& func, onErrorCallback errorCallback) noexcept: ObserverImpl(std::forward<Func>(func), errorCallback, nullptr) 
      {}

      ~ObserverImpl() override = default;

      void onNext(const value_type& value) override {
          nextCallback_.apply(value);
      }
      void onNext(value_type&& value) override {
          nextCallback_.apply(std::move(value));
      }

      void onError(error_type error) override {// optional
           if (errorCallback_) std::invoke(errorCallback_, error);
      }

      void onCompletion() override {// optional
           if (completionCallback_) completionCallback_();
      }

      IObserver::unique_ptr clone(Storage& storage) const override { 
        return details::make_unique<ObserverImpl>(storage, *this);
      }

      private:
         onNextCallback<std::decay_t<Func>> nextCallback_;
         onErrorCallback errorCallback_;
         onCompletionCallback completionCallback_;
       
    };// ObserverImpl
    
    Storage storage_{};
    IObserver::unique_ptr observer_;//pimpl idiom

    template <typename O, typename...Args>
    requires std::is_base_of_v<IObserver, O>
    auto make_unique(Args&&...args) {
        return details::make_unique<O, Storage>(storage_, std::forward<Args>(args)...);
    }

    public:
    
    //Templated C-tor: customization point
    template <typename Func>
    explicit Observer(Func&& func) noexcept: observer_(make_unique<ObserverImpl<Func>>(std::forward<Func>(func)))
    {}

    template <typename Func>
    explicit Observer(Func&& func, onErrorCallback errorCallback ) noexcept : 
    observer_(make_unique<ObserverImpl<Func>>(std::forward<Func>(func), errorCallback))
    {}
    
    template <typename Func>
    explicit Observer(Func&& func, onErrorCallback errorCallback, onCompletionCallback completionCallback ) noexcept : 
    observer_(make_unique<ObserverImpl<Func>>(std::forward<Func>(func), errorCallback, completionCallback))
    {}

    //  Copy functions: to support value semantic
    Observer(const Observer& other): observer_(other.observer_->clone(storage_)) {}
    Observer& operator = (const Observer& other) {
        Observer tmp(other);
        tmp.observer_.swap(observer_);
        return *this;
    }

    // Move functions - defaulted
    Observer(Observer&& ) noexcept = default;
    Observer& operator = (Observer&& ) noexcept = default;

    //NVI: onNext method that support both, lvalue & rvalue arguments
    // @see Consumer<T> implementation
    template <typename U>
    requires std::is_base_of_v<T, U>  || std::is_constructible_v<T, U> || std::is_convertible_v<U, T>
    void onNext(U&& update){
        observer_->onNext(std::forward<U>(update));
    }

    void onError(error_type error) {
        observer_->onError(error);
    }

    void onCompletion() {
        observer_->onCompletion();
    }
};

template <typename Observer>
class Observable {
 public:
    using observer_type = std::decay_t<Observer>;
   /**
      Subscribe the observer, by meaning of the callable
      that will be invoked each time when there is something to be 
      emitted.
      @note In order to utilize on RVO, the return value must not be discarded
   */
   [[nodiscard]] std::shared_ptr<observer_type> subscribe(const observer_type& observer) {
       auto subscription = std::make_shared<observer_type>(observer);
       observers_.push_back(subscription);
       return subscription;   
    }
 private:
    //template <typename Item>
    //using notify_f = void (observer_type::*)(Item&&);

    template <typename T, typename Func>
    void notifyImpl(T&& item, Func&& func) {
        using std::cbegin;
        using std::cend;

        auto notify = [item = std::forward<T>(item), func = std::forward<Func>(func)](const auto& observer) {
            if (auto ptr = observer.lock()){
                std::invoke(func, *ptr, item);
            }
        };
        std::for_each(cbegin(observers_),
                      cend(observers_),
                      notify);
    }
 public:
    template <typename T>
    requires std::is_base_of_v<typename observer_type::value_type, T> || std::convertible_to<T, typename observer_type::value_type>
    void notify(T&& value) {
        auto dispatch = [](observer_type& observer, auto&& val)
        { 
            using U = std::decay_t<decltype(val)>;
            
            if constexpr(std::is_rvalue_reference_v<U>){
                observer.onNext(std::forward<U>(val));
                return;
            }
            observer.onNext(val);
        };
        notifyImpl(std::forward<T>(value), dispatch);
    }

    void notifyError(typename observer_type::error_type error) {
        notifyImpl(error, &observer_type::onError);
    }

    void notifyCompletion() {
        notifyImpl(&observer_type::onCompletion);
    }

 private:
   std::vector<std::weak_ptr<observer_type>> observers_;
};

//Operators: map example

template <typename T, typename Observer, typename Func>
requires std::invocable<Func, T>
auto map(Func&& func) {
    // New - transformed observable (Decorator pattern - kind of)
    struct MappedObservable : Observable<Observer>{
       explicit MappedObservable(Func&& func) noexcept :
           func_(std::forward<Func>(func)){}
       
       using value_type = std::decay_t<T>;
       void notify(const value_type& arg) {// "decorate" base-class implementation by applying transformation function
         try {
            auto&& value = std::invoke(func_, arg);
            Observable<Observer>::notify(std::forward<decltype(value)>(value));
         } catch(...) {
            const auto error = std::current_exception();
            Observable<Observer>::notifyError(error);//you can't have uniform, overloaded notify(error) method, since predominant function template in overloading set
         }
         
       }
       private:
           
           Func func_;
        
    };

    return MappedObservable{std::forward<Func>(func)};
}


// Subject to observe: one can use the strong type for fields
struct Person {
    std::string name_;
    std::string address_;
    std::uint8_t age_;
};

std::ostream& operator << (std::ostream& os, const Person& person) {
    return os << "Name: " << person.name_<< ", address: " << person.address_ << ", age: " << (int)person.age_ << '\n';
}



int main()
{
   // Alias as customization point for storage
   // using Storage = details::DynamicStorage;
   using Storage = details::StackStorage<128u, alignof(void*)>;

   using PersonObserver = Observer<Person, Storage>;
   using PersonObservable = Observable<PersonObserver>;
   PersonObservable personObservable;
   
   auto subscription = personObservable.subscribe(PersonObserver{[](const auto& person){std::cout << person;}});
   //to unsubscribe
   //if (subscription) subscription.reset();
   const auto alex = Person{.name_="Alex", .address_="Sunset 1", .age_=8};
   personObservable.notify(alex);

   // Operators: map - create the observable based on the given transformation function
   using NameObserver = Observer<std::string, Storage>;
   auto personNameObservable = map<Person,NameObserver>([](const Person& person){
    return person.name_;
   });

   auto nameSubscription = personNameObservable.subscribe(
    NameObserver{[](const auto& name){std::cout << "Person->Name: " << name << '\n';},
                 [](auto error){std::rethrow_exception(error);}});

   personNameObservable.notify(alex);

}