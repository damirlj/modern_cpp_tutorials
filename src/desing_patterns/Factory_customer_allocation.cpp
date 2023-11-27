#include <iostream>
#include <array>
#include <memory>
#include <memory_resource>


#define fw(arg) std::forward<decltype(arg)>(arg)
#define log_function() std::cout << __func__ << "():\n"

namespace details
{

    /**
     * Generic Factory method
     *
     * Allocates the memory based on the given allocation policy (strategy)
     * and creates the std::unique_ptr<T, Deleter>, embedding the proper deleter into resulting type
     *
     *@tparam T - Element to create
     *@tparam Allocator - Allocation strategy to apply
    */
    template <typename T, template <class> class Allocator  = std::allocator>
    struct Factory
    {
        using allocator_type = Allocator<T>;

        explicit Factory(allocator_type& allocator) noexcept : m_allocator(allocator) 
        {}

        explicit Factory(allocator_type&& allocator) noexcept : m_allocator(std::move(allocator)) 
        {}

        
        template <typename...Args>
        auto create(Args&&...args)
        {
            //static_assert(std::is_constructible_v<T, Args...>, "<Factory> Invalid arguments pack, or c-tor is not accessible");
            
            // Allocate memory
            auto* ptr = m_allocator.allocate(sizeof(T));
            
            // Construct the object of type T - T::T(Args...) needs to be accessible
            new(ptr)T(std::forward<Args>(args)...); // placement new

            // Embed deleter into resulting type
            auto deleter = [alloc = m_allocator](T* ptr) mutable
            {
                 if (ptr)
                 {
                     ptr->~T(); // @note: call destructor explicitly, since we use placement new
                     std::cout << "<Factory> Deallocate ptr:" << ptr << '\n';
                     alloc.deallocate(ptr, sizeof(*ptr));
                 } 
            };

            return std::unique_ptr<T, decltype(deleter)> (ptr, deleter);
            
        }
       
        private:
            allocator_type m_allocator;
    };
    
    // CTAD - Class Template Argument Deduction rule
    template <typename T, template <class> class Allocator>
    Factory(Allocator<T>) -> Factory<T, Allocator>;

} // namespace: details


class A 
{
    int m_id;

    
   protected:
    // Restrict the creation of the class through the "virtual" c-tor - @see A::create()
     explicit A(int id) noexcept: m_id(id) {}

    public:

        // Factory class as a friend, since c-tor A::A(int) is protected
        template <typename T, template <class> class Allocator>
        friend class details::Factory;

     
        // Factory method of the class that depends on the allocation policy.
        // Unlike the std::vector, the allocator is not part of the type identity
        // Convert internaly to std::shared_ptr<A>, since it's implicitly convertable from std::unique_ptr.
        // This way, the types created with different allocation policy are interchangeable - the same
        template <typename Factory, typename...Args>
        static auto create(Factory&& factory, Args&&...args)
        {
            return static_cast<std::shared_ptr<A>>(fw(factory).create(fw(args)...));
        }

        virtual ~A()
        {
            std::cout << "A::~A(), id=" << m_id << '\n';
        }
        
        virtual void doSomething()
        {
            std::puts("A::doSomething()");
        }

        friend std::ostream& operator<<(std::ostream& out, const A& a) 
        {
            return out << "id=" << a.m_id << '\n';
        }
};


struct B final : public A
{
    using A::A; // use parent class c-tor(s)

    ~B() override 
    {
       std::puts("B::~B()");
    }

    void doSomething() override
    {
        std::puts("B::doSomething()");
    }
};

auto test_create_A_With_StdAllocator()
{
    log_function();

    // Create on the heap
    auto pa = A::create(details::Factory {std::allocator<A>()}, 11);

    std::cout << *pa;

    return pa;
}

template <typename T1, typename T2>
constexpr void check_types() noexcept
{
    std::cout << "Types are same: " << std::boolalpha << std::is_same_v<T1, T2> << '\n';    
}

void test_type_equality()
{
    log_function();

    // Create on the stack
    std::array<std::byte, 1024> buffer;
    std::pmr::monotonic_buffer_resource mem(buffer.data(), buffer.size());

    details::Factory fact{std::pmr::polymorphic_allocator<A>(&mem)};
    auto pa = A::create(fact, 37);
    pa->doSomething();

    auto pa1 = test_create_A_With_StdAllocator();
    check_types<decltype(pa), decltype(pa1)>();
    std::cout << *pa;

    // Inheritance: factoring a family of (sub)types
    auto pb = A::create(details::Factory{std::allocator<B>{}}, 8);
    pb->doSomething();
    check_types<decltype(pa), decltype(pb)>();
    std::cout << *pb;
}


int main()
{
    test_type_equality();
}