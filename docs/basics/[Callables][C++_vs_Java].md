# [Callables][C++ vs Java]
**Author**: Damir Ljubic  
**e-mail**: damirlj@yahoo.com  
# Intro  
  
When I've started with Java (and that was long time ago) - the first obstacle that I've encountered  coming from  
C++ world was  how to implement the ***higher-order function***: a function that either takes another function as an  
argument, and/or even returns one.  
  
Let's see how we can accomplish this task in both languages, and what parallels can be drawn.  
  
## C++ approach  
  
In C++ this is quite straight forward: you just need to specify the *signature *that function has to satisfy  

```c++
template <typename R>  
using callable_type = R (*)(std::string);  
```
  
where the *callable_type* defines the signature of the **free function**.  
At the client side  

```c++  
void func(std::vector<std::string> input, callable_type<void> callback) {  
    for (const auto in : input) {  
        callback (in);  
        // std::invoke(callback, in);  
    }  
}  
```  
our higher-order function can lift this - visiting all elements in array and applying the same functionality accordingly.  
This way, our callable type becomes the *customization poin*t (Strategy Design Pattern): any callable that  
satisfies the signature, can be applied (!)  
  
If we want to restrict to the *non-static member function* of a particular UDT, we can redefine it as  

```c++  
template <ypename R, class T>  
using callable_type = R (T::)(std::string);  
```  
@note std::function is universal, polymorphic placeholder for any callable: but this is out of the scope right now.  

```c++  
template <class T>  
void func(std::vector<std::string> input, callable_type<void> callback, T& obj) {  
    for (const auto& in : input) {  
        (obj.*callback)(in);  
        // (ptr->*callback)(in); // in case that we pass the pointer  
        // std::invoke(callback, obj, in);  
   }  
}
```
  
Back to signature.  
It becomes even more obvious using the *std::invoke* utility function, that the instance of the UDT needs to be the  
very first argument of any non-static member function invocation.  
Welcome to the world of OO programming.  
  
### Variadic arguments pack  
  
Where C++ prevails is that with C++ we can specify really generic: universal function signature,  
with arbitrary number of arguments - even of a different type, using **variadic arguments** pack  

```c++  
template <typename R, typename...Args>
using universal_callback_type = R (*)(Args&&...);
```
  
@note We can also add the *const qualifier* to signature - to make the function's enclosing type T immutable  

```c++  
template <typename R, typename T, typename...Args>  
using universal_callback_type = R (T::*)(const Args&&...) const;
```
  
@note Actually, we can add *volatile* qualifier as well - which means, that the function will be called on the  
volatile instance of the enclosing class T  
  
### Exception in signature  
  
In C++  - one can also specify explicitly - as part of the function signature, whether  
the function may throw  

```c++  
template <typename R>  
using callback_type = R (*)(void) throw (std::logic_error);
```  
  
Starting with C++11 - this is considered deprecated.  
Instead - assuming that every function (except destructor) can implicitly throw, as a hint to compiler  
there is a new operator ***noexcept ***that indicates whether the function may throw - or not  
(noexcept == noexcept(true)): it can be conditionally expressed  

```c++  
template <typename R, typename...Args>  
using callback_type = R (*)(Arg&&...) noexcept (std::is_nothrow_copy_constructible_v<Args> &&...);  
```
  
As a consequence - there will be no stack unwinding in order to propagate the exception to the  
outer functions that presumably catch and handle exception: but rather if the exception is thrown - the program  
will terminate (std::terminate)  
  
  
## Java approach  
  
So, how we can accomplish the same with Java?  
And Java is indeed the pure OO language.  
  
In Java, we can specify a custom **callback interface**  

```java
@FunctionalInterface  
interface CallbableType<R, T> {  
    R apply(T obj);  
}  
```
  
This would be equivalent to defining the non-static member function of T, that is **parameterless** - since the very first argument  
must be the instance on which the method will be invoked.  
  
Then, we define the higher-order function as  

```java  
<R, T> R func(@NonNull CallableType <R, T> callback, T obj) {  
    // do something  
    return callback.apply(obj);  
}
```  
  
This is similar calling the std::invoke, providing the instance of T, as a first argument  
  
We can, on the place where *callback* is expected, provide:  
- Reference to the non-static member function of the enclosing class
  ```java
      this::<function>
  ```
- Reference to the non-static member function of another class,  for which we need to provide argument as well
  ```java
     <Class>::<function>  
  ```
- We can provide the lambda object on the fly, that satisfies the signature
  
  ```java
    (obj)-> {  
        // do something  
        return obj.<func>();  
    }  
  ```
Pay attention - we don't explicitly implement the functional interface: we use it as a *placeholder* for providing  
the already existing callables that satisfy the signature  

```java  
  private <R> List<R> transform(@NonNull List<Person> list, @NonNull CallableType<R, Person> callable) {  
      return list.stream().map(callable::apply).collect(toList());  
  }  
    
  @Test  
  public void testTransformPersonToName() {  
      List<Person> people = List.of(new Person("Alice", 25), new Person("Bob", 30));  
    
      transform(people, Person::getName).forEach(System.out::println); // Reference to method  
      transform(people, person -> person.getAge()).forEach(System.out::println); // Lambda expression  
  }  
```
  
@note C++ introduced the *ranges* in C++20. Java has *streams* since Java 8 SDK.  
  
As matter of fact - there are already predefined* Functional Interfaces* which are part of the *java.util.function *package,  
that is introduced with Java 8 SDK.  
  
Function<R, T> offers the same apply() callback as our manually written interface.  
  
Actually, it's callbacks interface - since it provides the signature for three additional callbacks.  
It's even composable, since you can instantiate it with the callable that will be applied first, and  
then we can specify additional callback, that will be invoked consequently.  
  
@note There are more other useful predefined Functional Interfaces, like *Consumer\<T&gt;*==Functional\<Void, T&gt; to support  
the functional programming style in Java.  
  
### Exception in signature  
  
Interesting enough, the Exception Type can be also part of the function signature in Java  

```java
  @FunctionalInterface  
  interface CallableType<R, T> {  
      R apply(T obj) throws RemoteException;  
  }  
```  
or to be generic as possible  

```java  
  @FunctionalInterface  
  interface CallableType<R, T, E extends Exception> {  
      R apply(@NonNull T obj) throws E;  
  }  
```  
Our higher-order function may preserve the exception indication in its own signature  

```java  
  <T, R, E extends Exception> R invoke(CallableType<T, R, E> f, T arg) throws E {  
      // do something  
      return f.apply(arg);  
  }  
```  
or it can handle it internally.  
  
In case that due to *interoperability* - the API expects the Function Interface which is not throwable, and we use the one which may throw, we can write the safe wrapper to bridge it  

```java  
  static <T, R> Function<T, R> safeWrapper(ThrowingFunction<T, R, ?> f) {  
      return t -> {  
          try{  
                  return f.apply(t);  
             } catch (RuntimeException e) {  
                  throw e; // Re-throw unchecked exceptions  
             } catch (Exception e) {  
                  throw new IllegalStateException("Unexpected checked exception", e);  
             }  
     };  
  }  
```      
  
### Variadic arguments pack  
  
Unlike C++ - Java doesn't support variadic arguments pack, at least not for expressing the arbitrary number of  
heterogenous types.  
It supports only variadic arguments list of the same type  

```java
@FunctionalInterface  
interface CallableType<R, T, E extends Exception> {  
    R apply(@NonNull T...objs) throws E;  
}  
```  
  
## Conclusion  
  
At the end - comparing these two approaches, Java is doing quite well, considering the fact that the generic - template  
programming was not originally part of the language core - it's added afterwards, with Java 5 SDK - implementing it as  
*type erasur*e (stripping all type information - and treating it as Object inside the function template).  
  
More on **type erasure**, and different interpretations - in different languages:  
[Type Erasure](https://github.com/damirlj/modern_cpp_tutorials?tab=readme-ov-file#tut7)  
  
  
The only where Java is inferior in fulfilling this particular task - is the fact that heterogenous variadic argument pack is not  
supported (nor the fold expressions, type-traits, auto type deduction and all other mechanics of template metaprogramming).  
The fact is - that both languages inspire each other to be a better version of itself: embracing mutually the concepts that  
make the language more expressive, safe and overall modern: tailored to customer expectations.  
  

