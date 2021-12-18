/*
 * Truck.hxx
 *
 *  Created on: Dec 18, 2021
 *      Author: <a href="mailto:damirlj@yahoo.com">Damir Ljubic</a>
 */

#ifndef TUTORIAL_TYPE_ERASURE_TYPE_ERASURE_TRUCK_HXX_
#define TUTORIAL_TYPE_ERASURE_TYPE_ERASURE_TRUCK_HXX_

#include <iostream>

#include "DrivingType.hxx"


namespace test::erasure
{
    class Truck final
    {
        public:

            Truck() = default;
            explicit Truck(std::string platform, int serial) noexcept
                : m_platform(std::move(platform))
                , m_serial(serial)
            {}

            void drive(drive_type driveType) const
            {
                if (driveType == drive_type::sport)
                {
                    std::cout << "<Truck> \"Sport mode\" is not supported\n";
                    return;
                }
                std::cout << "<Truck>: driving mode=" << printDriveType(driveType) << '\n';
            }

            std::string getPlatform() const { return m_platform; }
            int getSerial() const { return m_serial; }

       private:
            std::string m_platform;
            int m_serial = 0;
    };

    std::ostream& operator << (std::ostream& os, const Truck& truck)
    {
        os << "<Truck> platform=" << truck.getPlatform() << ", serial=" <<  truck.getSerial() << '\n';
        return os;
    }
}


#endif /* TUTORIAL_TYPE_ERASURE_TYPE_ERASURE_TRUCK_HXX_ */
