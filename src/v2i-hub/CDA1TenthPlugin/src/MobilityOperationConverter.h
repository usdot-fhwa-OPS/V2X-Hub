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

#include <tmx/j2735_messages/testMessage03.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <tmx/j2735_messages/J2735MessageFactory.hpp>

using pt = boost::property_tree::ptree;
using std::string;
namespace CDA1TenthPlugin
{
  class MobilityOperationConverter
  {
    public:
    MobilityOperationConverter() = default;
    ~MobilityOperationConverter() = default;

    /***
     * @brief Convert a tsm3Message (MOM) to a JSON ptree
     * @param  mobility_operation_message tsm3Message (MOM)
     * @return JSON ptree
     */
    static pt toTree(const tmx::messages::tsm3Message &mobility_operation_msg);
    /**
    * Method to create MobilityOperation XML ptree.
    * 
    * @param ptree JSON payload
    * @return MobilityOperation message XML ptree
    */
    static pt fromTree( const pt &json_payload);
    /**
     * @brief Convert a JSON ptree to a JSON string
     * @param tree JSON ptree
     * @return MOM string in JSON format
     */
    static string toJsonString(const pt &tree);

  };


}