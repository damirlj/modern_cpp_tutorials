/*
 * TestTuples.cxx
 *
 *  Created on: Sep 25, 2021
 *      Author: <a href="mailto:damirlj@yahoo.com">Damir Ljubic</a>
 */


#include <string>
#include <vector>
#include <algorithm>

#include "TestTuples.hxx"
#include "Setter.hxx"


#define CHECK_ENUM(x) case (x) : return #x

namespace test::tuples
{
    using gender_t = enum class Gender
            {
                female = 0,
                male
            };

    std::string printEnum(gender_t gender)
    {
        switch(gender)
        {
            CHECK_ENUM(gender_t::female);
            CHECK_ENUM(gender_t::male);
        }

        return "n/a";
    }

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

    template <typename T>
    struct Optional
    {
        T value;
        bool valid;

//        Optional(T& t) : value(t), valid(true) // implicit convertible
//        {}
//
//        Optional(T&& t) : value(std::move(t)), valid(true)
//        {}
    };

    template <typename T>
    std::ostream& operator << (std::ostream& out, const Optional<T>& optional)
    {
        return out << "Valid=" << std::boolalpha << optional.valid
                   << ", Value=" << optional.value;
    }


    int testTuples()
    {
        int age;
        std::string name;
        std::vector<int> arr;
        Optional<std::string> shoolClass;
        // Optional<std::string> shoolClass {""}; // with proper c-tor

        std::cout << "Universal setter:\n\n";

        Setter setter {age, name, arr, shoolClass};
        setter.set<0>(7)
              .set<1>("Alex")
              .set<2>(std::vector<int>{1, 2, 3})
              .set<3>(Optional<std::string>{.value = std::string("1D")})
              //.set<3>(std::string("1D")) //with proper c-tor
              ;

        setter.print2Console();

        std::cout << "\nSetting by references:\n\n";

        std::cout << "age=" << age << '\n';
        std::cout << "name=" << name << '\n';
        std::cout << "arr=\n";
        print(arr);
        std::cout << "shoolClass=" << shoolClass << '\n';

        std::cout << "\nLexicographical ordering:\n\n";

        std::vector<Person> persons { {45, "Steven", gender_t::male},
            {7, "Alex", gender_t::male},
            {45, "Mary", gender_t::female}
        };

        std::sort(persons.begin(), persons.end());
        print(persons);

        return 0;
    }
}

