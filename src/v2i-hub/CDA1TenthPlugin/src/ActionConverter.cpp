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
    ptree ActionConverter::toTree( const Action_Object &cda1t_obj) {
        ptree json_payload;
        json_payload.put("veh_id", cda1t_obj.vehicle.veh_id );
        json_payload.put("cargo_uuid", cda1t_obj.cargo.cargo_uuid );
        ptree destination;
        destination.put<double>("latitude", cda1t_obj.area.latitude);
        destination.put<double>("longitude", cda1t_obj.area.longitude);
        json_payload.put_child("destination",destination);
        json_payload.put("operation", cda1t_obj.area.name);
        json_payload.put("action_id", cda1t_obj.action_id );
        return json_payload;
    }

    Action_Object ActionConverter::toActionObject( const ptree &json_payload ) {
        std::unique_ptr<Action_Object> action_obj( new Action_Object());
        try {
            action_obj->vehicle.veh_id = json_payload.get_child("veh_id").get_value<std::string>();
            boost::optional<const ptree& > child = json_payload.get_child_optional( "action_id" );
            if( !child )
            {
                FILE_LOG(logINFO) << "No action_id present! This is the vehicle's first action" << std::endl;
            }
            else {
                action_obj->action_id = child.get_ptr()->get_value<int>();
                // For eventually tracking of completed actions
                action_obj->area.name = json_payload.get_child("operation").get_value<string>();
                child = json_payload.get_child_optional("cargo_uuid");
                if ( child ) {
                    action_obj->cargo.cargo_uuid = json_payload.get_child("cargo_uuid").get_value<string>();
                }
                // child = json_payload.get_child_optional("location");
                // if ( child ) {
                // 	ptree location = json_payload.get_child("location");
                // 	action_obj->location_lat =location.get_child("latitude").get_value<double>();
                // 	action_obj->location_long = location.get_child("longitude").get_value<double>();
                // }

            }
            return *action_obj.get();

        }
        catch( const ptree_error &e ) {
            FILE_LOG(logERROR) << "Error parsing Mobility Operation payload: " << e.what() << std::endl;
            return *action_obj.get();
        }
    }


}