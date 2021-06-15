
#include <type_traits>
#include <cstddef>
#include <algorithm>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <iostream>


template<typename T>
struct is_byte_compatible
{
    static constexpr bool value = std::is_same_v<std::byte, T> ||
    std::is_same_v<std::uint8_t, T> || 
    std::is_same_v<unsigned char, T>
    ;
};

template <typename T>
inline  constexpr bool is_byte_compatible_v = is_byte_compatible<T>::value;

/*
 * Helper method for converting byte to string representation
 */
template <typename T, typename = std::enable_if_t<is_byte_compatible_v<T>>>
std::string byte2string(T byte, std::string_view fmt)
{
    
    char buff[fmt.size()] = {0};

    if constexpr (std::is_same_v<std::byte, T>)
    {
        std::sprintf(buff, fmt.data(), std::to_integer<unsigned char>(byte));
    }
    else
    {
        std::sprintf(buff, fmt.data(), byte);
    }

    return std::string(buff);
}

/*
 * Conversion byte array to string representation
 */
template <template<class> class Container, typename T, typename = std::enable_if_t<is_byte_compatible_v<T>>>
std::string bytes2string(const Container<T>& bytes)
{
    std::string s;
    constexpr std::string_view fmt {"0x%02X, "};

    std::for_each(bytes.cbegin()
    , bytes.cend()
    , [&s, fmt](const T& byte) mutable 
    {
        s.append(byte2string(byte, fmt));
    });

    s.resize(s.size() - 2);//remove trailing delimeters for the last entry
    
    return s;
}

/*
 * Helper function for generating the std::byte array
 * from underlying type (unsigned char)
 */
template<typename...Args>
std::vector<std::byte> make_byte_arr(Args...args)
{
    return {std::byte{static_cast<unsigned char>(args)}...};
}

int main()
{
    using namespace std;
    
    const std::vector<std::uint8_t> in {2, 11, 37, 221, 26};//@note const auto in - will be deduced to initializer_list<int>

    cout << bytes2string(in) << endl;

    cout << bytes2string( make_byte_arr(0x00, 0x12, 0xA2, 0x43, 0x37, 0x2A)) << endl;
    
    return 0;
}

