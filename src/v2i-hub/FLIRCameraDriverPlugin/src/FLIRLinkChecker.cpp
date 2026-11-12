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
#include "FLIRLinkTime.hpp"

#include <PluginLog.h>
#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace net = boost::asio;
using tcp = net::ip::tcp;

namespace FLIRCameraDriverPlugin
{
    namespace
    {
        // One connection attempt. Shared by its connect and deadline handlers; whichever runs first decides
        // the outcome.
        struct PendingCheck
        {
            PendingCheck(net::io_context &ioc, uint64_t startMs) : socket(ioc), deadline(ioc), startMs(startMs)
            {
            }
            tcp::socket socket;
            net::steady_timer deadline;
            uint64_t startMs;
            bool done = false;
        };
    }

    std::string LinkCheckTarget::describe() const
    {
        const auto address = host + ":" + std::to_string(port);
        return isCamera() ? "camera " + sensorId + " (" + address + ")" : "control host " + address;
    }

    LinkCheckHistory::LinkCheckHistory(uint64_t keepMs) : keepMs_(keepMs)
    {
    }

    std::optional<LinkCheckEvent> LinkCheckHistory::add(uint64_t startMs, uint64_t endMs, CheckOutcome outcome,
        const std::string &error)
    {
        checks_.push_back({startMs, endMs, outcome});
        while (checks_.front().endMs + keepMs_ < endMs)
        {
            checks_.pop_front();
        }

        if (outcome == CheckOutcome::Answered)
        {
            if (!failureRun_)
            {
                return std::nullopt;
            }
            auto answeredAgain = *failureRun_;
            failureRun_.reset();
            answeredAgain.answeredAgain = true;
            answeredAgain.answeredAtMs = startMs;
            return answeredAgain;
        }

        failedChecks_++;
        if (failureRun_)
        {
            failureRun_->failedChecks++;
            return std::nullopt;
        }
        LinkCheckEvent stoppedAnswering;
        stoppedAnswering.failedSinceMs = startMs;
        stoppedAnswering.failedChecks = 1;
        stoppedAnswering.outcome = outcome;
        stoppedAnswering.error = error;
        failureRun_ = stoppedAnswering;
        return stoppedAnswering;
    }

    LinkCheckSummary LinkCheckHistory::summarize(uint64_t fromMs, uint64_t toMs) const
    {
        LinkCheckSummary summary;
        for (const auto &check : checks_)
        {
            if (check.startMs <= toMs && check.endMs >= fromMs)
            {
                summary.checks++;
                if (check.outcome == CheckOutcome::NoAnswer)
                {
                    summary.noAnswer++;
                }
                else if (check.outcome == CheckOutcome::Refused)
                {
                    summary.refused++;
                }
            }
        }
        return summary;
    }

    uint64_t LinkCheckHistory::getFailedChecks() const
    {
        return failedChecks_;
    }

    struct FLIRLinkChecker::TargetState
    {
        explicit TargetState(net::io_context &ioc) : timer(ioc)
        {
        }
        // Unset if the host could not be resolved; the target is then not checked
        std::optional<tcp::endpoint> endpoint;
        // Starts the next check
        net::steady_timer timer;
        std::chrono::steady_clock::time_point nextStart;
        // Guarded by FLIRLinkChecker::mutex_
        LinkCheckHistory history;
    };

    FLIRLinkChecker::FLIRLinkChecker(std::vector<LinkCheckTarget> targets, std::chrono::milliseconds interval)
        : targets_(std::move(targets)), interval_(interval)
    {
        for (size_t i = 0; i < targets_.size(); i++)
        {
            states_.push_back(std::make_unique<TargetState>(ioc_));
        }
    }

    FLIRLinkChecker::~FLIRLinkChecker()
    {
        stop();
    }

    void FLIRLinkChecker::start()
    {
        tcp::resolver resolver(ioc_);
        for (size_t i = 0; i < targets_.size(); i++)
        {
            boost::system::error_code ec;
            const auto results = resolver.resolve(targets_[i].host, std::to_string(targets_[i].port), ec);
            if (ec || results.empty())
            {
                PLOG(tmx::utils::logERROR) << "FLIR link check: cannot resolve " << targets_[i].describe() << ": "
                    << ec.message() << ". It will not be checked.";
                continue;
            }
            states_[i]->endpoint = results.begin()->endpoint();
            states_[i]->nextStart = std::chrono::steady_clock::now();
            startCheck(i);
        }
        thread_ = std::thread([this] { ioc_.run(); });
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

    std::optional<LinkCheckSummary> FLIRLinkChecker::summarizeCamera(const std::string &sensorId, uint64_t fromMs,
        uint64_t toMs) const
    {
        std::scoped_lock lock{mutex_};
        for (size_t i = 0; i < targets_.size(); i++)
        {
            if (targets_[i].isCamera() && targets_[i].sensorId == sensorId && states_[i]->endpoint)
            {
                return states_[i]->history.summarize(fromMs, toMs);
            }
        }
        return std::nullopt;
    }

    std::vector<LinkCheckSummary> FLIRLinkChecker::summarizeControls(uint64_t fromMs, uint64_t toMs) const
    {
        std::scoped_lock lock{mutex_};
        std::vector<LinkCheckSummary> summaries;
        for (size_t i = 0; i < targets_.size(); i++)
        {
            if (!targets_[i].isCamera() && states_[i]->endpoint)
            {
                summaries.push_back(states_[i]->history.summarize(fromMs, toMs));
            }
        }
        return summaries;
    }

    uint64_t FLIRLinkChecker::getFailedChecks(bool cameras) const
    {
        std::scoped_lock lock{mutex_};
        uint64_t failedChecks = 0;
        for (size_t i = 0; i < targets_.size(); i++)
        {
            if (targets_[i].isCamera() == cameras)
            {
                failedChecks += states_[i]->history.getFailedChecks();
            }
        }
        return failedChecks;
    }

    std::vector<LinkCheckEvent> FLIRLinkChecker::takeEvents()
    {
        std::scoped_lock lock{mutex_};
        return std::exchange(events_, {});
    }

    std::vector<LinkCheckTarget> FLIRLinkChecker::parseControlHosts(const std::string &list)
    {
        std::vector<LinkCheckTarget> targets;
        std::stringstream stream(list);
        std::string entry;
        while (std::getline(stream, entry, ','))
        {
            const auto first = entry.find_first_not_of(" \t");
            if (first == std::string::npos)
            {
                continue;
            }
            entry = entry.substr(first, entry.find_last_not_of(" \t") - first + 1);
            LinkCheckTarget target;
            const auto colon = entry.find(':');
            target.host = entry.substr(0, colon);
            if (colon != std::string::npos)
            {
                target.port = parsePort(entry.substr(colon + 1));
            }
            if (target.host.empty())
            {
                throw std::invalid_argument("Missing host in \"" + entry + "\"");
            }
            targets.push_back(target);
        }
        return targets;
    }

    uint16_t FLIRLinkChecker::parsePort(const std::string &text)
    {
        size_t parsed = 0;
        unsigned long port = 0;
        try
        {
            port = std::stoul(text, &parsed);
        }
        catch (const std::exception &)
        {
            parsed = 0;
        }
        if (parsed == 0 || parsed != text.size() || port == 0 || port > 65535)
        {
            throw std::invalid_argument("Invalid port \"" + text + "\"");
        }
        return static_cast<uint16_t>(port);
    }

    void FLIRLinkChecker::scheduleCheck(size_t target)
    {
        auto &state = *states_[target];
        // The next check starts one interval after the previous one started, or now if that time has passed
        state.nextStart = std::max(state.nextStart + interval_, std::chrono::steady_clock::now());
        state.timer.expires_at(state.nextStart);
        state.timer.async_wait([this, target](const boost::system::error_code &ec) {
            if (!ec)
            {
                startCheck(target);
            }
        });
    }

    void FLIRLinkChecker::startCheck(size_t target)
    {
        auto check = std::make_shared<PendingCheck>(ioc_, wallClockMs());
        check->deadline.expires_after(interval_);
        check->deadline.async_wait([this, target, check](const boost::system::error_code &ec) {
            if (ec || check->done)
            {
                return;
            }
            check->done = true;
            boost::system::error_code ignored;
            check->socket.close(ignored);
            finishCheck(target, check->startMs, CheckOutcome::NoAnswer,
                "no answer within " + std::to_string(interval_.count()) + " ms");
        });
        check->socket.async_connect(*states_[target]->endpoint, [this, target, check](const boost::system::error_code &ec) {
            if (check->done)
            {
                // The deadline passed first
                return;
            }
            check->done = true;
            check->deadline.cancel();
            boost::system::error_code ignored;
            check->socket.close(ignored);
            if (!ec)
            {
                finishCheck(target, check->startMs, CheckOutcome::Answered, "");
            }
            else if (ec == net::error::connection_refused)
            {
                // A reset proves the path to a control host is up, so any of its ports will do
                finishCheck(target, check->startMs,
                    targets_[target].isCamera() ? CheckOutcome::Refused : CheckOutcome::Answered, ec.message());
            }
            else
            {
                finishCheck(target, check->startMs, CheckOutcome::NoAnswer, ec.message());
            }
        });
    }

    void FLIRLinkChecker::finishCheck(size_t target, uint64_t startMs, CheckOutcome outcome, const std::string &error)
    {
        {
            std::scoped_lock lock{mutex_};
            if (auto event = states_[target]->history.add(startMs, wallClockMs(), outcome, error))
            {
                event->target = target;
                events_.push_back(*event);
            }
        }
        scheduleCheck(target);
    }
}
