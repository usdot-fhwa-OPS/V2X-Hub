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
#pragma once
#include <functional>
#include <string>
#include <vector>
#include <rapidjson/document.h>

namespace ODEForwardPlugin
{

	/**
	 * @brief Object for warning when conversion to number fails 
	 */
	using ConversionWarningLog = std::function<void(const std::string &)>;

	/**
	 * @brief Convert the named string fields of @p parent to integers in place.
	 * @param parent The JSON object whose fields may be converted.
	 * @param fields The names of the fields to attempt to convert.
	 * @param warn Object invoked once per field that throws during parsing
	 */
	void toNumber(rapidjson::Value &parent, const std::vector<std::string> &fields,
	              const ConversionWarningLog &warn = {});

	/**
	 * @brief Convert TMX's all-string CTI 4501 event JSON into correctly typed numbers.
	 * @param json The validation-event JSON payload as emitted by TMX.
	 * @param warn Object for per-field conversion-failure messages.
	 * @return The JSON with numeric fields typed as numbers, or the input unchanged on parse failure.
	 */
	std::string convertToNum(const std::string &json, const ConversionWarningLog &warn = {});

} /* namespace ODEForwardPlugin */