/**
 * @file JsonObjectSerializerTest.cc
 * @brief Unit tests for JsonObjectSerializer utility
 * @details Tests cover value extraction with defaults and field serialization.
 */

#include <cstdint>
#include <string>

#include <gtest/gtest.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "serialization/JsonObjectSerializer.hpp"

using namespace common::serialization;

class JsonObjectSerializerTest : public testing::Test
{
protected:
    void SetUp() override
    {
        doc_.SetObject();
        doc_.AddMember("name", "test_name", doc_.GetAllocator());
        doc_.AddMember("count", 42, doc_.GetAllocator());
        doc_.AddMember("ratio", 3.14, doc_.GetAllocator());
        doc_.AddMember("enabled", true, doc_.GetAllocator());
    }

    rapidjson::Document doc_;
};

TEST_F(JsonObjectSerializerTest, GetStringOrDefaultFound)
{
    EXPECT_EQ(JsonObjectSerializer::getStringOrDefault(doc_, "name", "fallback"), "test_name");
}

TEST_F(JsonObjectSerializerTest, GetStringOrDefaultMissing)
{
    EXPECT_EQ(JsonObjectSerializer::getStringOrDefault(doc_, "missing", "fallback"), "fallback");
}

TEST_F(JsonObjectSerializerTest, GetStringOrDefaultWrongType)
{
    EXPECT_EQ(JsonObjectSerializer::getStringOrDefault(doc_, "count", "fallback"), "fallback");
}

TEST_F(JsonObjectSerializerTest, GetIntOrDefaultFound)
{
    EXPECT_EQ(JsonObjectSerializer::getIntOrDefault(doc_, "count", -1), 42);
}

TEST_F(JsonObjectSerializerTest, GetIntOrDefaultMissing)
{
    EXPECT_EQ(JsonObjectSerializer::getIntOrDefault(doc_, "missing", -1), -1);
}

TEST_F(JsonObjectSerializerTest, GetIntOrDefaultWrongType)
{
    EXPECT_EQ(JsonObjectSerializer::getIntOrDefault(doc_, "name", -1), -1);
}

TEST_F(JsonObjectSerializerTest, GetDoubleOrDefaultFound)
{
    EXPECT_DOUBLE_EQ(JsonObjectSerializer::getDoubleOrDefault(doc_, "ratio", 0.0), 3.14);
}

TEST_F(JsonObjectSerializerTest, GetDoubleOrDefaultMissing)
{
    EXPECT_DOUBLE_EQ(JsonObjectSerializer::getDoubleOrDefault(doc_, "missing", 1.5), 1.5);
}

TEST_F(JsonObjectSerializerTest, GetDoubleOrDefaultWrongType)
{
    EXPECT_DOUBLE_EQ(JsonObjectSerializer::getDoubleOrDefault(doc_, "name", 2.5), 2.5);
}

TEST_F(JsonObjectSerializerTest, GetBoolOrDefaultFound)
{
    EXPECT_TRUE(JsonObjectSerializer::getBoolOrDefault(doc_, "enabled", false));
}

TEST_F(JsonObjectSerializerTest, GetBoolOrDefaultMissing)
{
    EXPECT_TRUE(JsonObjectSerializer::getBoolOrDefault(doc_, "missing", true));
}

TEST_F(JsonObjectSerializerTest, GetBoolOrDefaultWrongType)
{
    EXPECT_FALSE(JsonObjectSerializer::getBoolOrDefault(doc_, "name", false));
}

TEST_F(JsonObjectSerializerTest, SerializeStringField)
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    writer.StartObject();
    JsonObjectSerializer::serializeField(writer, "key", "value");
    writer.EndObject();

    rapidjson::Document doc;
    doc.Parse(buffer.GetString());
    ASSERT_TRUE(doc.HasMember("key"));
    ASSERT_TRUE(doc["key"].IsString());
    EXPECT_STREQ(doc["key"].GetString(), "value");
}

TEST_F(JsonObjectSerializerTest, SerializeIntField)
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    writer.StartObject();
    JsonObjectSerializer::serializeField(writer, "num", 99);
    writer.EndObject();

    rapidjson::Document doc;
    doc.Parse(buffer.GetString());
    ASSERT_TRUE(doc.HasMember("num"));
    ASSERT_TRUE(doc["num"].IsInt());
    EXPECT_EQ(doc["num"].GetInt(), 99);
}

TEST_F(JsonObjectSerializerTest, SerializeDoubleField)
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    writer.StartObject();
    JsonObjectSerializer::serializeField(writer, "pi", 3.14159);
    writer.EndObject();

    rapidjson::Document doc;
    doc.Parse(buffer.GetString());
    ASSERT_TRUE(doc.HasMember("pi"));
    ASSERT_TRUE(doc["pi"].IsDouble());
    EXPECT_DOUBLE_EQ(doc["pi"].GetDouble(), 3.14159);
}

TEST_F(JsonObjectSerializerTest, SerializeBoolField)
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    writer.StartObject();
    JsonObjectSerializer::serializeField(writer, "flag", true);
    writer.EndObject();

    rapidjson::Document doc;
    doc.Parse(buffer.GetString());
    ASSERT_TRUE(doc.HasMember("flag"));
    ASSERT_TRUE(doc["flag"].IsBool());
    EXPECT_TRUE(doc["flag"].GetBool());
}
