/*
 * DrivingType.hxx
 *
 *  Created on: Dec 18, 2021
 *  Author: <a href="mailto:damirlj@yahoo.com">Damir Ljubic</a>
 */

#ifndef TUTORIAL_TYPE_ERASURE_TYPE_ERASURE_DRIVINGTYPE_HXX_
#define TUTORIAL_TYPE_ERASURE_TYPE_ERASURE_DRIVINGTYPE_HXX_

#include <cstdint>
#include <string>

namespace test::erasure
{
    using drive_type = enum class DriveType : std::uint8_t
    {
        normal = 0,
        eco,
        sport
    };

   std::string printDriveType (test::erasure::drive_type type)
   {

       #define ENUM_VALUE(x) case(x) : return #x

       switch (type)
       {
           ENUM_VALUE(drive_type::eco);
           ENUM_VALUE(drive_type::normal);
           ENUM_VALUE(drive_type::sport);
           default: break;
       }

       return "<n/a>";
   }
}



#endif /* TUTORIAL_TYPE_ERASURE_TYPE_ERASURE_DRIVINGTYPE_HXX_ */
