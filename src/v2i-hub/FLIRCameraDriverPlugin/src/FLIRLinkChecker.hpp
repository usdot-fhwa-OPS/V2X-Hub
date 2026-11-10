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
#include <boost/asio.hpp>
#include <chrono>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>

namespace FLIRCameraDriverPlugin
{
    /**
     * @brief Result of one link check, i.e. one attempt to open a TCP connection.
     */
    enum class CheckOutcome
    {
        // The connection was established within the check interval
        Answered,
        // The host replied with a reset: its network is up but nothing listens on the port. Only reported for
        // cameras; for a control host the reply proves the path is up, so it counts as Answered.
        Refused,
        // No reply within the check interval, or a network error such as "host unreachable"
        NoAnswer
    };

    /**
     * @brief A host the link checker connects to: a camera, or a control host elsewhere on the network.
     */
    struct LinkCheckTarget
    {
        // The camera's sensor id. Empty for a control host.
        std::string sensorId;
        std::string host;
        uint16_t port = 80;

        bool isCamera() const { return !sensorId.empty(); }
        // For logs, e.g. "camera FLIR_North (192.168.55.83:80)" or "control host 192.168.55.1:80"
        std::string describe() const;
    };

    /**
     * @brief Counts of one target's checks that overlapped a time window.
     */
    struct LinkCheckSummary
    {
        unsigned checks = 0;
        unsigned noAnswer = 0;
        unsigned refused = 0;

        unsigned answered() const { return checks - noAnswer - refused; }
    };

    /**
     * @brief A target stopped answering link checks, or answered again after failed checks.
     */
    struct LinkCheckEvent
    {
        // Index of the target in FLIRLinkChecker::getTargets()
        size_t target = 0;
        // False: the target stopped answering. True: it answered again, ending the run of failed checks.
        bool answeredAgain = false;
        // Start of the first failed check of the run (ms since epoch)
        uint64_t failedSinceMs = 0;
        // If answeredAgain: start of the check that was answered (ms since epoch)
        uint64_t answeredAtMs = 0;
        // If answeredAgain: number of failed checks in the run
        unsigned failedChecks = 0;
        // Outcome and error message of the first failed check of the run
        CheckOutcome outcome = CheckOutcome::NoAnswer;
        std::string error;
    };

    /**
     * @brief Recent link check results of one target and its current run of failed checks.
     *
     * Not thread safe.
     */
    class LinkCheckHistory
    {
        public:
            /**
             * @param keepMs How long results are kept, in ms.
             */
            explicit LinkCheckHistory(uint64_t keepMs = 15 * 60 * 1000);

            /**
             * @brief Records the result of one check.
             * @return An event if this check started or ended a run of failed checks.
             */
            std::optional<LinkCheckEvent> add(uint64_t startMs, uint64_t endMs, CheckOutcome outcome, const std::string &error);

            /**
             * @brief Counts the kept checks that overlapped [fromMs, toMs].
             */
            LinkCheckSummary summarize(uint64_t fromMs, uint64_t toMs) const;

            /**
             * @brief Number of failed (not answered or refused) checks recorded so far.
             */
            uint64_t getFailedChecks() const;

        private:
            struct Check
            {
                uint64_t startMs;
                uint64_t endMs;
                CheckOutcome outcome;
            };
            uint64_t keepMs_;
            std::deque<Check> checks_;
            // The run of failed checks in progress, if any
            std::optional<LinkCheckEvent> failureRun_;
            uint64_t failedChecks_ = 0;
    };

    /**
     * @brief Repeatedly opens a TCP connection to each target and closes it again, on its own thread, so that
     * the network path to each camera is checked independently of its data stream. See
     * docs/CameraLinkMonitoring.md.
     *
     * Each check may take at most one interval, and the next check of a target starts one interval after the
     * previous one started.
     */
    class FLIRLinkChecker
    {
        public:
            /**
             * @param targets Cameras and control hosts to check.
             * @param interval Time between checks of each target, and the time each check may take.
             */
            FLIRLinkChecker(std::vector<LinkCheckTarget> targets, std::chrono::milliseconds interval);
            ~FLIRLinkChecker();
            FLIRLinkChecker(const FLIRLinkChecker &) = delete;
            FLIRLinkChecker &operator=(const FLIRLinkChecker &) = delete;

            /**
             * @brief Resolves the targets and starts checking them on a new thread.
             */
            void start();
            /**
             * @brief Stops checking and joins the thread.
             */
            void stop();

            const std::vector<LinkCheckTarget> &getTargets() const;
            std::chrono::milliseconds getInterval() const;

            /**
             * @brief Counts the checks of the camera with this sensor id that overlapped [fromMs, toMs].
             * @return Nothing if the camera is not a target.
             */
            std::optional<LinkCheckSummary> summarizeCamera(const std::string &sensorId, uint64_t fromMs, uint64_t toMs) const;
            /**
             * @brief Counts the checks of each control host that overlapped [fromMs, toMs].
             */
            std::vector<LinkCheckSummary> summarizeControls(uint64_t fromMs, uint64_t toMs) const;
            /**
             * @brief Number of failed checks so far, of all cameras or of all control hosts.
             */
            uint64_t getFailedChecks(bool cameras) const;
            /**
             * @brief Returns the events since the last call and clears them.
             */
            std::vector<LinkCheckEvent> takeEvents();

            /**
             * @brief Parses a comma-separated "host[:port]" list of control hosts. The port defaults to 80.
             * @throws std::invalid_argument if an entry is malformed.
             */
            static std::vector<LinkCheckTarget> parseControlHosts(const std::string &list);

        private:
            struct TargetState;

            void scheduleCheck(size_t target);
            void startCheck(size_t target);
            void finishCheck(size_t target, uint64_t startMs, CheckOutcome outcome, const std::string &error);

            std::vector<LinkCheckTarget> targets_;
            std::chrono::milliseconds interval_;
            // Declared before the per-target timers and sockets that use it, so it is destroyed after them
            boost::asio::io_context ioc_;
            std::vector<std::unique_ptr<TargetState>> states_;
            // Guards the per-target histories and events_
            mutable std::mutex mutex_;
            std::vector<LinkCheckEvent> events_;
            std::thread thread_;
    };
}
