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
#include <gtest/gtest.h>
#include <stdexcept>
#include <thread>

#include "FLIRLinkChecker.hpp"
#include "FLIRLinkTime.hpp"

using namespace FLIRCameraDriverPlugin;
namespace net = boost::asio;
using tcp = net::ip::tcp;

TEST(FLIRLinkCheckerTest, historyReportsRunsOfFailedChecks)
{
    LinkCheckHistory history;
    EXPECT_FALSE(history.add(1000, 1001, CheckOutcome::Answered, ""));

    const auto stoppedAnswering = history.add(1200, 1400, CheckOutcome::NoAnswer, "no answer within 200 ms");
    ASSERT_TRUE(stoppedAnswering);
    EXPECT_FALSE(stoppedAnswering->answeredAgain);
    EXPECT_EQ(stoppedAnswering->failedSinceMs, 1200u);
    EXPECT_EQ(stoppedAnswering->outcome, CheckOutcome::NoAnswer);
    EXPECT_EQ(stoppedAnswering->error, "no answer within 200 ms");

    // Further failures continue the run
    EXPECT_FALSE(history.add(1400, 1600, CheckOutcome::NoAnswer, "no answer within 200 ms"));
    EXPECT_FALSE(history.add(1600, 1601, CheckOutcome::Refused, "Connection refused"));

    const auto answeredAgain = history.add(1800, 1801, CheckOutcome::Answered, "");
    ASSERT_TRUE(answeredAgain);
    EXPECT_TRUE(answeredAgain->answeredAgain);
    EXPECT_EQ(answeredAgain->failedSinceMs, 1200u);
    EXPECT_EQ(answeredAgain->answeredAtMs, 1800u);
    EXPECT_EQ(answeredAgain->failedChecks, 3u);
    EXPECT_EQ(answeredAgain->error, "no answer within 200 ms");

    EXPECT_FALSE(history.add(2000, 2001, CheckOutcome::Answered, ""));
    EXPECT_EQ(history.getFailedChecks(), 3u);
}

TEST(FLIRLinkCheckerTest, historySummarizesChecksOverlappingAWindow)
{
    LinkCheckHistory history;
    history.add(1000, 1001, CheckOutcome::Answered, "");
    history.add(1200, 1400, CheckOutcome::NoAnswer, "");
    history.add(1400, 1600, CheckOutcome::NoAnswer, "");
    history.add(1600, 1601, CheckOutcome::Refused, "");
    history.add(1800, 1801, CheckOutcome::Answered, "");

    const auto all = history.summarize(0, 5000);
    EXPECT_EQ(all.checks, 5u);
    EXPECT_EQ(all.noAnswer, 2u);
    EXPECT_EQ(all.refused, 1u);
    EXPECT_EQ(all.answered(), 2u);

    // A check that was still waiting for an answer when the window began counts
    const auto window = history.summarize(1300, 1650);
    EXPECT_EQ(window.checks, 3u);
    EXPECT_EQ(window.noAnswer, 2u);
    EXPECT_EQ(window.refused, 1u);

    EXPECT_EQ(history.summarize(1002, 1199).checks, 0u);
}

TEST(FLIRLinkCheckerTest, historyKeepsRecentChecksOnly)
{
    LinkCheckHistory history(1000);
    history.add(0, 10, CheckOutcome::NoAnswer, "");
    history.add(500, 510, CheckOutcome::Answered, "");
    history.add(1500, 1510, CheckOutcome::Answered, "");
    // The first check ended more than 1000 ms before the latest one
    EXPECT_EQ(history.summarize(0, 2000).checks, 2u);
    EXPECT_EQ(history.getFailedChecks(), 1u);
}

TEST(FLIRLinkCheckerTest, parseControlHosts)
{
    EXPECT_TRUE(FLIRLinkChecker::parseControlHosts("").empty());
    EXPECT_TRUE(FLIRLinkChecker::parseControlHosts(" , ").empty());

    const auto targets = FLIRLinkChecker::parseControlHosts(" 192.168.55.1:80, 192.168.55.64 ,gateway:8080");
    ASSERT_EQ(targets.size(), 3u);
    EXPECT_EQ(targets[0].host, "192.168.55.1");
    EXPECT_EQ(targets[0].port, 80);
    EXPECT_EQ(targets[1].host, "192.168.55.64");
    EXPECT_EQ(targets[1].port, 80);
    EXPECT_EQ(targets[2].host, "gateway");
    EXPECT_EQ(targets[2].port, 8080);
    EXPECT_FALSE(targets[0].isCamera());

    EXPECT_THROW(FLIRLinkChecker::parseControlHosts("192.168.55.1:http"), std::invalid_argument);
    EXPECT_THROW(FLIRLinkChecker::parseControlHosts("192.168.55.1:70000"), std::invalid_argument);
    EXPECT_THROW(FLIRLinkChecker::parseControlHosts("192.168.55.1:"), std::invalid_argument);
    EXPECT_THROW(FLIRLinkChecker::parseControlHosts(":80"), std::invalid_argument);
}

TEST(FLIRLinkCheckerTest, parsePort)
{
    EXPECT_EQ(FLIRLinkChecker::parsePort("80"), 80);
    EXPECT_EQ(FLIRLinkChecker::parsePort("65535"), 65535);
    EXPECT_THROW(FLIRLinkChecker::parsePort("0"), std::invalid_argument);
    EXPECT_THROW(FLIRLinkChecker::parsePort("-1"), std::invalid_argument);
    EXPECT_THROW(FLIRLinkChecker::parsePort("80a"), std::invalid_argument);
    EXPECT_THROW(FLIRLinkChecker::parsePort(""), std::invalid_argument);
}

TEST(FLIRLinkCheckerTest, describeTargets)
{
    EXPECT_EQ((LinkCheckTarget{"FLIR_North", "192.168.55.83", 80}).describe(), "camera FLIR_North (192.168.55.83:80)");
    EXPECT_EQ((LinkCheckTarget{"", "192.168.55.1", 80}).describe(), "control host 192.168.55.1:80");
}

TEST(FLIRLinkCheckerTest, checksLocalTargets)
{
    net::io_context ioc;
    const auto loopback = net::ip::make_address("127.0.0.1");
    // Answers: a listening socket. The kernel completes connections without accept().
    tcp::acceptor open(ioc, tcp::endpoint(loopback, 0));
    // Refuses: a port nothing listens on
    uint16_t closedPort = 0;
    {
        tcp::acceptor closed(ioc, tcp::endpoint(loopback, 0));
        closedPort = closed.local_endpoint().port();
    }
    // Does not answer: a listening socket whose accept queue is full, so the kernel drops connection requests
    tcp::acceptor full(ioc);
    full.open(tcp::v4());
    full.bind(tcp::endpoint(loopback, 0));
    full.listen(0);
    tcp::socket filler(ioc);
    filler.connect(full.local_endpoint());

    FLIRLinkChecker checker({
        {"OpenCamera", "127.0.0.1", open.local_endpoint().port()},
        {"ClosedCamera", "127.0.0.1", closedPort},
        {"FullCamera", "127.0.0.1", full.local_endpoint().port()},
        // A control host that refuses the connection answered: the reset proves the path is up
        {"", "127.0.0.1", closedPort},
    }, std::chrono::milliseconds(100));
    checker.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(550));
    checker.stop();

    const uint64_t nowMs = wallClockMs();
    const auto openCamera = checker.summarizeCamera("OpenCamera", 0, nowMs);
    ASSERT_TRUE(openCamera);
    EXPECT_GE(openCamera->checks, 4u);
    EXPECT_EQ(openCamera->answered(), openCamera->checks);

    const auto closedCamera = checker.summarizeCamera("ClosedCamera", 0, nowMs);
    ASSERT_TRUE(closedCamera);
    EXPECT_GE(closedCamera->checks, 4u);
    EXPECT_EQ(closedCamera->refused, closedCamera->checks);

    const auto fullCamera = checker.summarizeCamera("FullCamera", 0, nowMs);
    ASSERT_TRUE(fullCamera);
    EXPECT_GE(fullCamera->checks, 4u);
    EXPECT_EQ(fullCamera->noAnswer, fullCamera->checks);

    const auto controls = checker.summarizeControls(0, nowMs);
    ASSERT_EQ(controls.size(), 1u);
    EXPECT_GE(controls[0].checks, 4u);
    EXPECT_EQ(controls[0].answered(), controls[0].checks);

    EXPECT_FALSE(checker.summarizeCamera("UnknownCamera", 0, nowMs));
    EXPECT_EQ(checker.getFailedChecks(true), closedCamera->checks + fullCamera->checks);
    EXPECT_EQ(checker.getFailedChecks(false), 0u);

    // The closed and the full camera each stopped answering once
    const auto events = checker.takeEvents();
    ASSERT_EQ(events.size(), 2u);
    for (const auto &event : events)
    {
        EXPECT_FALSE(event.answeredAgain);
        EXPECT_FALSE(checker.getTargets()[event.target].sensorId == "OpenCamera");
    }
    EXPECT_TRUE(checker.takeEvents().empty());
}

TEST(FLIRLinkCheckerTest, reportsWhenACameraAnswersAgain)
{
    net::io_context ioc;
    const auto loopback = net::ip::make_address("127.0.0.1");
    uint16_t port = 0;
    {
        tcp::acceptor probe(ioc, tcp::endpoint(loopback, 0));
        port = probe.local_endpoint().port();
    }
    FLIRLinkChecker checker({{"Camera", "127.0.0.1", port}}, std::chrono::milliseconds(50));
    checker.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    // The camera's service comes back
    tcp::acceptor camera(ioc, tcp::endpoint(loopback, port));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    checker.stop();

    const auto events = checker.takeEvents();
    ASSERT_EQ(events.size(), 2u);
    EXPECT_FALSE(events[0].answeredAgain);
    EXPECT_EQ(events[0].outcome, CheckOutcome::Refused);
    EXPECT_TRUE(events[1].answeredAgain);
    EXPECT_GE(events[1].failedChecks, 2u);
    EXPECT_EQ(events[1].failedSinceMs, events[0].failedSinceMs);
    EXPECT_GT(events[1].answeredAtMs, events[1].failedSinceMs);
}
