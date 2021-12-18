/*
 * Type2.hxx
 *
 *  Created on: Dec 18, 2021
 *  Author: <a href="mailto:damirlj@yahoo.com">Damir Ljubic</a>
 */

#ifndef TUTORIAL_TYPE_ERASURE_EXTERNAL_POLYMORPHISM_TYPE2_HXX_
#define TUTORIAL_TYPE_ERASURE_EXTERNAL_POLYMORPHISM_TYPE2_HXX_

#include <string>

namespace test::ep
{

    class Type2 final
    {
        public:
            Type2() = default;
            Type2(std::string value, int a) : m_value(std::move(value)), m_a(a) {}
            Type2(std::string value) : m_value(std::move(value)) {}

            /**
             * Logging method - which signature needs to be adapted
             *
             * @return  Message to log
             */
            std::string print() const
            {
                return m_value + ", [a]=" + std::to_string(m_a);
            }

            std::string getValue() const {return m_value;}
            int get() const { return m_a;}

        private:
            std::string m_value {"n\a"};
            int m_a = 0;
    };

    /**
     * Signature adapter, as a free function
     *
     * @see Logger::log
     *
     * @param type  The type to log
     * @return      String representation of a type
     */
    std::string dump(const Type2& type)
    {
        return type.print();
    }


}


#endif /* TUTORIAL_TYPE_ERASURE_EXTERNAL_POLYMORPHISM_TYPE2_HXX_ */
