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
#pragma once

#include <boost/algorithm/string.hpp>
using std::string;

struct Action_Object {
    int action_id;
    int next_action;
    int prev_action;

    struct Area {
    std::string name;
    double latitude;
    double longitude;
    bool status;
    bool is_notify;
    } area;

    struct Cargo {
    std::string cargo_uuid;
    std::string name;
    } cargo;

    struct Vehicle {
    std::string veh_id;
    std::string name;
    } vehicle;
};