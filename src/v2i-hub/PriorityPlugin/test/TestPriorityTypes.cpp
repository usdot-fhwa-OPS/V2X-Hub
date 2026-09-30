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

#include <gtest/gtest.h>
#include "PriorityTypes.hpp"

using namespace PriorityPlugin;

TEST(PriorityTypesTest, RequestStatusValues) {
    EXPECT_EQ(static_cast<uint8_t>(RequestStatus::idleNotValid),          1);
    EXPECT_EQ(static_cast<uint8_t>(RequestStatus::readyQueued),           2);
    EXPECT_EQ(static_cast<uint8_t>(RequestStatus::readyOverridden),       3);
    EXPECT_EQ(static_cast<uint8_t>(RequestStatus::activeProcessing),      4);
    EXPECT_EQ(static_cast<uint8_t>(RequestStatus::activeCancel),          5);
    EXPECT_EQ(static_cast<uint8_t>(RequestStatus::activeOverride),        6);
    EXPECT_EQ(static_cast<uint8_t>(RequestStatus::activeNotOverridden),   7);
    EXPECT_EQ(static_cast<uint8_t>(RequestStatus::closedCanceled),        8);
    EXPECT_EQ(static_cast<uint8_t>(RequestStatus::reserviceError),        9);
    EXPECT_EQ(static_cast<uint8_t>(RequestStatus::closedTimeToLiveError), 10);
    EXPECT_EQ(static_cast<uint8_t>(RequestStatus::closedTimerError),      11);
    EXPECT_EQ(static_cast<uint8_t>(RequestStatus::closedStrategyError),   12);
    EXPECT_EQ(static_cast<uint8_t>(RequestStatus::closedCompleted),       13);
    EXPECT_EQ(static_cast<uint8_t>(RequestStatus::activeAdjustNotNeeded), 14);
    EXPECT_EQ(static_cast<uint8_t>(RequestStatus::closedFlash),           15);
}

TEST(PriorityTypesTest, PriorityRequestEntries) {
    PriorityRequestEntry entry;
    EXPECT_EQ(entry.vehicleClassType, 10);
    EXPECT_EQ(entry.vehicleClassLevel, 1);
    EXPECT_EQ(entry.serviceStrategyNumber, 0);
    EXPECT_EQ(entry.timeOfServiceDesiredInPRS, 0u);
    EXPECT_EQ(entry.timeOfEstimatedDepartureInPRS, 0u);
    EXPECT_EQ(entry.statusInPRS, RequestStatus::idleNotValid);
    EXPECT_EQ(entry.ssmBroadcastCount, 0);
    EXPECT_EQ(entry.ssmLastStatus, RequestStatus::idleNotValid);
}

TEST(PriorityTypesTest, IsReadyXtest) {
    EXPECT_TRUE(IsReadyX(RequestStatus::readyQueued));
    EXPECT_TRUE(IsReadyX(RequestStatus::readyOverridden));
    EXPECT_FALSE(IsReadyX(RequestStatus::idleNotValid));
    EXPECT_FALSE(IsReadyX(RequestStatus::activeProcessing));
    EXPECT_FALSE(IsReadyX(RequestStatus::closedCompleted));
}

TEST(PriorityTypesTest, IsActiveXtest) {
    EXPECT_TRUE(IsActiveX(RequestStatus::activeProcessing));
    EXPECT_TRUE(IsActiveX(RequestStatus::activeCancel));
    EXPECT_TRUE(IsActiveX(RequestStatus::activeOverride));
    EXPECT_TRUE(IsActiveX(RequestStatus::activeNotOverridden));
    EXPECT_TRUE(IsActiveX(RequestStatus::activeAdjustNotNeeded));
    EXPECT_FALSE(IsActiveX(RequestStatus::readyQueued));
    EXPECT_FALSE(IsActiveX(RequestStatus::closedCompleted));
    EXPECT_FALSE(IsActiveX(RequestStatus::idleNotValid));
}

TEST(PriorityTypesTest, IsClosedXtest) {
    EXPECT_TRUE(IsClosedX(RequestStatus::closedCanceled));
    EXPECT_TRUE(IsClosedX(RequestStatus::closedTimeToLiveError));
    EXPECT_TRUE(IsClosedX(RequestStatus::closedTimerError));
    EXPECT_TRUE(IsClosedX(RequestStatus::closedStrategyError));
    EXPECT_TRUE(IsClosedX(RequestStatus::closedCompleted));
    EXPECT_TRUE(IsClosedX(RequestStatus::closedFlash));
    EXPECT_FALSE(IsClosedX(RequestStatus::reserviceError));
    EXPECT_FALSE(IsClosedX(RequestStatus::readyQueued));
    EXPECT_FALSE(IsClosedX(RequestStatus::activeProcessing));
    EXPECT_FALSE(IsClosedX(RequestStatus::idleNotValid));
}
