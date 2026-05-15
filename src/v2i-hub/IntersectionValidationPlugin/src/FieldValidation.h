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
#include <rapidjson/document.h>
#include <rapidjson/schema.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

namespace IntersectionValidation
{

    struct FieldValidation
    {
        bool valid = true;
        std::vector<std::string> errors;
    };

    /**
     * @brief Recursively convert numeric strings in a RapidJSON value to integers, except for keys named "status".
     * @param value The RapidJSON value to process.
     * @param allocator The RapidJSON allocator for modifying the value.
     * @param key The current key being processed (used to skip "status" fields).
     */
    void convertNumericStrings(rapidjson::Value &value,
                                      rapidjson::Document::AllocatorType &allocator,
                                      const rapidjson::Value &schema);
    /**
     * @brief Load a file's contents into a string.
     * @param filePath Path to the file.
     * @return File contents as a string.
     */
    std::string loadFileContents(const std::string &filePath);

    /**
     * @brief Check if a JSON Schema node declares a specific type (e.g., "integer"), handling both string and array forms of the "type" keyword.
     * @param schema The JSON Schema node to check.
     * @param typeName The type name to look for (e.g., "integer").
     * @return True if the schema declares the type, false otherwise.
     */
    static bool schemaHasType(const rapidjson::Value &schema, const char *typeName);

    /**
     * @brief Get the schema node for a property key, or nullptr if not found.
     *        First checks the direct "properties" object, If not found, then 
     *        search through the oneOf, anyOf, allOf branches recursively.
     * @param schema The JSON Schema node representing the parent object.
     * @param key The property key to look up.
     * @return Pointer to the schema node for the property, or nullptr if not found.
     */
    static const rapidjson::Value *getPropertySchema(const rapidjson::Value &schema, const char *key);
   
    /**
     * @brief Get the schema node for array items, or nullptr if not found.
     * @param schema The JSON Schema node representing the array.
     * @return Pointer to the schema node for the array items, or nullptr if not found
     */
    static const rapidjson::Value *getItemsSchema(const rapidjson::Value &schema);

    /**
     * @brief Attempt to convert a RapidJSON value from a numeric string to an integer. Returns true if conversion was successful.
     * @param value The RapidJSON value to convert. Must be a string containing a valid integer representation.
     * @return True if the value was successfully converted to an integer, false otherwise.
     */
    static bool tryConvertToInt(rapidjson::Value &value);
    
    /**
     * @brief Validate a JSON string against a JSON Schema string using RapidJSON SchemaValidator.
     * @param jsonStr The JSON string to validate.
     * @param schemaStr The JSON Schema as a string.
     * @return FieldValidation containing validity and list of errors.
     */
    FieldValidation validateJsonAgainstSchema(const std::string &jsonStr, const std::string &schemaStr);

    /**
     * @brief Recursively remove empty-string values from a JSON document.
     *
     *        Checks to see if a value is an empty string, if so it removes it
     *        so that during validation, the schema treats it as an empty field rather than a present but empty string value.
     * 
     * @param value The JSON value to process in-place.
     * @param allocator The document allocator.
     */
    static void removeEmptyStrings(rapidjson::Value &value, rapidjson::Document::AllocatorType &allocator);

    /**
     * @brief Validate a JSON string against a JSON Schema.
     * @param jsonStr The JSON string to validate.
     * @param schemaFilePath Path to the JSON Schema file.
     * @return FieldValidation containing validity and list of errors.
     */
    FieldValidation validateJsonAgainstSchemaFile(const std::string &jsonStr, const std::string &schemaFilePath);

}