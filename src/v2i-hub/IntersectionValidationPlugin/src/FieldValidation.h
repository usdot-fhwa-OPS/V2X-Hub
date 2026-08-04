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
     * @brief Resolve a local "#/..." $ref against the root schema. Returns nullptr for
     * external or unresolvable refs
     * @param node The schema node that may contain a $ref member
     * @param schemaRoot The root schema against which local pointers are resolved
     * @return The resolved schema node, the node itself if it has no "$ref", or nullptr if unresolvable
     */
    const rapidjson::Value *resolveRef(const rapidjson::Value &node,
                                       const rapidjson::Value &schemaRoot);

    /**
     * @brief Return a node's own "$ref" string if it declares one, otherwise the supplied fallback
     * @param node The schema node that may declare a "$ref"
     * @param fallback The schema path to return when the node has no "$ref"
     * @return The node's "$ref" string, or @p fallback when it declares none
     */
    std::string refOr(const rapidjson::Value &node, const std::string &fallback);

    /**
     * @brief Determine the schema reference to cite for a missing required property
     * @param schema The parent (object) schema node containing the "properties" map
     * @param name The name of the missing required property
     * @param fallback The inline schema path to use when no property "$ref" is found
     * @return The property's "$ref", or @p fallback.
     */
    std::string propertySchemaRef(const rapidjson::Value &schema, const char *name,
                                  const std::string &fallback);

                                  
    /**
     * @brief Collect missing required properties for an object instance and recurse into present ones
     * @param schema The (already $ref-resolved) object schema node
     * @param instance The instance value being validated
     * @param schemaRoot The root schema, used to resolve "$ref" during recursion
     * @param jsonPath The JSON-path prefix of @p instance (e.g. "$.value.SPAT")
     * @param resolvedSchemaPath The schema-path prefix used when citing missing elements
     * @param out Output list; one formatted string is appended per missing element
     * @param depth Current recursion depth, capped to guard against "$ref" cycles
     */
    void handleObject(const rapidjson::Value &schema, const rapidjson::Value &instance,
                      const rapidjson::Value &schemaRoot, const std::string &jsonPath,
                      const std::string &resolvedSchemaPath,
                      std::vector<std::string> &out, int depth);

    /**
     * @brief Recursively compare an instance against a schema and collect every absent required property
     * @param schemaNode The schema node (may carry a "$ref") to compare against
     * @param instance The instance value being validated
     * @param schemaRoot The root schema, used to resolve local "$ref" pointers
     * @param jsonPath The JSON-path prefix of @p instance
     * @param schemaPath The schema-path prefix used when citing missing elements
     * @param out Output list; one formatted string is appended per missing element
     * @param depth Current recursion depth, capped to guard against "$ref" cycles
     */
    void collectMissingRequired(const rapidjson::Value &schemaNode,
                                const rapidjson::Value &instance,
                                const rapidjson::Value &schemaRoot,
                                const std::string &jsonPath,
                                const std::string &schemaPath,
                                std::vector<std::string> &out,
                                int depth = 0);

    /**
     * @brief Enumerate every required property absent from the document, as JSON-path strings
     * @param schema The root JSON Schema node
     * @param doc The instance document to check against the schema
     * @return One string per missing required element; empty if none are missing
     */
    std::vector<std::string> collectMissingRequiredFields(const rapidjson::Value &schema,
                                                          const rapidjson::Value &doc);


    /**
     * @brief Recursively convert numeric strings in a RapidJSON value to integers, except for keys named "status".
     * @param value The RapidJSON value to process.
     * @param allocator The RapidJSON allocator for modifying the value.
     * @param schema The JSON Schema node providing type context for the value.
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
    void removeEmptyStrings(rapidjson::Value &value, rapidjson::Document::AllocatorType &allocator);

    /**
     * @brief Validate a JSON string against a JSON Schema.
     * @param jsonStr The JSON string to validate.
     * @param schemaFilePath Path to the JSON Schema file.
     * @return FieldValidation containing validity and list of errors.
     */
    FieldValidation validateJsonAgainstSchemaFile(const std::string &jsonStr, const std::string &schemaFilePath);
}