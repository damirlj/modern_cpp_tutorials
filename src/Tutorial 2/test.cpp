
// Catch2 single-header test framework

#include "catch.hpp"

// Application

#include "LoggingHelper.h" //@see type traits related str() conversion implementation



namespace
{
    using check_t = void (*)(bool);

    void check_true(bool arg) { CHECK(arg); }
    void check_false(bool arg) { CHECK_FALSE(arg);}

    template <typename T>
    void checkOptional(check_t check_f, T&& t)
    {
        using namespace std;
        const optional<std::string> result = utils::log::str(std::forward<T>(t));
        const auto has_value = result.has_value();
        cout << std::boolalpha;
        cout << "Succeed: " << has_value << endl;

        check_f(has_value);

        if (has_value) cout << *result << endl;
    }
}

namespace 
{
	void testToString()
	{
		using namespace std;
		using namespace utils::log;

		using test_en_t = enum class Test : uint8_t
		{
				test_0,
				test_1,
				test_2
		};

		cout << "Test enum value:\n";
		const auto value1 = test_en_t::test_1;
		checkOptional(check_true, value1);


		cout << "Test floating point value:\n";
		const auto value2 = 3.2f;
		checkOptional(check_true, value2);

		cout << "Test string literal:\n";
		checkOptional(check_true, "Alex");

		cout << "Test integral value:\n";
		checkOptional(check_true, 37);

		struct A
		{
			std::string toString() const
			{
				return std::string("Calling A::toString()");
			}
		};
		cout << "Test java-like object with toString() public method\n";
		checkOptional(check_true, A{});

		struct B{};
		cout << "Test non-conversional type\n";
		checkOptional(check_false, B{});
	}

}


TEST_CASE("Logging str", "[log][str]")
{
    
    testToString();
}