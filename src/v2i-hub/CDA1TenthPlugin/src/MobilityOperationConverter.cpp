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
#include "MobilityOperationConverter.h"

namespace CDA1TenthPlugin
{
  pt MobilityOperationConverter::toTree(const tmx::messages::tsm3Message &mobility_operation_msg)
  {
	pt message;
	pt strategy;
	pt payload;

	//strategy.put("strategy", mobility_operation_msg->body.strategy.buf);
	// read_json(mobility_operation_msg->body->payload, payload);

	// message.add_child("strategy", strategy);
	// message.add_child("payload", payload);

	return message;
  }

  pt MobilityOperationConverter::fromTree(const pt &json_payload) // or config
  {
	pt mobilityOperationXml;
	std::stringstream string_payload;
	write_json(string_payload, json_payload);

	// Create XML MobilityOperationMessage
	pt message;
	pt header;
	pt body;
	body.put("strategy","carma/port_drayage");
	body.put("operationParams", string_payload.str());
	header.put("hostStaticId", "UNSET");
	header.put("targetStaticId", "UNSET");
	header.put("hostBSMId", "00000000");
	header.put("planId", "00000000-0000-0000-0000-000000000000");
	header.put("timestamp", "0000000000000000000");
	message.put_child("header", header);
	message.put_child("body",body);
	mobilityOperationXml.put_child("TestMessage03", message);

	return mobilityOperationXml;
  }

  string MobilityOperationConverter::toJsonString(const pt &tree)
  {
    std::ostringstream buf;
    write_json(buf, tree, false);
    std::string jsonStr = buf.str();
    boost::algorithm::erase_all(jsonStr, "\n");
    boost::algorithm::erase_all(jsonStr, "\t");
    boost::algorithm::erase_all(jsonStr, " ");
    return jsonStr;
    return buf.str();
  }

}