/*
 * TestFP.cxx
 *
 *  Created on: Oct 4, 2021
 *      Author: <a href="mailto:damirlj@yahoo.com">Damir Ljubic</a>
 */

#include <string>
#include <vector>
#include <algorithm>
#include <numeric>

#include <iostream>
#include <iterator>
#include <future>
#include <thread>
#include <chrono>



#include "TestFP.hxx"

namespace test::fp
{
    using gender_t = enum class Gender
            {
                female,
                male
            };

    class Person
    {
        public:
            Person(std::string name, int age, gender_t gender):
                m_name(name),
                m_age(age),
                m_gender(gender)
            {}

            std::string getName() const { return m_name;}
            int getAge() const { return m_age;}
            gender_t getGender() const { return m_gender;}

        private:
            std::string m_name;
            int m_age;
            gender_t m_gender;
    };


    using persons_t = std::vector<Person>;

    /*
     * Imperative way
     *
     * This is actually kind of lifting the generic filter function to
     * be used by the higher-order namesOf function, without having any
     * side-effects: changing the input data
     * So, it's more functional approach, but it uses loop instead of std algorithm, and
     * it's less composable as the genuine functional approach
     */
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

    /*
     *  Declarative way using std algorithms
     *
     *  Pros:
     *  - more expressive code
     *  - reusable code
     *  - no side effects (immutable input data)
     *  Cons:
     *  - suboptimal in terms of both performance and memory space
     *
     *  One of the crucial problem with std algorithms is that they are not
     *  composable. The reason for that is that they accept range of iterators as arguments,
     *  rather then iterable collection itself. Therefore, to overcome this limitation, the auxiliary
     *  memory space is required
     */

    template <typename UnaryPredicate>
    persons_t filterPersons(const persons_t& persons, UnaryPredicate func)
    {
        persons_t filteredPersons; //auxiliary memory space
        filteredPersons.reserve(persons.size());

        /*
         * std library provides different kind of
         * filter algorithms on iterable types, like
         * std::count_if, or any std::*_if candidate,
         * depends on the concrete task
         */
        std::copy_if(persons.cbegin(), persons.cend()
                , std::back_inserter(filteredPersons)
                , func);

        return filteredPersons;
    }

    /*
     * Erase-Remove idiom
     *
     * @note Since C++20 there is std::erase_if
     *
     * This violate the FP principle of having "pure" function
     * In mathematical sense, that always produce
     * the same result, for the same given arguments, without having
     * any side effects - without changing the state of the captured
     * objects
     *
     *
     */
    template <typename UnaryPredicate>
    void filterPersons(persons_t& persons, UnaryPredicate func)
    {
        persons.erase(std::remove_if(persons.begin(), persons.end(), func), persons.end());
    }

    // Different kind of filters, as pure functions: which makes them easy-safe to compose

    persons_t filterByAge(const persons_t& persons, int age)
    {
        return filterPersons(persons, [age](const Person& person){//unary predicate
            return person.getAge() >= age;
        });
    }

    persons_t filterByGender(const persons_t& persons, gender_t gender)
    {
        return filterPersons(persons, [gender](const Person& person){//unary predicate
            return person.getGender() == gender;
        });
    }

    /*
     * Compose two functions in most generic way,
     * returning their composition in form of the reusable
     * closure - lambda instance.
     *
     * Precondition:
     *    f1: T1->T2
     *    f2: T2->T3
     *
     */
    template <typename Func1, typename Func2>
    auto compose(Func1 f1, Func2 f2)
    {
        return [=](const auto& value)
               {
                    return f2(f1(value));
               };
    }

    template <typename MapFunc, typename R = std::invoke_result_t<MapFunc, const Person&>>
    auto mapPersons(const persons_t& persons, MapFunc func)
    {
        std::vector<R> result; // result of transformation
        result.reserve(persons.size());

        std::transform(persons.cbegin(), persons.cend(), std::back_inserter(result), func);

        return result;
    }

    std::vector<std::string> personsNames(const persons_t& persons)
    {
        return mapPersons(persons, [](const Person& person){
            return person.getName();
        });
    }


    void print(const std::vector<std::string>& items)
    {
        using namespace std;
        for (const auto& item : items){
            cout << item << '\n';
        }
    }


    void testComposable(const persons_t& persons)
    {
        constexpr int adults = 18;
        const auto adultPersons = std::bind(filterByAge, std::placeholders::_1, adults);
        const auto malePersons = std::bind(filterByGender, std::placeholders::_1, gender_t::male);
        const auto femalePersons = std::bind(filterByGender, std::placeholders::_1, gender_t::female);

        std::cout << "\nAdults:\n";
        print(personsNames(adultPersons(persons)));

        std::cout << "\nMales:\n";
        print(personsNames(malePersons(persons)));

        std::cout << "\nFemales:\n";
        print(personsNames(femalePersons(persons)));

        // Combining two filters into single one

        std::cout << "\nMale adults:\n";
        const auto maleAdultsFilter = compose(malePersons, adultPersons);
        print(personsNames(maleAdultsFilter(persons)));

        // Explicit filter composition

        std::cout << "\nFemale adults:\n";
        print(personsNames(femalePersons(adultPersons(persons))));

    }


    /*
     * std::future<T> as monad.
     *
     * To compose the futures (or better say, asynchronous tasks),
     * one needs to convert the blocking std::future::get call into
     * non-blocking, by spawning another asynchronous task that will wait
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

    auto testFuturesAsMonads(const persons_t& persons)
    {
        auto f = std::async(std::launch::async, [p = persons]{

               using namespace std::chrono_literals;

               std::this_thread::yield();
               std::this_thread::sleep_for(1s); // simulates background work

               return filterByGender(p, gender_t::female);
        });

        // This express intention: what to do with the result when it becomes available

        return then(then(std::move(f), personsNames), print);
    }




    /*
     * Call in your main.cpp as
     *
     *      int main()
     *      {
     *          return test::fp::testFP();
     *      }
     */
    int testFP()
    {
        const std::vector<Person> persons =
        {
            Person("Alex", 7, gender_t::male),
            Person("John", 45, gender_t::male),
            Person("Marry", 47, gender_t::female),
            Person("Suzanne", 14, gender_t::female)
        };

        std::cout << "Test std::future<T> as monad...\n";

        auto f = testFuturesAsMonads(persons);
        f.wait();

        std::cout << "\nTest functions composition\n";

        testComposable(persons);

        return 0;
    }
}
