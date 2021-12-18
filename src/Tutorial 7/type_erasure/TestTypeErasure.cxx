/*
 * TestTypeErasure.cxx
 *
 *  Created on: Dec 18, 2021
 *  Author: <a href="mailto:damirlj@yahoo.com">Damir Ljubic</a>
 */

#include "TestTypeErasure.hxx"

#include <iostream>
#include <initializer_list>
#include <algorithm>

#include "Vehicle.hxx"
#include "Car.hxx"
#include "Truck.hxx"

namespace test::erasure
{
    template <typename VehicleType>
    struct Configurator
    {
        void operator()(VehicleType& vehicle)
        {
            // dummy - you would configure the platform specific engine type, transmission, etc.
            std::cout << "Configure: " << vehicle;
        }
    };


    int test()
    {
        std::initializer_list<Vehicle> vehicles = {
                Vehicle(Car{"Audi", "A3985"}, Configurator<Car>{}),
                Vehicle(Truck{"MQB_3273", 37}, Configurator<Truck>{})
        };

        std::for_each(vehicles.begin(), vehicles.end(), [](auto& vehicle){
            vehicle.configure();
            vehicle.drive(drive_type::normal);
        });

        return 0;
    }
}
