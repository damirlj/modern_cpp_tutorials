# Introduction

These are some collection of working concepts that served me as 
teaching platform - for exploring the new features introduced by the latest C++ standards.
Some of them were starting point in implementation of various utility 
classes and libraries in real projects, across different platforms (like socket library, logging mechanism, benchmarking, etc.)
I’ve also used them to introduce the colleagues, as part of the internal discussions and knowledge transfer, the new approaches 
in realization of the well know topics, like concurrency and new memory model, the power of callable objects and 
functional programming, generics and algorithms – template metaprogramming, chrono library, etc.

Please have in mind that using the code in commercial purposes is not allowed.  
Exposing them to the community is the way for getting the valuable feedback.  
Thanks in advance.  
    
>[Contact](damirlj@yahoo.com)


## Tutorial 1

<b>AOT – Active Object Thread</b> design pattern is introduced, as way to have asynchronous interthread communication, 
delegating the tasks to the background thread, enqueuing them into the tasks queue.
The thread drains the queue and provides the execution context, separate from the thread(s) from which 
the tasks are sent, in which they will be executed sequentially, in order of arrival (FIFO).
The caller thread can be also synchronized on result of task being executed, waiting on signaling the execution completion 
through the communication channel (future).

This concept is heavily used for asynchronous massage-based interthread communication, especially for time consuming – blocking tasks that are delegated to the background thread in asynchronous way, where order of execution is preserved (first came, first served).    
Typically, you would use this approach for

* <i>Producer-consumer</i> scenarios (audio, video streaming, etc.)
* [Actor model](https://en.wikipedia.org/wiki/Actor_model)
    * example: [Arataga](https://github.com/Stiffstream/arataga)


## Tutorial 2
<b>Type traits</b> are small objects for inspecting the type (rather than value) at compile time.
I’ve first encountered the type traits like constructs with [“Modern C++ Design”](https://en.wikipedia.org/wiki/Modern_C%2B%2B_Design) from A. Aleksandrescu and his Loki library.
They are heavily used in template metaprogramming in various scenarios:
-	For SFINAE: imposing the template parameter substitution constraints: which types can be potentially consider as a valid for template parameter substitution (instantiation) both, as argument or return values.
-	For checking compile time condition that are type related (if constexpr)
-	For creating function overloading set (tag dispatching)
-	For any compile time type based decisions

This is small demonstration of using type traits with generic programming, where the type needs to be logged to logging medium
either converted to string (numeric values ), or expecting java-like user-defined types with public non-static member method “toString()”
In pre C++17 times, that couldn’t be done elegant as with compile time <i>if constexpr</i> check

```
       
        // Tag dispatching in action 
        
        using string_tag_v = enum class StringTagValues : uint8_t
       {
               tag_string,
               tag_numeric,
               tag_enum,
               tag_invalid
       };

       using string_tag_t = std::integral_constant<string_tag_v , string_tag_v::tag_string>;
       using numeric_tag_t = std::integral_constant<string_tag_v , string_tag_v::tag_numeric>;
       using enum_tag_t = std::integral_constant<string_tag_v , string_tag_v::tag_enum>;
       using invalid_tag_t = std::integral_constant<string_tag_v , string_tag_v::tag_invalid>;


       template <typename T>
       std::string conv2string(const T& t, invalid_tag_t)
       {
           return {}; //type is not convertible to string
       }

       template <typename T>
       std::string conv2string(const T& t, string_tag_t)
       {
           return t;
       }

       template <typename T>
       std::string conv2string(const T& t, numeric_tag_t)
       {
           return std::to_string(t);
       }

       template <typename T>
       std::string conv2string(const T& t, enum_tag_t)
       {
           const auto e = static_cast<std::underlying_type_t<T>>(t);
           /*
            * @note: for some compiler the sign promotion may happen
            * enum class : uint8_t
            * passing the scoped enum to std::to_string() may invoke singed promotion, i.e.
            * std::to_string(int) overloading will be called instead of expected std::to_string(unsigned)
            * In case that is building process set to fail on any compiler issues, additional 
            * cast to double is required
            * std::to_string(static_cast<double>(e));
           */
           return std::to_string(e); 
       }
       
       template <typename T>
       struct is_string_compatible
       {
          static constexpr bool value = std::is_same<std::decay_t<T>, std::string>::value ||
                            std::is_constructible<std::string, T>::value ||
                            std::is_convertible<T, std::string>::value;
       };

       template <typename T>
       std::string conv2string(const T& t)
       {
           return conv2string(t,
                   std::integral_constant<string_tag_v,
                            is_string_compatible<T>::value ? string_tag_v::tag_string :
                            std::is_arithmetic<std::decay_t<T>>::value ? string_tag_v::tag_numeric :
                            std::is_enum<std::decay_t<T>>::value ? string_tag_v::tag_enum :  
                            string_tag_v::tag_invalid
                       >{});
       }

```

## Tutorial 3

### Tuples

Although I myself don't use tuples so frequently in everyday practice, it is sometime 
quite useful to employ this data structure, so I’ve decided to introduce some use-cases, at least 
to some of You who never encountered std::tuple in this  particular scenarios.

**First scenario** would be if there is some repetitive assignment work, which is non-trivial, because you have 
some nullable, std::optional-like custom data type that may look like
```
template <typename T>
struct Optional
{
    bool valid;
    T value;
    …
};
```
Appealing characteristic of std::tuple, is that it's fixed-size heterogenous data structure – it can hold different data types.
We can write a generic class, with variadic number of parameters, stored into tuple as lvalue references.
To do that, we use *std::tie()* call

```
    template <class T, class = void>
    struct is_optional_t : std::false_type {};

    template <class T>
    struct is_optional_t<T
                         , std::void_t<decltype(T::valid),
                                       decltype(T::value)>> : std::true_type {};
                                       
    template <typename...Args>
    class Setter
    {
        public:

            explicit Setter(Args&...args): m_values(std::tie(args...))
            {}
            explicit Setter(std::tuple<Args&...>& values) : m_values(values)
            {}

            template <std::size_t I, typename Value>
            Setter& set(Value&& value)
            {

                /*
                 * This is firstly meant for a non-trivial assignments, that are tedious to
                 * write by hand
                 *
                 */
                auto& opt = std::get<I>(m_values);
                if constexpr (is_optional_t<Value>::value)
                {
                    // In case that there is no proper c-tor
                    opt.valid = true;
                    opt.value = std::forward<Value>(value).value;
                }
                else
                {
                    opt = std::forward<Value>(value);
                }
                
                return *this;
            }

            …

        private:

            std::tuple<Args&...> m_values; // Store arguments as lvalue references!
            static constexpr size_t N = sizeof...(Args);

    };
```

**Second use-case** would be for user-defined types, where you want to provide class specific 
*“less than”* comparison operator, in order to be able to ascending sort the collection of this type using f.e. *std::sort*
algorithm.
>To accomplish this task, one can utilize on the std::tuple internal implementation of comparison 
operators, which is known as *lexicographical (alphabetical) ordering*.
This way, you just wrap your type into std::tuple, instead of writing your own comparison logic, which can be tedious and 
sometime also error-pron task


Consider having simple class which represents the person

```
    class Person
    {
        public:

            Person(int _age, std::string _name, gender_t _gender):
                age(_age)
                , name(_name)
                , gender(_gender)
            {}

            std::string getName() const { return name;}
            int getAge() const { return age;}
            gender_t getGender() const { return gender;}

        private:

            int age;
            std::string name;
            gender_t gender;
    };

    std::ostream& operator << (std::ostream& out, const Person& p)
    {
        return out << "Name=" << p.getName()
                   << ", Age=" << p.getAge()
                   << ", gender=" << printEnum(p.getGender())
                   ;
    }
    
    /*
    * You can choose class scope - unary, or binary version of 
    * the operator <
    * The binary version can be implemented also as the friend function, and 
    * wrap the private members as references within std::tie,
    * instead of use the copied values with std::make_tuple
    */
    bool operator < (const Person& p1, const Person& p2)
    {
        /*
         * Lexicographic ordering
         *
         * It's relying on the std::tuple internal implementation of the
         * comparison operator ("less than"), instead of writing your own tedious,
         * and error-pron code
         *
         */
        return std::make_tuple(p1.getAge(), p1.getName(), p1.getGender()) <
               std::make_tuple(p2.getAge(), p2.getName(), p2.getGender());
    }
```
In most cases that would be sufficient.
On the other hand, there is no garantie of the semantical-logical correctness of the comparison.
Take for instance this example

```
    struct Point { int x, y;};
    
    Writing something like
     
    bool operator < (const Point& p1, const Point& p2)
    {
         return (p1.x < p2.x) && (p1.y < p2.y);
    }
    
    is wrong, in terms of the points in Descartes coordinate system
    Instead, the correct comparison would be
    bool operator < (const Point& p1, const Point& p2)
    {
           // Distance from (0,0)
           return (p1.x * p1.x + p1.y * p1.y) < (p2.x * p2.x + p2.y * p2.y);
    }
    
    If you want to rely on the lexicographical (alphabetical) ordering,
    you would need manually write, something like
    
    bool operator < (const Point& p1, const Point& p2)
    {
         if (p1.x < p2.x) return true;
         if (p1.y > p2.y) return false;
         return p1.x < p2.x;
    }
    
    which, by the way, also give you an unsatisfactory result (doesn't concern the
    coordinates as absolute distance from (0,0)).
    
```

Check the source code for more details on this topic.

