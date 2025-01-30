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
    pt toTree(const Action_Object &cda1t_obj)
    {

    }

    CDA1TenthPlugin::Action_Object toActionObject(const pt &json_payload )
    {
        std::unique_ptr<Action_Object> rtn( new CDA1TenthPlugin::Action_Object());

        return *rtn;

    }


}