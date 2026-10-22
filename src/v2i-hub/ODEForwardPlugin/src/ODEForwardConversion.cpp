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

#include "ODEForwardConversion.h"

#include <cstdint>
#include <exception>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

namespace ODEForwardPlugin
{

	void toNumber(rapidjson::Value &parent, const std::vector<std::string> &fields,
	              const ConversionWarningLog &warn)
	{
		for (const auto &field : fields)
		{
			auto it = parent.FindMember(field.c_str());
			if (it == parent.MemberEnd() || !it->value.IsString())
			{
				continue;
			}

			const std::string raw = it->value.GetString();
			try
			{
				size_t consumed = 0;
				const int64_t parsed = std::stoll(raw, &consumed);
				// Only replace on a clean, complete parse; leave anything else as-is.
				if (consumed == raw.size())
				{
					it->value.SetInt64(parsed);
				}
			}
			catch (const std::exception &e)
			{
				// Not an integer (invalid_argument / out_of_range)
				if (warn)
				{
					warn("Failed to convert field '" + field + "' to number: " + e.what());
				}
			}
		}
	}

	std::string convertToNum(const std::string &json, const ConversionWarningLog &warn)
	{
		static const std::vector<std::string> numericFields {
			"eventGeneratedAt", "intersectionID", "roadRegulatorID",
			"numberOfMessages", "messageCountA", "messageCountB"
		};
		static const std::vector<std::string> timePeriodFields { "beginTimestamp", "endTimestamp" };

		rapidjson::Document doc;
		doc.Parse(json.c_str());
		if (doc.HasParseError() || !doc.IsObject())
		{
			return json;
		}

		toNumber(doc, numericFields, warn);

		auto timePeriod = doc.FindMember("timePeriod");
		if (timePeriod != doc.MemberEnd() && timePeriod->value.IsObject())
		{
			toNumber(timePeriod->value, timePeriodFields, warn);
		}

		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		doc.Accept(writer);
		return buffer.GetString();
	}

} /* namespace ODEForwardPlugin */