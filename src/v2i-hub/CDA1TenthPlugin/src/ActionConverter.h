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

#include <boost/property_tree/ptree.hpp>
#include <boost/algorithm/string.hpp>
#include "CDA1TenthPlugin.h"

using pt = boost::property_tree::ptree;
using std::string;

namespace CDA1TenthPlugin
{
    class ActionConverter
    {
        public:
          ActionConverter() = default;
          ~ActionConverter() = default;


          static pt toTree(const CDA1TenthPlugin::Action_Object &cda1t_obj);

          CDA1TenthPlugin::Action_Object toActionObject(const pt &json_payload );
    };


} // namespace CDA1TenthPlugin