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
#include <string>
#include <vector>

namespace IntersectionValidation
{

    struct FieldValidation
    {
        bool valid = true;
        std::vector<std::string> errors;
    };

    /**
     * @brief Load a file's contents into a string.
     * @param filePath Path to the file.
     * @return File contents as a string.
     */
    std::string loadFileContents(const std::string &filePath);
    /**
     * @brief Validate a JSON string against a JSON Schema string using RapidJSON SchemaValidator.
     * @param jsonStr The JSON string to validate.
     * @param schemaStr The JSON Schema as a string.
     * @return FieldValidation containing validity and list of errors.
     */
    FieldValidation validateJsonAgainstSchema(const std::string &jsonStr, const std::string &schemaStr);

    /**
     * @brief Validate a JSON string against a JSON Schema.
     * @param jsonStr The JSON string to validate.
     * @param schemaFilePath Path to the JSON Schema file.
     * @return FieldValidation containing validity and list of errors.
     */
    FieldValidation validateJsonAgainstSchemaFile(const std::string &jsonStr, const std::string &schemaFilePath);

}