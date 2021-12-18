/*
 * Type1.hxx
 *
 *  Created on: Dec 18, 2021
 *  Author: <a href="mailto:damirlj@yahoo.com">Damir Ljubic</a>
 */

#ifndef TUTORIAL_TYPE_ERASURE_TYPE1_HXX_
#define TUTORIAL_TYPE_ERASURE_TYPE1_HXX_

#include <string>

namespace test::ep
{
    class Type1 final
    {
        public:

            Type1() = default;
            Type1(int value) : m_value(value) {}

            /**
             * Logging method, which signature needs to be unified - adapted
             *
             * @return  The message to be logged
             */
            std::string toString() const
            {
                return std::to_string(m_value);
            }

            int get() const {return m_value;}

        private:
            int m_value = 0;
    };

    /**
     * Signature adapter, as free function
     *
     * @param type  Type to be logged
     * @return      String representation of a type
     */
    std::string dump(const Type1& type)
    {
        return type.toString();
    }
}


#endif /* TUTORIAL_TYPE_ERASURE_TYPE1_HXX_ */
