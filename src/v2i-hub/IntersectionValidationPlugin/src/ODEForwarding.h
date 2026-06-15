/**
 * Copyright (C) 2026 LEIDOS.
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
#include <map>
#include "RevisionCounterValidator.h"

namespace IntersectionValidation
{
    /**
     * @brief What to do with a validated message: whether to re-broadcast it,
     *        and any revision corrections to apply before doing so.
     */
    struct ODEForwarding
    {
        bool shouldForward = false;
        std::map<int, int> corrections; // intersectionId -> corrected per-intersection revision
        int msgRevisionCorrection = -1;  // -1 = none; MAP msgIssueRevision corrected value (MAP only)
    };

    /**
     * @brief Decide whether a validated message should be forwarded and what
     *        revision corrections to apply. Used for BOTH SPaT and MAP:
     *        the message-level (msgIssueRevision) branch only fires when
     *        result.hasMsgRevision is true, which is MAP-only.
     */
    ODEForwarding planForwarding(const RevisionCounterResult &result);
}