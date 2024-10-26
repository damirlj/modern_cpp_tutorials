#include <iostream>

#include "Publisher.hpp"
#include "Subscriber.hpp"

struct Int 
{
    constexpr Int(int val) noexcept: val_{val} {}
    constexpr operator int() const { return val_; }
    private:
        int val_;
};

int main()
{
    using namespace patterns::observer;
    using subscriber_t = Subscriber<Int>;

    auto subscriber = subscriber_t::create(
        [](const Int& update) { std::cout << update << '\n';},
        nullptr, 
        []{ std::cout << "Completed!\n"; }
        );
    
    using publisher_t = Publisher<Int, Subscriber>;
    std::shared_ptr<publisher_t> publisher = std::make_shared<publisher_t>();
    publisher->subscribe(subscriber);
    publisher->notify(Int{11});
    
    return 0;
}