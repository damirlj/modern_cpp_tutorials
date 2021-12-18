/*
 * Car.hxx
 *
 *  Created on: Dec 18, 2021
 *  Author: <a href="mailto:damirlj@yahoo.com">Damir Ljubic</a>
 */

#ifndef TUTORIAL_TYPE_ERASURE_TYPE_ERASURE_CAR_HXX_
#define TUTORIAL_TYPE_ERASURE_TYPE_ERASURE_CAR_HXX_

#include <string>
#include <iostream>

#include "DrivingType.hxx"


namespace test::erasure
{
    class Car final
    {
        public:

            Car() = default;
            explicit Car(std::string manufacture, std::string model) noexcept
                : m_manufacture(std::move(manufacture))
                , m_model(std::move(model))
            {}

            void drive(drive_type driveType) const
            {
                std::cout << "<Car>: driving mode=" << printDriveType(driveType) << '\n';
            }

            std::string getManufacture() const { return m_manufacture; }
            std::string getModel() const { return m_model;}

        private:
            std::string m_manufacture;
            std::string m_model;
    };

    std::ostream& operator << (std::ostream& os, const Car& car)
    {
       os << "<Car> manufacture=" << car.getManufacture() << ", model=" << car.getModel() << '\n';
       return os;
    }
}


#endif /* TUTORIAL_TYPE_ERASURE_TYPE_ERASURE_CAR_HXX_ */
