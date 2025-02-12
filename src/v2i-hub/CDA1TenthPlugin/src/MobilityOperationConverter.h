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
#include "ActionObject.h"

using std::string;
using namespace boost::property_tree;

namespace CDA1TenthPlugin
{
  class MobilityOperationConverter
  {
    public:
    MobilityOperationConverter() = default;
    ~MobilityOperationConverter() = default;

    /**
    * Method to create MobilityOperation XML ptree.
    * 
    * @param ptree JSON payload
    * @param strategy String representation of the current mobility operaiton strategy
    * @return MobilityOperation message XML ptree
    */
    static ptree toXML( const ptree &json_payload, const std::string &strategy);

  };

} // namespace CDA1TenthPlugin