/*
 * ExternalPolimorphyism.hxx
 *
 *  Created on: Dec 18, 2021
 *  Author: <a href="mailto:damirlj@yahoo.com">Damir Ljubic</a>
 */

#ifndef TUTORIAL_TYPE_ERASURE_LOGGER_HXX_
#define TUTORIAL_TYPE_ERASURE_LOGGER_HXX_

#include <string>
#include <memory>

namespace test::ep
{
    /**
     * Adapter interface, for a classes for which defining the new common ancestor (base class)
     * in inheritance tree is not possible: due to, for example, fact that
     * these classes are imported from the (different) 3rd party libraries
     */
    struct Logging
    {
          virtual ~Logging() = default;
          virtual void log() const = 0;
        protected:
          Logging() = default;
    };

    /**
     * Wrapper type around the Logging interface
     *
     * @tparam T        The type to be logged
     * @tparam Logger   The specific logger (medium): Dependency Injection
     */
    template <typename T, typename Logger>
    class LoggingImpl final : public Logging
    {
        public:
            LoggingImpl(const T& type, const Logger& logger) noexcept :
              m_obj(type)
            , m_logger(logger)
            {}

            ~LoggingImpl() override = default;

            void log() const override
            {
                /*
                 * "dump()" is a signature adapter, here as a free function.
                 *
                 * Internally, it will call T specific logging function.
                 * Actually, it's modified (http://www.dre.vanderbilt.edu/~schmidt/PDF/External-Polymorphism.pdf)
                 * to return the object string representation, and use external logger medium.
                 *
                 * Alternatively, one could use function template as signature adapter
                 *
                 * template <typename T>
                 * std::string dump(const T& t)
                 * {
                 *     return t.toString(); // this would assume that most of the classes have already the same "java-like" signature
                 * }
                 *
                 * and for those types who don't share the same signature - we have overloading specialization
                 *
                 * std::string dump<A>(const A& a)
                 * {
                 *    return a.print();
                 * }
                 */

                m_logger.log(dump(m_obj));
            }

        private:
            const T& m_obj; // unmutable object to log
            const Logger& m_logger; // unmutable logger itself
    };
}


#endif /* TUTORIAL_TYPE_ERASURE_LOGGER_HXX_ */
