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
  ptree MobilityOperationConverter::fromTree(const ptree &json_payload, const string &strategy) // or config
  {
	ptree mobilityOperationXml;
	std::stringstream string_payload;
	write_json(string_payload, json_payload);

	// Create XML MobilityOperationMessage
	ptree message;
	ptree header;
	ptree body;
	body.put("strategy", strategy);
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

}