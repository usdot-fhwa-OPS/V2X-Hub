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