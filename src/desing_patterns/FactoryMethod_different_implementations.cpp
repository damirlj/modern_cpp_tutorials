#include <memory>
#include <iostream>
#include <type_traits>
#include <cstddef>
#include <array>


// https://godbolt.org/z/x99Y61P73

namespace details 
{
    // Universal factory method
    // It will create any type of T on the heap: returning the std::unique_ptr so that client code takes the ownership
    template <typename T, typename...Args>
    auto create(Args&&...args) {
        if constexpr (std::constructible_from<T, Args...>) {
            return std::make_unique<T>(std::forward<Args>(args)...);
        } else {
            return nullptr;
        }
    }
}
 
 // 1. Dynamic polymorphism
 // The standard - textbook example is to have a single method witch switch-case enumeration, as indication which 
 // type to produce. This is not only wrong, but it's error-prone as well.
 // The fact is - the factory method shouldn't be aware of concreate types it fabricates: it should
 // only specify the mechanism (like allocation policy) behind

namespace factory::dynamic 
{
    using dynamic_types = enum class DynamicTypes {
        Derived1,
        Derived2,
        Derived3
    };
    struct Derived1 {};
    struct Derived2 {};
    struct Derived3 {};

    struct Base {

    template <typename T, typename...Args>
    requires std::is_base_of_v<Base, T>
    static auto create(dynamic_types type, Args&&...args) {
        using enum dynamic_types;
        // This violates the Open-Closed principle
        switch (type) {
            case Derived1 : return details::create<Derived1>(std::forward<Args>(args)...);
            case Derived2 : return details::create<Derived2>(std::forward<Args>(args)...);
            // It's also error-prone
            //case Derived3 : return details::create<Derived2>(std::forward<Args>(args)...);
            case Derived3 : return details::create<Derived3>(std::forward<Args>(args)...);
            default: break; // what if there is a new switch/type is required - and introduced in enum?
        }
        return nullptr;
    }
 };
}

 // 2. Static polymorphism: CRTP
// It's fair to assume that the famility types for which we want to specify the Factory Method is
// already known at compile time
namespace factory::crtp 
{
    template <typename Derived>
    class Base {
        Base() = default;
        friend Derived;

        public:
                // Factory method: as a single gateway to produce any derived implementation of the 
                // Base class interface.
                template <typename...Args>
                static auto create(Args&&...args) {
                    return details::create<Derived>(std::forward<Args>(args)...);
                    //return Derived::create(std::forward<Args>(args)...);
                }

                void doSomething() const {
                    impl().doSomethingImpl();
                }
        private:

            Derived& impl() { return static_cast<Derived&>(*this);}
            const Derived& impl() const {return static_cast<const Derived&>(*this);}
    };

    struct Derived1 : public Base<Derived1> {
        //private: Derived1() = default;
        public:
        // static std::unique_ptr<Base> create() {
        //     return details::create<Derived1>();
        // }
        void doSomethingImpl() const {
            std::cout << __func__ << "(): Inside Derived_1\n";
        }
    };


    struct Derived2 : public Base<Derived2> 
    {
    
        //private : explicit Derived2(int id) noexcept : id_(id) {}
        public:    
        explicit Derived2(int id) noexcept : id_(id) {}
        // static std::unique_ptr<Base> create(int id) {
        //     return std::unique_ptr<Derived2>(new Derived2(id));
        // }
        void doSomethingImpl() const {
            std::cout << __func__ << "(): Inside Derived_2, id= " << id_ << '\n';
        }
        private:
            int id_;
    };
} // factory::crtp 


namespace factory::conc 
{

 // 3. Using the Concept for specifying interface as a template parameter constraint
 // Each type which implements the interface can be generated with the given factory method

    using mode_type = enum class Mode {
        drive_forward,
        drive_backward,
        parked,
        neutral
    };

    template <typename Vehicle>
    concept is_vehicle = requires(Vehicle& v, const mode_type mode ) {
        {v.startEngine()} -> std::same_as<void>;
        {v.drive(mode)} -> std::same_as<void>;
        {v.breaking()} -> std::same_as<bool>;
        {v.stopEngine()} -> std::same_as<void>;
    };

    // To vehicles restricted factory method as free function.
    // It imposes restriction to the all vehicles that implement the very same interface: that are having the same 
    // behavior without being polymorphic by inheritance
    template <is_vehicle V, typename...Args>
    [[nodiscard]] auto createVehicle(Args&&...args) {
        return details::create<V>(std::forward<Args>(args)...);
    }

    // Simulation of using the stack allocation policy
    // One can write more generic factory method that will be used allocation policy and choose 
    // between heap and stack allocation strategy
    namespace stack_alloc
    {
        struct StackDeleter 
        {
                // Call operator
            void operator ()(auto* ptr) const {if (ptr)[[likely]] std::destroy_at(ptr);}
        };
        
        template <typename T>
        using unique_ptr = std::unique_ptr<T, StackDeleter>;

        template <typename T, typename Buffer, typename...Args>
        [[nodiscard]] auto create(Buffer& buffer, Args&&...args) {
            auto* ptr = std::construct_at(reinterpret_cast<T*>(buffer.data()), std::forward<Args>(args)...); 
            return unique_ptr<T>(ptr, StackDeleter{});
        }    
    }
    
    template <is_vehicle V, typename Buffer, typename...Args>
    [[nodiscard]] auto createVehicleStack(Buffer& buffer, Args&&...args) {
        return stack_alloc::create<V>(buffer, std::forward<Args>(args)...);
    }

    

    struct Auto {

        void startEngine() {
            std::cout << __func__ << "(): Start auto...\n";
        }

        void drive(mode_type mode) {
            std::cout << __func__ << "(): Drive auto in mode: " << static_cast<int>(mode) << '\n';
        }

        bool breaking() {
            std::cout << __func__ << "(): Breaking auto!\n";
            return true;
        }

        void stopEngine() {
            std::cout << __func__ << "(): Auto is stopped\n";
        }
    };

    using engine_type = enum class Engine {
        disel = 1 << 0,
        gasoline = 1 << 1,
        gas = 1 << 2,
        electric = 1 << 3,
        hydro = 1 << 4
    };

    #define CHECK(X) case (X) : return #X
    constexpr std::string_view printEngine(engine_type engine) noexcept {
        using namespace std::literals;
        using enum engine_type;
        switch(engine) {
            CHECK(disel);
            CHECK(gasoline);
            CHECK(gas);
            CHECK(electric);
            CHECK(hydro);
        }
        return "<n/a>"sv;
    }

    struct Truck {

        explicit Truck(std::string_view model, engine_type engine) : model_(model), engine_(engine) {}
        void startEngine() {
            std::cout << __func__ << "(): Start truck (" << model_ << "), engine=" << printEngine(engine_) << '\n';
        }

        void drive(mode_type mode) {
            std::cout << __func__ << "(): Drive truck in mode: " << static_cast<int>(mode) << '\n';
        }

        bool breaking() {
            std::cout << __func__ << "(): Breaking truck!\n";
            return true;
        }

        void stopEngine() {
            std::cout << __func__ << "(): Truck is stopped\n";
        }
        private: 
        std::string model_;
        engine_type engine_;
    };
} // factory::conc


// Unit tests
void test_CRTP_factory_method() {
    using namespace factory::crtp;
    // 2. Test Static Polymorphism approach: CRTP pattern
    std::cout << __func__ << "():\n";
    
    const auto base = Base<Derived1>::create();
    base->doSomething();
    const auto base2 = Base<Derived2>::create(2);
    base2->doSomething();
}

void test_Concept_approach() {
    using namespace factory::conc;
    
    // 3. Test Concept approach: as C++20 replacement for CRTP pattern
    std::cout << __func__ << "():\n";

    auto pAuto = createVehicle<Auto>();
    pAuto->startEngine();
    
    using namespace std::literals;
    auto pTruck = createVehicle<Truck>("MAN"sv, engine_type::disel);
    pTruck->startEngine();
}

void test_Concept_stack_alloc() {
    using namespace factory::conc;
    
    std::cout << __func__ << "():\n";

    static std::array<std::byte, 1024> buffer = {std::byte(0)};
    
    {
        auto pAuto = createVehicleStack<Auto>(buffer);
        pAuto->startEngine();
    }
    
    {
        using namespace std::literals;

        auto pTruck = createVehicleStack<Truck>(buffer, "Mercedes"sv, engine_type::electric);
        pTruck->startEngine();
    }
    
}

int main() {

    test_CRTP_factory_method();
    test_Concept_approach();
    test_Concept_stack_alloc();
    
}
