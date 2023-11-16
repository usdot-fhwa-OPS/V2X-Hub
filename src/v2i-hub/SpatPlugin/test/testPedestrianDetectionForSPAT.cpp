#include <gtest/gtest.h>
#include <PedestrianDetectionForSPAT.h>

using namespace tmx::messages;

static std::shared_ptr<SPAT> createSPAT() {
    auto spatPtr = std::make_shared<SPAT>();
    auto intersectionState = (IntersectionState *)calloc(1, sizeof(IntersectionState));
    intersectionState->id.id = 1;
    ASN_SEQUENCE_ADD(&(spatPtr->intersections), intersectionState);  

    for (int i = 1; i <= 2; i++) {
        auto movementState = (MovementState *)calloc(1, sizeof(MovementState));
        movementState->signalGroup = i;
        ASN_SEQUENCE_ADD(&(intersectionState->states), movementState);

        auto movementEvent = (MovementEvent* )calloc(1, sizeof(MovementEvent));
        movementEvent->eventState = 
            i % 2 == 0 ? MovementPhaseState_protected_Movement_Allowed : MovementPhaseState_stop_And_Remain;
        ASN_SEQUENCE_ADD(&(movementState->state_time_speed), movementEvent);
    }
    return spatPtr;
}

TEST(PedestrianDetectionForSPAT, updateEncodedSpat_testIncompleteSpat)
{
    PedestrianDetectionForSPAT pedSPAT;
    SpatEncodedMessage spatEncoded;
    // create incomplete message
    auto spatPtr = std::make_shared<SPAT>();
    auto spatMessage = std::make_shared<tmx::messages::SpatMessage>(spatPtr);
    bool execptionCaught = false;
    try {
        pedSPAT.updateEncodedSpat(spatEncoded, spatMessage, "");
    } catch (std::exception & e) {
        // should not encode but should get here
        execptionCaught = true;
    }
    EXPECT_EQ(execptionCaught, true);
}

TEST(PedestrianDetectionForSPAT, updateEncodedSpat)
{
    PedestrianDetectionForSPAT pedSPAT;
    // set up a J2735 SPAT to use and add to
    auto spatPtr = createSPAT();

    // first encode the message as is
    tmx::messages::SpatEncodedMessage spatEncoded;
    tmx::messages::MessageFrameMessage frame(spatPtr);
    spatEncoded.set_data(tmx::messages::TmxJ2735EncodedMessage<SPAT>::encode_j2735_message<tmx::messages::codec::uper<tmx::messages::MessageFrameMessage>>(frame));
    auto originalSpatHex = spatEncoded.get_payload_str();
    EXPECT_STRNE(originalSpatHex.c_str(), ""); 

    // test once with no ped zones and ensure it is the same
    {
        SpatEncodedMessage spatEncoded;
        auto spatMessage = std::make_shared<tmx::messages::SpatMessage>(spatPtr);
        pedSPAT.updateEncodedSpat(spatEncoded, spatMessage, "");
        EXPECT_STREQ(spatEncoded.get_payload_str().c_str(), originalSpatHex.c_str()); 
    }

    // test with ped zones to see that it updates
    {
        SpatEncodedMessage spatEncoded;
        auto spatMessage = std::make_shared<tmx::messages::SpatMessage>(spatPtr);
        pedSPAT.updateEncodedSpat(spatEncoded, spatMessage, "2");
        // check hex is not equal to orginal
        EXPECT_STRNE(spatEncoded.get_payload_str().c_str(), originalSpatHex.c_str()); 
        // check that the ped detect was added
        ASSERT_EQ(spatPtr->intersections.list.count, 1);
        ASSERT_EQ(spatPtr->intersections.list.array[0]->states.list.count, 2);
        EXPECT_EQ(spatPtr->intersections.list.array[0]->states.list.array[0]->signalGroup, 1);
        EXPECT_EQ(spatPtr->intersections.list.array[0]->states.list.array[1]->signalGroup, 2);
        ASSERT_NE(spatPtr->intersections.list.array[0]->maneuverAssistList, nullptr);
        EXPECT_NE(spatPtr->intersections.list.array[0]->maneuverAssistList->list.count, 0);
        ASSERT_NE(spatPtr->intersections.list.array[0]->maneuverAssistList->list.array[0]->pedBicycleDetect, nullptr);
        EXPECT_EQ(*(spatPtr->intersections.list.array[0]->maneuverAssistList->list.array[0]->pedBicycleDetect), 1);
    }
}
