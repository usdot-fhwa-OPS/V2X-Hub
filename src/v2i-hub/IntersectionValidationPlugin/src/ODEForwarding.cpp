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

#include "ODEForwarding.h"

namespace IntersectionValidation
{
    ODEForwarding planForwarding(const RevisionCounterResult &result)
    {
        ODEForwarding plan;

        // First message
        if (!result.comparisonPerformed)
        {
            plan.shouldForward = true;
            return plan;
        }

        bool anyContentChanged = false;

        // Per-intersection revision
        for (const auto &change : result.intersectionChanges)
        {
            if (change.contentChanged || change.revisionChanged)
            {
                plan.shouldForward = true;

                // content changed but revision did not increment -> correct it
                if (change.contentChanged && !change.revisionChanged)
                {
                    plan.corrections[change.id] = (change.currentRevision + 1) % 128;
                }
            }
            if (change.contentChanged)
            {
                anyContentChanged = true;
            }
        }

        // Message-level msgIssueRevision
        if (result.hasMsgRevision)
        {
            if (anyContentChanged)
            {
                plan.shouldForward = true;
                // msgIssueRevision must change if any fields change
                if (!result.msgRevisionChanged)
                {
                    plan.msgRevisionCorrection = (result.currentMsgRevision + 1) % 128;
                }
            }
            else if (result.msgRevisionChanged)
            {
                plan.shouldForward = true;
            }
        }

        return plan;
    }
}