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


## Lesson1

<b>AOT – Active Object Thread</b> design pattern is introduced, as way to have asynchronous interthread communication, 
delegating the tasks to the background thread, enqueuing them into the tasks queue.
The thread drains the queue and provides the execution context, separate from the thread(s) from which 
the tasks are sent, in which they will be executed sequentially, in order of arrival (FIFO).
The caller thread can be also synchronized on result of task being executed, waiting on signaling the execution completion 
through the communication channel (future).

This concept is heavily used for interthread communication, especially for time consuming – blocking tasks that 
are delegated to the background thread in asynchronous way, where order of execution is preserved (first came, first served).
Typically, you would use this approach for <i>producer-consumer</i> scenarios


## Lesson2
<b>Type traits</b> are small objects for inspecting the type (rather than value) at compile time.
I’ve first encountered the type traits like constructs with “Modern C++ Design” from A. Aleksandrescu and his Loki library.
They are heavily used in template metaprogramming in various scenarios:
-	For SFINAE: imposing the template parameter substitution constraints: which types can be potentially consider as a valid for template parameter substitution (instantiation) both, as argument or return values.
-	For checking compile time condition that are type related (if constexpr)
-	For creating function overloading set (tag dispatching)
-	For any compile time type based decisions

This is small demonstration of using type traits with generic programming, where the type needs to be logged to logging medium
either converted to string (numeric values ), or expecting java-like user-defined types with public non-static member method “toString()”
In pre C++17 times, that couldn’t be done elegant as with <i>if constexpr</i>

<div>
       
        // Tag dispatching in action 
        
        using string_tag_v = enum class StringTagValues : uint8_t
        {
                tag_string,
                tag_numeric,
                tag_invalid
        };
        using string_tag_t = std::integral_constant<string_tag_v , string_tag_v::tag_string>;
        using numeric_tag_t = std::integral_constant<string_tag_v , string_tag_v::tag_numeric>;
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
        std::string conv2string(const T& t)
        {
            return conv2string(t,
                    std::integral_constant<string_tag_v,
                            (std::is_same<std::decay_t<T>, std::string>::value ||
                             std::is_constructible<std::string, T>::value ||
                             std::is_convertible<T, std::string>::value) ? string_tag_v::tag_string :
                             std::is_arithmetic<std::decay_t<T>>::value ? string_tag_v::tag_numeric : string_tag_v::tag_invalid
                        >{});
        }
</div>
   

