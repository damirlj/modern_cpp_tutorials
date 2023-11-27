#include <iostream>
#include <type_traits>
#include <numbers>
#include <tuple>


namespace utils
{
    /**
     * For having more expressive interfaces, and prevent reordering of the arguments of the same type
     *
     * @tparam T The underlying type
     * @tparam Tag The additional tag parameter, for making the same underlying types distinguish
     *
     * <Example> <br>
     *
     * using property_1_t = NamedType<bool, struct Property1>; <br>
     * using property_2_t = NamedType<bool, struct Property2>; <br>
     * or for storing the reference
     * using property_3_t = NamedType<std::string&, struct Property3>; <br>
     * https://godbolt.org/z/nsq4W5sYh
     *
     * @source https://github.com/joboccara/NamedType
     */
    template <typename T, typename Tag>
    struct StrongType
    {
        // C-tors
        template <typename U = T, typename = std::enable_if_t<!std::is_reference_v<U>, void>>
        constexpr explicit StrongType(T&& val) noexcept(std::is_nothrow_move_constructible_v<T>)
            : value(std::move(val))
        {}

        constexpr explicit StrongType(const T& val) noexcept(std::is_nothrow_copy_constructible_v<T>)
            : value(val)
        {}

        // Special case for the enum class values
        template <
            typename U = T,
            typename V,
            typename = std::enable_if_t<
                std::is_enum_v<
                    U> && (std::is_same_v<std::underlying_type_t<U>, V> || std::is_constructible_v<std::underlying_type<U>, V>),
                void>>
        constexpr explicit StrongType(V val) noexcept
            : value(static_cast<T>(val))
        {}

        // Conversion operators
        constexpr operator T&() { return value; }
        constexpr operator const T&() const { return value; }

        // Getters
        [[nodiscard]] constexpr T& get() & { return value; }
        [[nodiscard]] constexpr T const& get() const& { return value; }

        /**
         * For being able to use it as function arguments with default values,
         * use internal structure to implement copy-initialization, creating the
         * outer - named type, from underlying value
         *
         * using FirstNameType = NamedType<std::string, struct FirstName>;
         * using LastNameType =  NamedType<std::string, struct LastName>;
         *
         * printName(FirstNameType::argument firstName = "James", LastNameType::argument lastName = "Bond");
         */
        struct argument
        {
            template <typename UnderlyingType>
            StrongType operator=(UnderlyingType&& val) const
            {
                return StrongType(std::forward<UnderlyingType>(val));
            }
            argument() = default;
            argument(argument const&) = delete;
            argument(argument&&) = delete;
            argument& operator=(argument const&) = delete;
            argument& operator=(argument&&) = delete;
        };


      private:
        T value;
    };
}  // namespace utils

namespace details
{
    template <typename Func, typename...Args>
    class Expression
    {
        public:
            using expression_t = Func;

            explicit Expression(Func&& func, const Args&...args) noexcept : expr_(std::forward<Func>(func)), args_(std::tie(args...)) {}

            auto operator()() const
            {
                return std::apply(expr_, args_);
            }

        private:
            expression_t expr_;
            std::tuple<const Args&...> args_; // std::tuple can strore only distinguish types!
    };

    // CTDA
    template <typename Func, typename...Args>
    Expression(Func&&, const Args&...) -> Expression<Func, Args...>;

    template <typename T1, typename T2>
    concept add_binary_expression_on_strong_type = requires(const T1& t1, const T2& t2) 
    {
        { t1.get() + t2.get() } -> std::same_as<decltype(t1.get() + t2.get())>;
    };

    /*
        We return resulting expression - without evaluating it!
        @note It is first evaluated when we invoke call operator on result Expression
        auto c = a + b;
        std::cout << c() << '\n'; // evaluated first here
    */
    template <typename LH, typename RH>
    requires add_binary_expression_on_strong_type<LH, RH>
    auto operator+(const LH& lhs, const RH& rhs) 
    {
        return Expression{[](const auto& arg1, const auto& arg2)
                          { return arg1.get() + arg2.get(); },
                           lhs, 
                           rhs
                         };
    }
}


// Example
class Radian 
{
    public:
        
        constexpr Radian(double r) noexcept : r_(r) {}

        constexpr double to_degree() const 
        {
            return (r_/std::numbers::pi) * 180;
        }

        constexpr Radian operator+(const Radian& r) const
        {
            return Radian{r_ + r.r_};
        }
        
        constexpr operator double () const 
        {
            return r_;
        }

        friend std::ostream& operator<<(std::ostream& os, const Radian& r)
        {
            return os << "r= " << r.to_degree() << "[degrees]\n";
        }

    private:
        double r_;

};

int main()
{
    // since Expression store the arguments by reference into std::tuple, arguments of the same type needs to be wrapped into StrongType: to make them distinguish
    const auto a = utils::StrongType<Radian, struct Arg1>{Radian{std::numbers::pi}};
    const auto b = utils::StrongType<Radian, struct Arg2>{Radian{std::numbers::pi/4}};

    using details::operator+;
    const auto c = a + b;
    //std::cout << c << '\n'; // No stream operator overloading for Expression type
    std::cout << c() << '\n'; // expression being evaluated first after calling call operator!
    
}