#ifndef INCLUDE_SIMULATED_PRESENCE_VECTOR_H_
#define INCLUDE_SIMULATED_PRESENCE_VECTOR_H_

namespace tmx
{
    namespace messages
    {
        namespace simulation
        {
            enum PRESENCE_VECTOR_TYPES
            {
                UNAVAILABLE = 0,
                ID_PRESENCE_VECTOR = 1,
                POSE_PRESENCE_VECTOR = 2,
                VELOCITY_PRESENCE_VECTOR = 4,
                VELOCITY_INST_PRESENCE_VECTOR = 8,
                SIZE_PRESENCE_VECTOR = 16,
                CONFIDENCE_PRESENCE_VECTOR = 32,
                OBJECT_TYPE_PRESENCE_VECTOR = 64,
                BSM_ID_PRESENCE_VECTOR = 128,
                DYNAMIC_OBJ_PRESENCE = 256,
                PREDICTION_PRESENCE_VECTOR = 512
            };
        }
    }

}; // namespace tmx
#endif