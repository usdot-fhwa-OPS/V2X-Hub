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
#include <boost/property_tree/json_parser.hpp>
#include <boost/optional/optional.hpp>
#include <boost/algorithm/string.hpp>
#include <PluginLog.h>
#include "ActionObject.h"

using std::string;

namespace CDA1TenthPlugin
{
  class ActionConverter
  {
      public:
        ActionConverter() = default;
        ~ActionConverter() = default;

      /**
       * Method to create port drayage payload JSON ptree using an Action Object.
       * 
       * @param action_obj Action Object.
       * @return json ptree
       */
      static boost::property_tree::ptree toTree(const Action_Object &action_obj);
      /**
       * Create Action_Object from ptree JSON.
       * 
       * @param json_payload CDA1Tenth JSON
       * @return Action_Object 
       */
      static Action_Object fromTree(const boost::property_tree::ptree &json_payload);

    };

} // namespace CDA1TenthPlugin