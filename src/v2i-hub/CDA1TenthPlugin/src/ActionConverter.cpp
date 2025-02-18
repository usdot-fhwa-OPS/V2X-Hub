/**
 * Copyright (C) 2025 LEIDOS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this except in compliance with the License. You may obtain a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */
#include "ActionConverter.h"


namespace CDA1TenthPlugin
{
    ptree ActionConverter::toTree( const Action_Object &action_object) {
        ptree json_payload;
        json_payload.put<int>("action_id", action_object.action_id);
        json_payload.put("operation", action_object.area.name);
        json_payload.put("cmv_id", action_object.vehicle.veh_id);
        json_payload.put("cargo_name", action_object.cargo.name);
        json_payload.put("cargo_id", action_object.cargo.cargo_uuid);

        ptree destination;
        destination.put<double>("latitude", action_object.area.latitude);
        destination.put<double>("longitude", action_object.area.longitude);
        json_payload.put_child("destination", destination);
        return json_payload;
    }

    Action_Object ActionConverter::fromTree( const ptree &json_payload ) {
        Action_Object action_obj;
        action_obj.vehicle.veh_id = json_payload.get_child("cmv_id").get_value<std::string>();
        boost::optional<const ptree& > child = json_payload.get_child_optional( "action_id" );
        if( !child )
        {
            FILE_LOG(logINFO) << "No action_id present! This is the vehicle's first action" << std::endl;
            action_obj.is_first_action = true;
        }
        else if(child.get_ptr()->get_value<int>() == -1){
            FILE_LOG(logINFO) << "The action_id is present but uninitialized (-1). Check the action list." << std::endl;
        }
        else {
            action_obj.action_id = child.get_ptr()->get_value<int>();
            // For eventually tracking of completed actions
            action_obj.area.name = json_payload.get_child("operation").get_value<string>();
            child = json_payload.get_child_optional("cargo_id");
            if ( child ) {
                action_obj.cargo.cargo_uuid = json_payload.get_child("cargo_id").get_value<string>();
            }
            child = json_payload.get_child_optional("destination");
            if ( child ) {
                ptree destination = json_payload.get_child("destination");
                action_obj.area.latitude = destination.get_child("latitude").get_value<double>();
                action_obj.area.longitude = destination.get_child("longitude").get_value<double>();
            }

        }
        return action_obj;

    }
}