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
#include "FLIRLinkChecker.hpp"

#include <utility>

namespace FLIRCameraDriverPlugin
{
    std::string LinkCheckTarget::describe() const
    {
        const auto address = host + ":" + std::to_string(port);
        return isCamera() ? "camera " + sensorId + " (" + address + ")" : "control host " + address;
    }

    LinkCheckHistory::LinkCheckHistory(uint64_t keepMs) : keepMs_(keepMs)
    {
    }

    std::optional<LinkCheckEvent> LinkCheckHistory::add(uint64_t /*startMs*/, uint64_t /*endMs*/,
        CheckOutcome /*outcome*/, const std::string & /*error*/)
    {
        // TODO(PR 3, link checks): keep the check, drop checks older than keepMs_, count failures, and return an
        // event when a failed check starts a run of failures or an answered check ends one.
        return std::nullopt;
    }

    LinkCheckSummary LinkCheckHistory::summarize(uint64_t /*fromMs*/, uint64_t /*toMs*/) const
    {
        // TODO(PR 3, link checks): count the kept checks with startMs <= toMs and endMs >= fromMs, by outcome.
        return {};
    }

    uint64_t LinkCheckHistory::getFailedChecks() const
    {
        return failedChecks_;
    }

    // TODO(PR 3, link checks): resolved endpoint, next-check timer and LinkCheckHistory of one target.
    struct FLIRLinkChecker::TargetState
    {
    };

    FLIRLinkChecker::FLIRLinkChecker(std::vector<LinkCheckTarget> targets, std::chrono::milliseconds interval)
        : targets_(std::move(targets)), interval_(interval)
    {
    }

    FLIRLinkChecker::~FLIRLinkChecker()
    {
        stop();
    }

    void FLIRLinkChecker::start()
    {
        // TODO(PR 3, link checks): resolve each target once, schedule its first check and run ioc_ on thread_.
    }

    void FLIRLinkChecker::stop()
    {
        ioc_.stop();
        if (thread_.joinable())
        {
            thread_.join();
        }
    }

    const std::vector<LinkCheckTarget> &FLIRLinkChecker::getTargets() const
    {
        return targets_;
    }

    std::chrono::milliseconds FLIRLinkChecker::getInterval() const
    {
        return interval_;
    }

    std::optional<LinkCheckSummary> FLIRLinkChecker::summarizeCamera(const std::string & /*sensorId*/,
        uint64_t /*fromMs*/, uint64_t /*toMs*/) const
    {
        // TODO(PR 3, link checks): summarize the history of the camera target with this sensor id.
        return std::nullopt;
    }

    std::vector<LinkCheckSummary> FLIRLinkChecker::summarizeControls(uint64_t /*fromMs*/, uint64_t /*toMs*/) const
    {
        // TODO(PR 3, link checks): summarize the history of each control host target.
        return {};
    }

    uint64_t FLIRLinkChecker::getFailedChecks(bool /*cameras*/) const
    {
        // TODO(PR 3, link checks): sum the failed checks of the camera or the control host targets.
        return 0;
    }

    std::vector<LinkCheckEvent> FLIRLinkChecker::takeEvents()
    {
        std::scoped_lock lock{mutex_};
        return std::exchange(events_, {});
    }

    std::vector<LinkCheckTarget> FLIRLinkChecker::parseControlHosts(const std::string & /*list*/)
    {
        // TODO(PR 3, link checks): split on commas, trim, split host and optional port (default 80), and throw
        // std::invalid_argument for an empty host or a port outside 1-65535.
        return {};
    }

    void FLIRLinkChecker::scheduleCheck(size_t /*target*/)
    {
        // TODO(PR 3, link checks): start the next check one interval after the previous one started.
    }

    void FLIRLinkChecker::startCheck(size_t /*target*/)
    {
        // TODO(PR 3, link checks): async_connect with a fresh socket and a deadline of one interval. Answered on
        // success, Refused on connection_refused (Answered for a control host), NoAnswer on any other error or
        // the deadline. Close the socket and call finishCheck().
    }

    void FLIRLinkChecker::finishCheck(size_t /*target*/, uint64_t /*startMs*/, CheckOutcome /*outcome*/,
        const std::string & /*error*/)
    {
        // TODO(PR 3, link checks): add the result to the target's history under mutex_, keep any event, and
        // schedule the next check.
    }
}
