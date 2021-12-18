/*
 * Vehicle.hxx
 *
 *  Created on: Dec 18, 2021
 *  Author: <a href="mailto:damirlj@yahoo.com">Damir Ljubic</a>
 */

#ifndef TUTORIAL_TYPE_ERASURE_TYPE_ERASURE_VEHICLE_HXX_
#define TUTORIAL_TYPE_ERASURE_TYPE_ERASURE_VEHICLE_HXX_

#include <string>
#include <memory>

#include "DrivingType.hxx"

namespace test::erasure
{
    class Vehicle final
    {
        private:

            /**
             * Set of behavioral affordances :interface(contract), that erased type needs
             * to satisfied in order to be a valid candidate - to be polymorphically
             * used with other non-related types ("external polymorphism").
             * @link http://www.dre.vanderbilt.edu/~schmidt/PDF/External-Polymorphism.pdf
             *
             * It's basically Duck typing: non-related types with the same behavior (interface),
             * that can be treated in the same manner: applying the same peace of code (algorithm)
             * @link https://en.wikipedia.org/wiki/Duck_typing
             */
            struct VehicleConcept
            {
                virtual ~VehicleConcept() = default;
                virtual void drive(drive_type type) const = 0;
                virtual void configure() = 0;

                /*
                 * Prototype design pattern
                 * Virtual copy-constructor required to support value semantics - so that
                 * enclosing type is copy-constructible: can be created on the stack
                 */
                virtual std::unique_ptr<VehicleConcept> clone() const = 0;

                protected:
                   VehicleConcept() = default;
            };

            /**
             * Private implementation of the interface (pimpl idiom)
             *
             * @tparam VehicleType  Type of the vehicle (car, truck, etc.)
             *                      Ducking type: non-related types that behave the same - implement
             *                      the same interface
             *
             * @tparam Configurator The vehicle platform specific configuration.
             *                      Dependency Injection: relying on the external service
             *                      which is injected through the c-tor
             */
            template <typename VehicleType, typename Configurator>
            class VehicleConceptImpl final : public VehicleConcept
            {
                public:
                    using vehicle_type = VehicleType;
                    using configurator_type = Configurator;

                    VehicleConceptImpl(vehicle_type&& vehicle, configurator_type&& configurator) noexcept :
                          m_vehicle(std::move(vehicle)) // if type is not movable: fall through, calling copy-constructor instead
                        , m_configurator(std::move(configurator))
                    {}

                    std::unique_ptr<VehicleConcept> clone() const override
                    {
                        return std::make_unique<VehicleConceptImpl>(*this);
                    }

                    void drive(drive_type type) const override
                    {
                        m_vehicle.drive(type);
                    }

                    void configure() override
                    {
                        m_configurator(m_vehicle);
                    }

                private:
                    vehicle_type m_vehicle;
                    configurator_type m_configurator;
            };

        public:

            // Templated c-tor of enclosing class as customization point

            template <typename VehicleType, typename Configurator>
            Vehicle(VehicleType&& vehicle, Configurator&& configurator) noexcept
                : m_vehicle(std::make_unique<VehicleConceptImpl<VehicleType, Configurator>>(
                        std::move(vehicle),
                        std::move(configurator)))
            {}

            // For supporting value semantics.
            // Copy functions are defined

            Vehicle(const Vehicle& other) : m_vehicle(other.m_vehicle->clone())
            {}

            Vehicle& operator = (const Vehicle& other)
            {
                m_vehicle = other.m_vehicle->clone();
                return *this;
            }

            Vehicle(Vehicle&&) = default;
            Vehicle& operator=(Vehicle&&) = default;

            // Non-virtual interface of the enclosing class

            void drive(drive_type type) const
            {
                if (m_vehicle) {m_vehicle->drive(type);}
            }

            void configure() const
            {
                if (m_vehicle) {m_vehicle->configure();}
            }

        private:
            mutable std::unique_ptr<VehicleConcept> m_vehicle; //pimpl idiom - reference to the private implementation
    };
}


#endif /* TUTORIAL_TYPE_ERASURE_TYPE_ERASURE_VEHICLE_HXX_ */
