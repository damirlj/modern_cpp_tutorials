#include <iostream>
#include <type_traits>
#include <numbers>
#include <functional>


namespace details
{
   
    template <typename Function, typename T1, typename T2>
    concept is_binary_expression = requires(const T1& arg1, const T2& arg2) {
        Function{}(arg1, arg2);
    };

    template <typename Func, typename T1, typename T2>
    static constexpr bool is_associative_bi_expression = is_binary_expression<Func, T1, T2> && is_binary_expression<Func, T2, T1>;

    /**
        A simple example of "expression templates" in C++.
        https://en.wikipedia.org/wiki/Expression_templates
        https://gieseanw.wordpress.com/2019/10/20/we-dont-need-no-stinking-expression-templates/
       
    */
    template <typename Function, typename T, typename U>
    requires is_binary_expression<Function, T, U>
    struct BinaryExpression
    {
        constexpr explicit BinaryExpression(Function&& f, const T& arg1, const U& arg2) noexcept(std::is_nothrow_move_constructible_v<Function>) : 
            callable_(std::move(f)), arg1_(arg1), arg2_(arg2) 
        {}

        constexpr explicit BinaryExpression(const Function& f, const T& arg1, const U& arg2) noexcept(std::is_nothrow_copy_constructible_v<Function>) : 
            callable_(f), arg1_(arg1), arg2_(arg2) 
        {}

        // Lazy evaluation: happens first by invoking the call operator!
        constexpr decltype(auto) operator()() const
        {
            return std::invoke(callable_, arg1_, arg2_);
        }

        // Implicit conversion operator
        using result_t = decltype(std::declval<Function>()(std::declval<const T&>(), std::declval<const U&>()));
        constexpr operator result_t () const 
        {
            return this->operator();
        }

        
        private:
            Function callable_;
            T arg1_;
            U arg2_;
    };

    // CTAD - Class Template Argument Deduction
    template <typename Function, typename T, typename U>
    explicit BinaryExpression(Function, const T&, const U&) -> BinaryExpression<Function, T, U>;


    template <typename T>
    concept Numeric = std::is_integral_v<T> || std::is_arithmetic_v<T>;
    // todo: or objects a.operator+(b)

    // Binary operations
    struct plus
    {
        template <Numeric T1, Numeric T2>
        constexpr auto operator()(const T1& a, const T2& b) const
        {
            return a + b;
        }
    };

    struct minus
    {
        template <Numeric T1, Numeric T2>
        constexpr auto operator()(const T1& a, const T2& b) const
        {
            return a - b;
        }
    };

    // Expresion templates + operators overloading

    // Operator+: returns the resulting expression, without calculating result
    template <typename Func, typename T, typename U>
    constexpr auto operator+(const BinaryExpression<Func, T, U>& l, const BinaryExpression<Func, T, U>& r)
    {
        return BinaryExpression {
                    [](const auto& a, const auto& b)
                    {
                        return a() + b();
                    }, 
                    l,
                    r
               };
    }
    // Operator-: returns the resulting expression, without calculating result
    template <typename Func, typename T, typename U>
    constexpr auto operator-(const BinaryExpression<Func, T, U>& l, const BinaryExpression<Func, T, U>& r)
    {
        return BinaryExpression {
                    [](const auto& a, const auto& b)
                    {
                        return a() - b();
                    }, 
                    l,
                    r
                };
    }
} // namespace: details

// Domain conversion: radians->degrees
constexpr double to_degrees(double radian) noexcept
{
    return (180.0 / std::numbers::pi) * radian;
}

int main()
{
    // using details::plus
    details::BinaryExpression e1(details::plus{}, std::numbers::pi, std::numbers::pi/4);
    details::BinaryExpression e2(details::plus{}, std::numbers::pi, std::numbers::pi/2); 
    //details::BinaryExpression e2(std::plus{}, std::numbers::pi, std::numbers::pi/2); // type mismatch: callable embedded into type!

    // Resulting expression: nothing happens yet
    const auto ep = e1 + e2;

    // Lazy evaluation, happens by invoking the call operator on the resulting expression: ep()
    std::cout << to_degrees(ep()) << " [degrees]\n"; 

    const auto em = e2 - e1;
    std::cout << to_degrees(em()) << " [degrees]\n"; 
}