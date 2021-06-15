
#include <type_traits>
#include <cstddef>
#include <algorithm>
#include <cstdint>
#include <string>
#include <initializer_list>
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

template <typename T, typename = std::enable_if_t<is_byte_compatible_v<T>>>
std::string byte2string(T byte)
{
    
    std::string fmt {"0x%02X, "};
    char buff[fmt.size()] = {0};

    if constexpr (std::is_same_v<std::byte, T>)
    {
        std::sprintf(buff, fmt.c_str(), std::to_integer<unsigned char>(byte));
    }
    else
    {
        std::sprintf(buff, fmt.c_str(), byte);
    }

    return std::string(buff);
}

template <template<class> class Container, typename T, typename = std::enable_if_t<is_byte_compatible_v<T>>>
std::string bytes2string(const Container<T>& bytes)
{
    std::string s;

    std::for_each(bytes.cbegin()
    , bytes.cend()
    , [&s](const T& byte) mutable 
    {
        s.append(byte2string(byte));
    });

    s.resize(s.size() - 2);
    
    return s;
}

template<typename...Args>
std::vector<std::byte> make_byte_arr(Args...args)
{
    return {std::byte{static_cast<unsigned char>(args)}...};
}

int main()
{
    using namespace std;
    const std::vector<std::uint8_t> in {2, 11, 37, 221, 26};//const auto in - will be deduced to initializer_list<int>

    cout << bytes2string(in) << endl;

    cout << bytes2string( make_byte_arr(0x00, 0x12, 0xA2, 0x43, 0x37, 0x2A)) << endl;
    return 0;
}

