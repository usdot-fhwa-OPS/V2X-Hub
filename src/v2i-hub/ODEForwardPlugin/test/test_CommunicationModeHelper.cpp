#include <gtest/gtest.h>
#include "CommunicationModeHelper.h"

namespace ODEForwardPlugin{
    class test_CommunicationModeHelper: public ::testing::Test{
        protected:
            std::shared_ptr<ODEForwardPlugin::CommunicationModeHelper>  _communicationModeHelper = std::make_shared<CommunicationModeHelper>();
    };

    TEST_F(test_CommunicationModeHelper, test_compareCommunicationMode_Kafka){
        std::string modeSource = "KAFKA";
        EXPECT_TRUE(_communicationModeHelper->compareCommunicationMode(modeSource, CommunicationMode::KAFKA));
    }

    TEST_F(test_CommunicationModeHelper, test_compareCommunicationMode_UDP){
        std::string modeSource = "UDP";
        EXPECT_TRUE(_communicationModeHelper->compareCommunicationMode(modeSource, CommunicationMode::UDP));
    }

    TEST_F(test_CommunicationModeHelper, test_compareCommunicationMode_Unknown){
        std::string modeSource = "UNKNOWN";
        EXPECT_FALSE(_communicationModeHelper->compareCommunicationMode(modeSource, CommunicationMode::UDP));
    }

    TEST_F(test_CommunicationModeHelper, test_setMode_Kafka){
        std::string modeSource = "KAFKA";
        _communicationModeHelper->setMode(modeSource);
        EXPECT_TRUE(_communicationModeHelper->getKafkaMode());
        EXPECT_FALSE(_communicationModeHelper->getUDPMode());
    }

    TEST_F(test_CommunicationModeHelper, test_setMode_UDP){
        std::string modeSource = "UDP";
        _communicationModeHelper->setMode(modeSource);
        EXPECT_TRUE(_communicationModeHelper->getUDPMode());
        EXPECT_FALSE(_communicationModeHelper->getKafkaMode());
    }

    TEST_F(test_CommunicationModeHelper, test_setMode_Unknown){
        std::string modeSource = "UNKNOWN";
        _communicationModeHelper->setMode(modeSource);
        EXPECT_FALSE(_communicationModeHelper->getKafkaMode());
        EXPECT_FALSE(_communicationModeHelper->getUDPMode());
    }

    TEST_F(test_CommunicationModeHelper, test_getKafkaMode){
        std::string modeSource = "KAFKA";
        _communicationModeHelper->setMode(modeSource);
        EXPECT_TRUE(_communicationModeHelper->getKafkaMode());
    }
    
    TEST_F(test_CommunicationModeHelper, test_getKafkaMode_UDP){
        std::string modeSource = "UDP";
        _communicationModeHelper->setMode(modeSource);
        EXPECT_FALSE(_communicationModeHelper->getKafkaMode());
    }
}