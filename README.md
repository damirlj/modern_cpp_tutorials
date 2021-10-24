# Introduction

These are some collection of working concepts that served me as 
teaching platform - for exploring the new features introduced by the latest C++ standards.
Some of them were starting point in implementation of various utility 
classes and libraries in real projects, across different platforms (like socket library, logging mechanism, benchmarking, etc.).
I’ve also used them to introduce the colleagues, as part of the internal discussions and knowledge transfer, the new approaches 
in realization of the well known topics, like concurrency and new memory model, the power of callable objects and 
functional programming, generic – template (meta)programming, chrono library, etc.

Please have in mind that using the code in commercial purposes is not allowed.  
Exposing them to the community is the way for getting the valuable feedback.  
Thanks in advance.  
    
>[Contact](damirlj@yahoo.com)


## Tutorial 1

<b>AOT – Active Object Thread</b> design pattern is introduced, as way to have asynchronous inter-thread communication, 
delegating the tasks to the background thread, enqueuing them into the tasks queue.
The thread drains the queue and provides the execution context, a separate one from the thread(s) from which 
the tasks are sent, in which they will be executed sequentially, in order of arrival (FIFO).
The caller thread can be also synchronized on result of task being executed, waiting on signaling the execution completion 
through the communication channel (future).

This concept is heavily used for asynchronous massage-based inter thread communication, especially for time consuming – blocking tasks that are delegated to the background thread in asynchronous way, where order of execution is preserved (first came, first served).    
Typically, you would use this approach for

* <i>Producer-consumer</i> scenarios (audio, video streaming, etc.)
* [Actor model](https://en.wikipedia.org/wiki/Actor_model)
    * example: [Arataga](https://github.com/Stiffstream/arataga)


## Tutorial 2
<b>Type traits</b> are small objects for inspecting the type (rather than value) at compile time.
I’ve first encountered the type traits like constructs in [“Modern C++ Design”](https://en.wikipedia.org/wiki/Modern_C%2B%2B_Design) introduced by A. Aleksandrescu and his Loki library.
They are heavily used in template metaprogramming in various scenarios:
-	For **SFINAE**: imposing the template parameter substitution constraints: which types can be potentially consider as a valid for template parameter substitution (instantiation) both, as argument or return values.
-	For checking compile time condition that are type related (if constexpr)
-	For creating function overloading set (tag dispatching)
-	For any compile time type based decisions

This is small demonstration of using type traits with generic programming, where the type needs to be logged to logging medium
either converted to a string (for numeric values), or expecting java-like user-defined types with public non-static member method “toString()”.  
In pre C++17 times, this requirement couldn’t be done as elegant as with compile time <i>if constexpr</i> check

```c++
   // Tag dispatching in action for C++14 compiler

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

To be honest, I don't use tuples so frequently in everyday practice, although it's sometime inevitable, even 
recommended.  
Therefore, I’ve decided to introduce some use-cases, where tuples can be quite useful.  

**First scenario** would be if there is some repetitive assignment work, which is non-trivial, because you have 
some nullable, std::optional-like custom data type that may look like
```c++
    template <typename T>
    struct Optional
    {
        bool valid;
        T value;
        …
    };
```
Appealing characteristic of _std::tuple_, is that it's fixed-size heterogenous data structure – it can hold different data types.  
We can write a generic class, with variadic number of parameters, stored into tuple as lvalue references.  
To do that, we use *std::tie()* call

```c++
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
                    // In case that there is no proper c-tor: for POD like type
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
sometime also error-prone task


Consider having simple class which represents the person

```c++
    class Person
    {
        public:

            Person(int _age, std::string _name, gender_t _gender) noexcept :
                age(_age)
                , name(std::move(_name))
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
         * and error-prone code
         *
         */
        return std::make_tuple(p1.getAge(), p1.getName(), p1.getGender()) <
               std::make_tuple(p2.getAge(), p2.getName(), p2.getGender());
    }
```

On the other hand, there is no garantie of the semantical - logical correctness of the comparison.  
Take for instance this example

```c++
    struct Point { int x, y;};
```
    
Writing something like  

```c++
    bool operator < (const Point& p1, const Point& p2)
    {
         return (p1.x < p2.x) && (p1.y < p2.y);
    }
```
    
is wrong, in terms of the points in Descartes coordinate system
Instead, the correct comparison would be

```c++
    bool operator < (const Point& p1, const Point& p2)
    {
           // Distance from (0,0)
           return (p1.x * p1.x + p1.y * p1.y) < (p2.x * p2.x + p2.y * p2.y);
    }
```
    
If you want to rely on the lexicographical (alphabetical) ordering,
you would need manually write, something like

```c++
    bool operator < (const Point& p1, const Point& p2)
    {
         if (p1.x < p2.x) return true;
         if (p1.y > p2.y) return false;
         return p1.x < p2.x;
    }
```
which give you semantically an unsatisfactory result (doesn't concern the coordinates as absolute distance from (0,0)).


Check the [source code](/src/Tutorial%203) for more details on this topic.

## Tutorial 4
### Functional programming

It's a common mistake thinking on C++ as a solely OOL  - it's a hybrid language that supports the OOD paradigm, 
but It's also the functional language, especially having in mind that the _std_ library itself is written in a way to follow the
principles of the functional programming: having generic _higher-order_ functions that take as argument another functions,
or return another functions as result. 

The basic idea is to have a generic code, highly customizable, that can be tailored to different kind of scenarios,
by specifying  the callable object that will be internally applied, usually on some iterable collection. 
This generic code introduces some abstraction, by hiding the implementation details - the complexity if you like,
and providing the more expressive code - the code which is concise, compact, easy to read and understand, 
the code which is rather declarative, than imperative. 

For those who are familiar with **Java Streams**, they embody these kind of principals: having chain of composable 
operations - monads, that transform the stream until they reach the terminal operation

```java
    List<Person> persons = …
    persons.stream()
               .filter(person->person.getAge() >= age) //lambda
               .map(Person::getName) // non-static member method reference
               //.forEach(System.out::println) // static member method reference
              .collect(Collectors.toList());
```

Here, instead of being burden with implementation details (_"how"_ the operators are implemented) - imperative programming, 
we are more focused on _"what"_ should be done, as with declarative programming - expressing our intentions with callable objects 
on a chained operators. You can think of it as Strategy design pattern.
Talking about desing patterns, there is **ReactiveX** library. It's basically an implementation of Publisher-Subscriber 
architectural pattern, that comes in different languages. Java implementation is konw as _RxJava_.
The syntax is very similar with Java Streams, but that is where all other similarity seas. Instead of iterating over the collection, 
the reactive library is for having asynchronous event-based communication between observable (publisher) and observer (subscriber).
We can even specify different thread contexts in which the observable will emit events/items, from the one in which the observer 
will receive these items.
The library itself provides all the necessary infrastructure, so that we, once again, have declarative rather  than imperative programming.

```java
    public Disposable test_mapToList()
    {
        Button personsButton = (Button) app.findViewById(R.id.button);
        Observable<Unit> personsButtonPublisher = RxView.clicks(personsButton);

        return personsButtonPublisher
                .subscribeOn(Schedulers.io())
                .map(e->{
                    List<Person> persons = new ArrayList<Person>();
                    Collections.addAll(persons,
                            new Person("Alex", 7, Gender.MALE),
                            new Person("John", 45, Gender.MALE),
                            new Person("Marry", 47, Gender.FEMALE)
                    );
                    return persons;
                })
                .flatMap((Function<List<Person>, Observable<List<String>>>) persons-> {
                    return Observable.fromIterable(persons)
                            .filter(person->person.getAge() > 18)
                            .map(Person::getName) // transformation function f: Person->String
                            .toList()
                            .toObservable();
                })
                .observeOn(AndroidSchedulers.mainThread())
                .subscribe(this::updateNames);
    }
```

But, how C++ implementation may look like?

The first, traditional approach - using good-old loop

```c++
    /*
     * Imperative way
     *
     * This is actually kind of lifting the generic filter function to
     * be used by the higher-order namesOf function, without having any
     * side-effects: changing the input data
     * So, it's more functional approach, but it uses loop instead of std algorithm, and
     * it's less composable as the genuine functional approach
     */
    
    using persons_t = std::vector<Person>;
    
    template <typename FilterFunc>
    std::vector<std::string> namesOf(const persons_t& persons, FilterFunc filter)
    {
        std::vector<std::string> names;
        names.reserve(persons.size());
        
        for (const auto& person : persons)
        {
            if (filter(person))
            {
                names.push_back(std::move(person.getName()));
            }
        }

        return names;
    }
```

This would be an imperative - or halfway declarative approach. 
To turn it into truly **declarative** way, so that the code is more expressive and reusable,
we need to make separation of concerns. But this flexibility doesn't come without costs.
The main problem is that __std algorithms are not composable__. The reason for that is that 
they use iterators instead of the concreate collections, which requires the auxiliary memory space:
if we want to have a _pure_ function which doesn't alter input collection

__Filter__ function

```c++
    template <typename UnaryPredicate> 
    persons_t filterPersons(const persons_t& persons, UnaryPredicate p)
    {
          persons_t filteredPersons; // auxiliary memory space
          filteredPersons.reserve(persons.size());
          
          std::copy_if(persons.cbegin(), persons.cend(), std::back_inserter(filteredPersons), p);

          return filteredPersons;
    }
```

__Transformation__ function would be

```c++
    template <typename Func, typename R = std::invoke_result_t<Func, const Person&>>
    auto mapPersons(const persons_t& persons, Func func)
    {
        std::vector<R> result; // auxiliary memory space
        result.reserve(persons.size());

        std::transform(persons.cbegin(), persons.cend(), std::back_inserter(result), func);

        return result;
    }
```

Eventually, we can write something like

```
    print(mapToNames(adults(persons)); // from inner to outher call
```


> **Note** For more details please visit: [__Turorial 4__](/src/Tutorial%204/TestFP.cxx)


There is also a third-party range library, which simplified the syntax using '|' operator

 ```c++
    auto adultsNames(const persons_t& persons) 
    {
        return persons | filter(is_adult)| transform(to_name); // from left to right
    }
 ```

As we can see, this is the most resemble to the Java Streams solution, without additional penalties 
in terms of introducing the auxiliary memory space. 


#### Functors and monads

Functor: the function which takes as input argument class template of one instance F<T1>, and return the transformed instance 
of the same class template F<T2>:
>>>
    template <typename T1, typename Func>
    auto map(F<T1> t1, Func f)->decltype(F(f(T1))) 
    {
       ...
    }

    where f: T1->T2 is transformation function.
>>>
    
Typical example of a functor is _std::optional<T>_

```c++
    template <typename T, typename Func>
    auto map(const std::optional<T>& t, Func f) -> decltype(std::make_optional(f(*t)))
    {
           if (t) return std::make_optional(f(*t));
           return {};
    }
```
    
Let assume we have this two transformation functions

```c++
    std::string f1 (const std::string& s); // first transformation function
    std::string f2 (const std::string& s); // second transformation function
``` 

Now we can write

```c++
    map(map(s, f1), f2); //pay attention that map(s, f1) wraps the result into std::optional<std::string>
``` 

The problem arises when transformation functions return std::optional itself, to indicate the outcome of operation, 
since map(s, f1) would then return std::optional<std::optional<std::string>>.
This is where monads come in rescue, by slightly redefine the transformation helper function 
    
```c++
    template <typename T, typename Func>
    auto map(const std::optional<T>& t, Func f) -> decltype(f(*t))
    {
           if (t) return f(*t);
           return {};
    }
```

One interesting monad is also _std::future<T>_ which is used to wait on result of the asynchronous task.
We would usually call std::future::get method, which is blocking call.
In order to be able to combine the results of asyncrhonous tasks in a functional way,
we need helper function that converts blocking get call into non-blocking one.
    
@see [Herb Sutter's talk](https://channel9.msdn.com/Shows/Going+Deep/C-and-Beyond-2012-Herb-Sutter-Concurrency-and-Parallelism)    

```c++
    /*
     * std::future<T> as monad.
     *
     * To compose the futures (or better say, asynchronous tasks),
     * we need to convert the blocking std::future::get call into
     * non-blocking one, by spawning another asynchronous task that will wait
     * on previous one being signaled - result being returned
     */
    template <typename T, typename Func>
    auto then(std::future<T>&& f, Func func)
    {
        return std::async(std::launch::async, [f = std::move(f), func]() mutable
                {
                    return func(f.get());
                });
    }
```

Since this is presentation from the second hand, for this and many other concepts of functional programming,
I would highly recommend to read this exelent book ["Functional Programming in C++" by Ivan Čukić](https://cppcast.com/ivan-cukic)
    
## Tutorial 5
DIP - **Dependency Inversion Principle** is one in collection of well-known SOLID principles, which in short
advises that in order to loosely couple on dependency, we should depend on the _abstraction_ rather than on the _concreate implementation_.
This way, we decouple the behavioral aspect of the interface, from its concrete implementation.
In other words, the _Client_ shouldn't be aware of implementation details - nor it should directly instantiate the dependency object, the _Service_.
This should be responsibility of another component, _Injector_ - that would [inject the dependency](https://en.wikipedia.org/wiki/Dependency_injection)
at client side either through

* Constructor of the client
* Setter method
* Interface method
    
	```c++
        template <typename Service>
        struct ServiceSetter
        {
            virtual void set(std::shared_ptr<Service> service) = 0;
        };

        template <typename Services…>
        class Client : public ServiceSetter<Services>…
        {
        };
	```
    
In some languages (like Java), there are even  DI Frameworks that can be used for introducing the highly configurable
runtime dependency injection (Spring, Dagger, etc.).

But, what would be **benefit** of this?  
One obviously is for having flexible code, that can be easily configured for different kind of dependency implementations, 
without paying the price of refactoring - changing the code, since the interfaces remaining the same.  
Another benefit would be simplified test scenarios, where the real implementation is replaced with the _mocking_ one.  

### Implementation
    
There is another, more generic term: IOC-_Inversion Of Control_, where DI is just one variation of it.  
We usually talk about the IOC container - a centralized component that holds all dependency modules across the process.
Internally, it holds two associative arrays, since the DO (Dependency Object) can be either obtained as a shared (already created) instance,
or the client can require new instance of DO, which will be then factored - by invoking the matching factory.
This means, that we need to store the arbitrary number of arguments required by the factory, for being able to create on demand,
a new DO instance - on the same argument list. We could use, once again, [*tuple*](/src/Tutorial%203) as a storage for these arguments  

```c++
    template <typename DIServiceInterface, typename DIService, typename...Args>
    class DIFactory final : IFactory<DIServiceInterface>
    {
	    …
	
        /**
         * For binding the arguments of dependency object - service creation,
         * with factory method
         *
         * <p>
         * The service factory will be placed into IOC container.
         * In case that expectation is that each call of the @see DIFactory#create
         * creates the new instance of the service implementation, to accomplish that,
         * the arguments for the factory function need to be stored into tuple
         *
         * @param args  Service implementation construction arguments
         */
         explicit DIFactory(Args&&...args) noexcept :
            m_args(std::make_tuple(std::forward<Args>(args)...))
        {}

        private:
            std::tuple<std::decay_t<Args>...> m_args; // store the arguments
    };
```
    
To produce a new instance of DO, we call eventually [std::apply](https://en.cppreference.com/w/cpp/utility/apply) on that tuple  

```c++  
    /**
     * Factory method: creates the dependency object, a
     * Service that will be injected at client side.
     *
     * @return  Reference to the service concrete implementation upcasted (RVO)
     *          to the matching interface
     *
     * @note template <class T> class A{ public: A(T* t);};
     * There is no hierarchy "is a" relationship between A<Base> and A<Derived> instances
     * (only between template parameters!)
     */
    std::unique_ptr<DIServiceInterface> create() override
    {
           return std::apply(
                [](auto&&...args)
                {
                    return factory<DIService>(std::forward<decltype(args)>(args)...);
                }
               , m_args);
    }
```

In order to be able to store in homogenous collections objects of a different type - we need some kind of
universal type-container, that could hold virtually any type.

>Hint: We don't want to use _void *_.
    
    

Since C++17, std library offers exactly what we need - [std::any](https://en.cppreference.com/w/cpp/utility/any).  
Taking into account constraint, that contained type needs to be copy-constructible, we will use std::shared_ptr<T> to  
reference both, not only DO, but also the fatory object.  

```c++
    using factories_map = std::unordered_map<std::type_index, std::any>;
    using instances_map = std::unordered_map<std::type_index, std::any>;

    
   /**
    * Adding to container the factory method for creating the matching
    * dependencies
    *
    * @tparam DIService        Service interface
    * @tparam DIServiceImpl    Service concrete implementation
    * @tparam Args             Arbitrary argument types
    * @param  args             Arguments to construct the concrete service implementation
    */
    template <class DIService, class DIServiceImpl, typename...Args>
    inline void DIContainer::add(Args&&...args)
    {
        using namespace std;

        const auto id = type_index(typeid(DIService)); //C++ "reflection": service interface type index as a key

        const auto it = m_diFactories.find(id);
        if (it != m_diFactories.end()) throw std::runtime_error("DI: Service factory already specified!");

        auto factory = make_factory<DIService, DIServiceImpl>(std::forward<Args>(args)...);
        if (!factory) throw std::runtime_error("DI: Service factory not created!");

        // Save instance - for the case that shared instance is required

        m_diServices[id] = std::any(std::shared_ptr<DIService>(factory->create()));

        // Save service factory - for the case that new instance is required

        m_diFactories[id] = std::any(std::shared_ptr<IFactory<DIService>>(std::move(factory)));
    }
```

At client side, we are only aware of the interface - not the concrete implementation.  
To achieve that, to inject a proper implementation, we rely on the **C++ "reflection"**, mapping the index
of the service interface type (_std::type_index_) to the associated counterpart  
    
```c++  
    /**
     * Retrieving at client side the concrete service implementation, based on
     * the given service interface type - its std::type_index
     *
     * @tparam DIService    Service interface
     * @param shared        Whether to retrieve the shared reference to the service, or
     *                      create a new instance
     * @return              The reference to the concrete service implementation
     */
    template <class DIService>
    inline std::optional<std::shared_ptr<DIService>> get(bool shared)
    {
         using namespace std;
         // Service interface as key to find the associated implementation
         const auto id = type_index(typeid(DIService)); 
         /*
          * If the shared instance of the service implementation is required,
          * grab from the container the matching one
          */
         if (shared)
         {
             if (const auto it = m_diServices.find(id); it != m_diServices.end())
             {
                 return std::any_cast<std::shared_ptr<DIService>>(it->second);
             }
             else
             {
                 cerr << "DI: Service implementation not found!\n";
                 return nullopt;
             }
         }

         // Otherwise, return the new instance of the service implementation

         if (const auto it = m_diFactories.find(id); it != m_diFactories.end())
         {
             const auto& factory = std::any_cast<const std::shared_ptr<IFactory<DIService>>&>(it->second);
             return factory->create();
         }
         else
         {
             cerr << "DI: Service factory not found!\n";
             return nullopt;
         }
    }
```  

Additionally, we can write a generic client as well, that stores the dependencies into type-safe unions [std::variant](https://en.cppreference.com/w/cpp/utility/variant),  
and use the **visitor pattern** along with the function object which overloaded _call operator_ set covers all dependencies.  
Usually, we want to target the single dependency, rather than visit all of them at once.  
For that, we will call _std::holds_alternative_ explicitly, to check whether DO is held by the union  
    
```c++
    /**
     * Provide for the particular service type
     * the callable object that will be invoked
     * on the injected service implementation
     *
     * @param func  Function object that will be invoked on the injected
     *              service implementation
     * @return      The return value of invocation, for existing service: otherwise, exception will be thrown
     */
    template <typename Service, typename Function>
    decltype(auto) call(Function func)
    {
        if (auto it = std::find_if(m_services.begin(), m_services.end()
                , [](const auto& service)
                {
                    return std::holds_alternative<Service>(service);
                }
                ); it != m_services.end())
        {
            return func(std::get<Service>(*it));
        }
        else
        {
           throw std::logic_error("<DI> Non-existing service required!");
        }
    }
```

The complete source code with examples is available at: [Tutorial 5](/src/Tutorial%205)
    
