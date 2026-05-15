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
        if (!schema.IsObject() || !schema.HasMember("type"))
        {
            return false;
        }

        const auto &typeVal = schema["type"];
        if (typeVal.IsString())
        {
            return std::string(typeVal.GetString()) == typeName;
        }

        // Handle "type": ["integer", "string"] arrays
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

    static const rapidjson::Value *getPropertySchema(const rapidjson::Value &schema, const char *key)
    {
        if (!schema.IsObject())
        {
            return nullptr;
        }

        // Direct properties lookup
        if (schema.HasMember("properties") &&
            schema["properties"].IsObject() && schema["properties"].HasMember(key))
        {
            return &schema["properties"][key];
        }

        // Search inside oneOf / anyOf / allOf
        for (const char *combiner : {"oneOf", "anyOf", "allOf"})
        {
            if (!schema.HasMember(combiner) || !schema[combiner].IsArray())
            {
                continue;
            }

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
        if (schema.IsObject() && schema.HasMember("items") && schema["items"].IsObject())
        {
            return &schema["items"];
        }
        return nullptr;
    }

    static bool tryConvertToInt(rapidjson::Value &value)
    {
        if (!value.IsString())
        {
            return false;
        }

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
        if (value.IsString() && schemaHasType(schema, "integer"))
        {
            tryConvertToInt(value);
        }
        else if (value.IsObject())
        {
            for (auto it = value.MemberBegin(); it != value.MemberEnd(); ++it)
            {
                const rapidjson::Value *propSchema = getPropertySchema(schema, it->name.GetString());
                if (propSchema == nullptr)
                {
                    continue;
                }

                if (schemaHasType(*propSchema, "integer"))
                {
                    tryConvertToInt(it->value);
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

    static void removeEmptyStrings(rapidjson::Value &value, rapidjson::Document::AllocatorType &allocator)
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

        // Remove TMX empty string placeholders for optional object fields
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