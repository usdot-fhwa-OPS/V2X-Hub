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
#include <rapidjson/document.h>
#include <rapidjson/schema.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

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

    static void convertNumericStrings(rapidjson::Value &value,
                                      rapidjson::Document::AllocatorType &allocator,
                                      const std::string &key = "")
    {
        if (value.IsString() && key != "status")
        {
            const char *str = value.GetString();
            char *end;
            long num = std::strtol(str, &end, 10);
            if (*end == '\0' && str[0] != '\0')
            {
                value.SetInt64(num);
            }
        }
        else if (value.IsObject())
        {
            for (auto it = value.MemberBegin(); it != value.MemberEnd(); ++it)
            {
                convertNumericStrings(it->value, allocator, it->name.GetString());
            }
        }
        else if (value.IsArray())
        {
            for (auto it = value.Begin(); it != value.End(); ++it)
            {
                convertNumericStrings(*it, allocator, key);
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

        // Convert numeric strings to integers
        convertNumericStrings(doc, doc.GetAllocator());

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