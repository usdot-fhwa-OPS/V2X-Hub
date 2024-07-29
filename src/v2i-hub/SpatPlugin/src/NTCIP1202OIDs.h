/**
 * Copyright (C) 2024 LEIDOS.
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

#include <string>
/**
 * @brief Contains OIDs (Object Identifier) described in NTCIP 1202 V3 (National Transporation Communication for ITS Protocol Object
 * Definitions for Actuated Signal Controllers (ASC) Interface) used for SPaT Plugin functionality.
 */
namespace NTCIP1202V3{
    /**
     * @brief OID for Intersection ID.
     */
    static const std::string INTERSECTION_ID = " 1.3.6.1.4.1.1206.4.2.1.17.1.2.1.2";
}
/**
 * @brief Contains OIDs (Object Identifier) described in NTCIP 1202 V3 (National Transporation Communication for ITS Protocol Object
 * Definitions for Actuated Signal Controllers (ASC) Interface) used for SPaT Plugin functionality.
 */
namespace NTCIP1202V2{
    /**
     * @brief OID for ENABLE_SPAT
     */
    static const std::string ENABLE_SPAT_OID = "1.3.6.1.4.1.1206.3.5.2.9.44.1.0";
}