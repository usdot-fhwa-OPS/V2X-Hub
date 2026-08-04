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

#include "FieldValidation.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstring>
#include <rapidjson/pointer.h>

// Collects missing required fields. This functionality creates a list of the missing data elements
// based on the format that CV Manager expects.
namespace
{
    // Depth cap guards against $ref cycles.
    constexpr int MAX_SCHEMA_DEPTH = 64;

    // Resolve a local "#/..." $ref against the root schema. Returns nullptr for
    // external or unresolvable refs.
    const rapidjson::Value *resolveRef(const rapidjson::Value &node,
                                       const rapidjson::Value &schemaRoot)
    {
        auto ref = node.FindMember("$ref");
        if (ref == node.MemberEnd() || !ref->value.IsString())
        {
            return &node;
        }

        const std::string refStr = ref->value.GetString();
        if (refStr.empty() || refStr[0] != '#')
        {
            return nullptr;
        }
        if (refStr == "#")
        {
            return &schemaRoot;
        }
        return rapidjson::Pointer(refStr.substr(1).c_str()).Get(schemaRoot);
    }

    // Following a $ref re-bases reported schema references on the ref target, so
    // entries cite "#/$defs/J2735TimeMark" rather than the full inline path
    std::string refOr(const rapidjson::Value &node, const std::string &fallback)
    {
        if (node.IsObject())
        {
            if (auto ref = node.FindMember("$ref");
                ref != node.MemberEnd() && ref->value.IsString())
            {
                return ref->value.GetString();
            }
        }
        return fallback;
    }

    // Schema reference to cite for a missing property: the property's own $ref if it
    // has one, otherwise the inline "<parent>/properties/<name>" fallback.
    std::string propertySchemaRef(const rapidjson::Value &schema, const char *name,
                                  const std::string &fallback)
    {
        auto properties = schema.FindMember("properties");
        if (properties == schema.MemberEnd() || !properties->value.IsObject())
        {
            return fallback;
        }
        auto propSchema = properties->value.FindMember(name);
        if (propSchema == properties->value.MemberEnd())
        {
            return fallback;
        }
        return refOr(propSchema->value, fallback);
    }

    // Forward declaration so handleObject can recurse back into the entry point.
    void collectMissingRequired(const rapidjson::Value &schemaNode,
                                const rapidjson::Value &instance,
                                const rapidjson::Value &schemaRoot,
                                const std::string &jsonPath,
                                const std::string &schemaPath,
                                std::vector<std::string> &out,
                                int depth = 0);

    // Object handling on its own so its two required/descend loops reset the nesting
    // baseline and stay within the depth limit
    void handleObject(const rapidjson::Value &schema, const rapidjson::Value &instance,
                      const rapidjson::Value &schemaRoot, const std::string &jsonPath,
                      const std::string &resolvedSchemaPath,
                      std::vector<std::string> &out, int depth)
    {
        if (!instance.IsObject())
        {
            return;
        }

        // Required properties absent from this object
        if (auto required = schema.FindMember("required");
            required != schema.MemberEnd() && required->value.IsArray())
        {
            for (const auto &name : required->value.GetArray())
            {
                if (!name.IsString() || instance.HasMember(name.GetString()))
                {
                    continue;
                }
                const std::string elementSchemaPath = propertySchemaRef(
                    schema, name.GetString(),
                    resolvedSchemaPath + "/properties/" + name.GetString());
                out.emplace_back(jsonPath + "." + name.GetString() +
                                 " is missing (" + elementSchemaPath + ")");
            }
        }

        // Descend into the properties that are actually present
        if (auto properties = schema.FindMember("properties");
            properties != schema.MemberEnd() && properties->value.IsObject())
        {
            for (auto it = properties->value.MemberBegin();
                 it != properties->value.MemberEnd(); ++it)
            {
                if (auto child = instance.FindMember(it->name);
                    child != instance.MemberEnd())
                {
                    collectMissingRequired(it->value, child->value, schemaRoot,
                                           jsonPath + "." + it->name.GetString(),
                                           resolvedSchemaPath + "/properties/" +
                                               it->name.GetString(),
                                           out, depth + 1);
                }
            }
        }
    }

    // Recursively compare the document against the schema and collect every required
    // property that is absent
    void collectMissingRequired(const rapidjson::Value &schemaNode,
                                const rapidjson::Value &instance,
                                const rapidjson::Value &schemaRoot,
                                const std::string &jsonPath,
                                const std::string &schemaPath,
                                std::vector<std::string> &out,
                                int depth)
    {
        if (depth > MAX_SCHEMA_DEPTH)
        {
            return;
        }

        const std::string resolvedSchemaPath = refOr(schemaNode, schemaPath);

        const rapidjson::Value *schema = resolveRef(schemaNode, schemaRoot);
        if (schema == nullptr || !schema->IsObject())
        {
            return;
        }

        // Objects: missing-required plus descent into present properties.
        handleObject(*schema, instance, schemaRoot, jsonPath, resolvedSchemaPath, out, depth);

        // Arrays: descend into each element against the "items" subschema.
        if (instance.IsArray())
        {
            if (auto items = schema->FindMember("items");
                items != schema->MemberEnd() && items->value.IsObject())
            {
                rapidjson::SizeType index = 0;
                for (const auto &element : instance.GetArray())
                {
                    collectMissingRequired(items->value, element, schemaRoot,
                                           jsonPath + "[" + std::to_string(index) + "]",
                                           resolvedSchemaPath + "/items", out, depth + 1);
                    ++index;
                }
            }
        }

        // allOf applies every subschema, so its required constraints all hold.
        if (auto allOf = schema->FindMember("allOf");
            allOf != schema->MemberEnd() && allOf->value.IsArray())
        {
            rapidjson::SizeType index = 0;
            for (const auto &sub : allOf->value.GetArray())
            {
                collectMissingRequired(sub, instance, schemaRoot, jsonPath,
                                       resolvedSchemaPath + "/allOf/" + std::to_string(index),
                                       out, depth + 1);
                ++index;
            }
        }
    }
}


namespace IntersectionValidation
{

    class FileLoadException : public std::runtime_error
    {
    public:
        using std::runtime_error::runtime_error;
    };

    std::string loadFileContents(const std::string &filePath)
    {
        std::ifstream file(filePath);
        if (!file.is_open())
        {
            throw FileLoadException("Failed to open file: " + filePath);
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    static bool schemaHasType(const rapidjson::Value &schema, const char *typeName)
    {
        // Verify that the schema node is an object and has a "type" member
        if (!schema.IsObject() || !schema.HasMember("type"))
        {
            return false;
        }

        const auto &typeVal = schema["type"];

        // Check if the type is an integer
        if (typeVal.IsString())
        {
            return std::string(typeVal.GetString()) == typeName;
        }

        // Check if the type is an array form 
        if (typeVal.IsArray())
        {
            for (rapidjson::SizeType i = 0; i < typeVal.Size(); ++i)
            {
                if (typeVal[i].IsString() && std::string(typeVal[i].GetString()) == typeName)
                {
                    return true;
                }
            }
        }

        return false;
    }

    static bool tryConvertToBool(rapidjson::Value &value)
    {
        if (!value.IsString())
        {
            return false;
        }

        const char *str = value.GetString();
        if (std::strcmp(str, "true") == 0)
        {
            value.SetBool(true);
            return true;
        }
        if (std::strcmp(str, "false") == 0)
        {
            value.SetBool(false);
            return true;
        }

        return false;
    }

    static const rapidjson::Value *getPropertySchema(const rapidjson::Value &schema, const char *key)
    {
        // Schema node must be an object to have properties
        if (!schema.IsObject())
        {
            return nullptr;
        }

        // Directly look at the properites of the current node
        // eg. {"properties": {"id": {"type": "integer"}}}
        if (schema.HasMember("properties") &&
            schema["properties"].IsObject() && schema["properties"].HasMember(key))
        {
            return &schema["properties"][key];
        }

        // If not found directly, look through the oneOf, anyOf, allOf branches (CHOICE options)
        for (const char *combiner : {"oneOf", "anyOf", "allOf"})
        {
            // Skip if this combiner doesn't exist or isn't an array
            if (!schema.HasMember(combiner) || !schema[combiner].IsArray())
            {
                continue;
            }

            // Search each branch recursively to find property since it could be nested within one of the branches
            for (rapidjson::SizeType i = 0; i < schema[combiner].Size(); ++i)
            {
                const rapidjson::Value *found = getPropertySchema(schema[combiner][i], key);
                if (found != nullptr)
                {
                    return found;
                }
            }
        }

        return nullptr;
    }

    static const rapidjson::Value *getItemsSchema(const rapidjson::Value &schema)
    {
        // Arrays in JSON Schema define their element type under the "items" key
        // Return a pointer to it so convertNumericStrings can recurse into
        // each array element with the correct schema context
        if (schema.IsObject() && schema.HasMember("items") && schema["items"].IsObject())
        {
            return &schema["items"];
        }

        // If no "items" defined, array elements have no schema to validate against
        return nullptr;
    }

    static bool tryConvertToInt(rapidjson::Value &value)
    {
        if (!value.IsString())
        {
            return false;
        }

        // Parse the string as an integer
        const char *str = value.GetString();
        char *end = nullptr;
        int64_t num = std::strtoll(str, &end, 10);

        if (end == str || *end != '\0')
        {
            return false;
        }

        value.SetInt64(num);
        return true;
    }

    void convertNumericStrings(rapidjson::Value &value,
                               rapidjson::Document::AllocatorType &allocator,
                               const rapidjson::Value &schema)
    {
        // Check if the the value is a string and if in the schema the field is defined as an integer
        if (value.IsString() && schemaHasType(schema, "integer"))
        {
            tryConvertToInt(value);
        }
        // Check if the the value is a string and if in the schema the field is defined as an boolean
        else if (value.IsString() && schemaHasType(schema, "boolean"))
        {
            tryConvertToBool(value);
        }
        // If the value is an object, recursively check its properties against the schema
        else if (value.IsObject())
        {
            for (auto it = value.MemberBegin(); it != value.MemberEnd(); ++it)
            {
                // Look up the field's schema definition 
                const rapidjson::Value *propSchema = getPropertySchema(schema, it->name.GetString());
                if (propSchema == nullptr)
                {
                    // Field is not defined in the schema, skip conversion for this field
                    continue;
                }

                if (schemaHasType(*propSchema, "integer"))
                {
                    tryConvertToInt(it->value);
                }
                else if (schemaHasType(*propSchema, "boolean"))
                {
                    tryConvertToBool(it->value);
                }
                else
                {
                    convertNumericStrings(it->value, allocator, *propSchema);
                }
            }
        }
        else if (value.IsArray())
        {
            const rapidjson::Value *itemsSchema = getItemsSchema(schema);
            if (itemsSchema != nullptr)
            {
                for (rapidjson::SizeType i = 0; i < value.Size(); ++i)
                {
                    convertNumericStrings(value[i], allocator, *itemsSchema);
                }
            }
        }
    }

    void removeEmptyStrings(rapidjson::Value &value, rapidjson::Document::AllocatorType &allocator)
    {
        if (!value.IsObject())
        {
            return;
        }

        if (value.IsArray())
        {
            for (rapidjson::SizeType i = 0; i < value.Size(); ++i)
            {
                removeEmptyStrings(value[i], allocator);
            }
        }

        for (auto it = value.MemberBegin(); it != value.MemberEnd();)
        {
            // If a member's value is an empty string (""), remove it so that the schema
            // sees it as absent rather than a present but empty value (string vs object)
            if (it->value.IsString() && it->value.GetStringLength() == 0)
            {
                it = value.EraseMember(it);
            }
            else
            {
                if (it->value.IsObject() || it->value.IsArray())
                {
                    removeEmptyStrings(it->value, allocator);
                }
                ++it;
            }
        }
    }

    std::vector<std::string> collectMissingRequiredFields(const rapidjson::Value &schema,
                                                          const rapidjson::Value &doc)
    {
        // Enumerate every missing required element in conflictmonitor's minimum-data
        // string format. Operates on the caller's already-preprocessed document; it
        // does not parse or mutate. The root schema doubles as the $ref resolution
        // base, so it is passed as both the current node and the root.
        std::vector<std::string> missing;
        collectMissingRequired(schema, doc, schema, "$", "#", missing);
        return missing;
    }

    FieldValidation validateJsonAgainstSchema(const std::string &jsonStr, const std::string &schemaStr)
    {
        FieldValidation result;

        // Parse the schema
        rapidjson::Document schemaDoc;
        schemaDoc.Parse(schemaStr.c_str());
        if (schemaDoc.HasParseError())
        {
            result.valid = false;
            result.errors.emplace_back("Failed to parse JSON schema");
            return result;
        }

        rapidjson::SchemaDocument schema(schemaDoc);

        // Parse the input JSON
        rapidjson::Document doc;
        doc.Parse(jsonStr.c_str());
        if (doc.HasParseError())
        {
            result.valid = false;
            result.errors.emplace_back("Failed to parse input JSON");
            return result;
        }

        // Remove empty strings
        removeEmptyStrings(doc, doc.GetAllocator());

        // Convert numeric strings to integers
        convertNumericStrings(doc, doc.GetAllocator(), schemaDoc);

        // Validate
        if (rapidjson::SchemaValidator validator(schema); !doc.Accept(validator))
        {
            result.valid = false;

            rapidjson::StringBuffer sb;

            const char *keyword = validator.GetInvalidSchemaKeyword();
            std::string error = "Schema validation failed: keyword = ";
            error += keyword ? keyword : "unknown";

            validator.GetInvalidDocumentPointer().StringifyUriFragment(sb);
            error += ", document_path =  " + std::string(sb.GetString());
            sb.Clear();

            validator.GetInvalidSchemaPointer().StringifyUriFragment(sb);
            error += ", schema_path = " + std::string(sb.GetString());

            result.errors.push_back(error);
        }

        return result;
    }

    FieldValidation validateJsonAgainstSchemaFile(const std::string &jsonStr, const std::string &schemaFilePath)
    {
        FieldValidation result;

        try
        {
            std::string schemaStr = loadFileContents(schemaFilePath);
            return validateJsonAgainstSchema(jsonStr, schemaStr);
        }
        catch (const FileLoadException &e)
        {
            result.valid = false;
            result.errors.emplace_back(e.what());
            return result;
        }
    }

}