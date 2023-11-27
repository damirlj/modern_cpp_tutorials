#include <iostream>
#include <string>
#include <memory>
#include <type_traits>


namespace details
{
    template <typename T>
    struct Factory final
    {
        /**
            Universal factory method
            Creates the std::unique_ptr<T> for an arbitrary arguments list
            Advantage: the c-tor of type T after specialization, can be changed - but 
            the factory method remains the same   
        */
        template <typename...Args>
        static std::unique_ptr<T> create(Args&&...args)
        {
            if constexpr(std::is_constructible_v<T, Args...>) // check weather T is constructible for a given arguments pack
            {
                return std::make_unique<T>(std::forward<Args>(args)...);
            }

            return nullptr;
            
        }
    };

} // namespace details


#define printFunc() std::cout << '\n' << __func__ << "():\n";


/*
    Bridge design pattern
    - structural design pattern
    - separates the Abstraction from the concreate Implementation, so that
      these two can vary independently, in two separate hierarchies.
      It can be seen as pimpl idiom, since it hides the implementation at client side.
*/


// Implementation common interface
struct IImplementation
{
    virtual ~IImplementation() = default;
    virtual void f() = 0;
};


// Implementation policy
// It's the way to inject the Dependency - implementation as policy, at compile time
template <typename Implementation>
struct Abstraction
{
    virtual ~Abstraction() = default;

    // Abstraction common interface
    virtual void g() = 0;
    virtual void h() = 0;

    explicit Abstraction(std::unique_ptr<Implementation> pimpl) noexcept : 
        m_pimpl(std::move(pimpl))
    {}

    template <typename...Args>
    explicit Abstraction(Args&&...args) noexcept: 
        m_pimpl(details::Factory<Implementation>::create(std::forward<Args>(args)...))
    {}

    protected:
        std::unique_ptr<Implementation> m_pimpl;
};

// Concreate implementations

struct A1 : IImplementation
{
    void f() override { puts("A1::f()");}
};


struct A2 : IImplementation
{
    explicit A2(int id) noexcept : m_id(id) {}
    
    void f() override { puts("A2::f()"); }
    int get() const { return m_id; }

    private:
       int m_id;
};

struct Client1 : public Abstraction<A1>
{
    using base = Abstraction<A1>;
    using base::base; 
    
    virtual ~Client1() override = default;

    void g() override 
    {
        printFunc();
        m_pimpl->f();
        puts("Client1::g()");
    }

    void h() override
    {
        printFunc();

        m_pimpl->f();

        puts("Client1::h()");
    }
};


struct Client2 : public Abstraction<A2>
{
    using base = Abstraction<A2>;
    using base::base; // base class c-tors
   
    virtual ~Client2() override = default;

    void g() override 
    {
        printFunc();

        m_pimpl->f(); // from implementation "borrowed" functionality
        
        puts("Client2::g()");
        std::cout << "id=" << m_pimpl->get() << '\n';
    }

    void h() override
    {
        printFunc();

        m_pimpl->f();

        puts("Client2::h()");
        std::cout << "id=" << m_pimpl->get() << '\n';
    }
};

int main()
{
    Client1 client1;
    client1.g();
    client1.h();


    Client2 client2(11);
    client2.g();
    client2.h();
}