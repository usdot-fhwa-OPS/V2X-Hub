// #include "jsoncpp/json/json.h"
// #include <tmx/j2735_messages/SensorDataSharingMessage.hpp>

// namespace CARMAStreetsPlugin
// {

//     // Template to use when created shared pointer objects for optional data
//     template <typename T>
//     T *create_store_shared(std::vector<std::shared_ptr<void>> &shared_pointers)
//     {
//         auto obj_shared = std::make_shared<T>();
//         shared_pointers.push_back(obj_shared);
//         return obj_shared.get();
//     }

//     // Template for shared pointers with array elements
//     template <typename T>
//     T *create_store_shared_array(std::vector<std::shared_ptr<void>> &shared_pointers, int size)
//     {
//         std::shared_ptr<T[]> array_shared(new T[size]{0});
//         shared_pointers.push_back(array_shared);
//         return array_shared.get();
//     }


//     class J3224ToSDSMJsonConverter
//     {
//         public:

//         J3224ToSDSMJsonConverter() = default;
//         ~J3224ToSDSMJsonConverter() = default;

//         /**
//          * @brief Convert the J3224 SensorDataSharingMessage into JSON format.
//          * @param sdsmMsgPtr The input is a constant SensorDataSharingMessage pointer. This prevent any modification to the original SDSM
//          * @param sdsmJson Pass by reference to allow the method to populate this object with the SensorDataSharingMessage data.
//          */
//         void convertJ3224ToSDSMJSON(const std::shared_ptr<SensorDataSharingMessage> sdsmMsgPtr, Json::Value &sdsmJson) const;


//     };

// }