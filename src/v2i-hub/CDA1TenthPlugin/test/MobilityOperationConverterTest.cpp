#include <gtest/gtest.h>
#include <tmx/j2735_messages/J2735MessageFactory.hpp>
#include "MobilityOperationConverter.h"

// TEST(MobilityOperationConverterTest, toTree)
// {
//     tsm3Message_t *mobilityOp = (tsm3Message_t *)calloc(1, sizeof(tsm3Message_t));

//     mobilityOp->header->hostStaticId = "HOST1";
//     mobilityOp->header->targetStaticId = "TARGET1";
//     mobilityOp->header->hostBSMId = 12341234;
//     mobilityOp->header->planId = 15;
//     mobilityOp->header->timestamp = 12345678;
//     mobilityOp->body->strategy = "carma/port_drayage";
//     mobilityOp->body->operationParams = ;

//     boost::property_tree::ptree tree = CDA1TenthPlugin::MobilityOperationConverter::toTree(*mobilityOp);

//     EXPECT_EQ(tree.get<std::string>("MobilityOperationMessage.body.strategy")"carma/port_drayage");

// }

// TEST(MobilityOperationConverterTest, fromTree)
// {
    
// }

// TEST(MobilityOperationConverterTest, toJsonString)
// {
    
// }