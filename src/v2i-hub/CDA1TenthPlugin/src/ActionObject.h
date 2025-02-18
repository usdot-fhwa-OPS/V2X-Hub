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
constexpr int INVALID_ACTION = -1;

struct Action_Object {
    int action_id = INVALID_ACTION;
    int next_action = INVALID_ACTION;
    int prev_action = INVALID_ACTION;
    bool is_first_action = false;

    struct Area {
        string name;
        double latitude = 0.0;
        double longitude = 0.0;
        string status;
        bool is_notify = false;
    } area;

    struct Cargo {
        string cargo_uuid;
        string name;
    } cargo;

    struct Vehicle {
        string veh_id;
        string name;
    } vehicle;
};