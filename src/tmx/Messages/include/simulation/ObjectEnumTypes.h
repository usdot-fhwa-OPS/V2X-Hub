#ifndef INCLUDE_SIMULATED_OBJECT_TYPE_H_
#define INCLUDE_SIMULATED_OBJECT_TYPE_H_

namespace tmx
{
    namespace messages
    {
        namespace simulation
        {
            // #used for object type
            enum OBJECT_TYPES
            {
                UNKNOWN = 0,
                SMALL_VEHICLE = 1,
                LARGE_VEHICLE = 2,
                MOTORCYCLE = 3,
                PEDESTRIAN = 4
            };
        }
    }

}; // namespace tmx
#endif