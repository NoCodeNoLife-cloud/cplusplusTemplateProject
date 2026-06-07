/**
 * @file IJsonSerializableTest.cc
 * @brief Unit tests for IJsonSerializable interface
 * @details Tests cover serialize and deserialize operations via a mock implementation.
 */

#include <gtest/gtest.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "interface/serialize/IJsonSerializable.hpp"

using namespace common::interfaces::serialize;

namespace
{
    /// @brief Mock implementation of IJsonSerializable for testing
    class MockJsonSerializable : public IJsonSerializable
    {
    public:
        std::string name_;
        int value_ = 0;

        MockJsonSerializable() = default;
        MockJsonSerializable(std::string name, int value)
            : name_(std::move(name)), value_(value) {}

        void serialize(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override
        {
            writer.StartObject();
            writer.Key("name");
            writer.String(name_.c_str());
            writer.Key("value");
            writer.Int(value_);
            writer.EndObject();
        }

        void deserialize(const rapidjson::Value& json) override
        {
            if (json.HasMember("name") && json["name"].IsString())
                name_ = json["name"].GetString();
            if (json.HasMember("value") && json["value"].IsInt())
                value_ = json["value"].GetInt();
        }
    };
}

class IJsonSerializableTest : public testing::Test
{
protected:
    void SetUp() override
    {
        obj_.reset(new MockJsonSerializable("test", 42));
    }

    void TearDown() override
    {
        obj_.reset();
    }

    std::unique_ptr<MockJsonSerializable> obj_;
};

TEST_F(IJsonSerializableTest, SerializeProducesValidJson)
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

    obj_->serialize(writer);

    rapidjson::Document doc;
    doc.Parse(buffer.GetString());
    ASSERT_FALSE(doc.HasParseError());

    ASSERT_TRUE(doc.HasMember("name"));
    ASSERT_TRUE(doc["name"].IsString());
    EXPECT_STREQ(doc["name"].GetString(), "test");

    ASSERT_TRUE(doc.HasMember("value"));
    ASSERT_TRUE(doc["value"].IsInt());
    EXPECT_EQ(doc["value"].GetInt(), 42);
}

TEST_F(IJsonSerializableTest, DeserializePopulatesFields)
{
    rapidjson::Document doc;
    doc.SetObject();
    doc.AddMember("name", "updated", doc.GetAllocator());
    doc.AddMember("value", 99, doc.GetAllocator());

    obj_->deserialize(doc);

    EXPECT_EQ(obj_->name_, "updated");
    EXPECT_EQ(obj_->value_, 99);
}

TEST_F(IJsonSerializableTest, RoundTripPreservesData)
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    obj_->serialize(writer);

    rapidjson::Document doc;
    doc.Parse(buffer.GetString());

    MockJsonSerializable deserialized;
    deserialized.deserialize(doc);

    EXPECT_EQ(deserialized.name_, "test");
    EXPECT_EQ(deserialized.value_, 42);
}

TEST_F(IJsonSerializableTest, DeserializeMissingField)
{
    rapidjson::Document doc;
    doc.SetObject();
    doc.AddMember("name", "partial", doc.GetAllocator());

    obj_->deserialize(doc);

    EXPECT_EQ(obj_->name_, "partial");
    EXPECT_EQ(obj_->value_, 42);
}

TEST_F(IJsonSerializableTest, MultipleSerializations)
{
    rapidjson::StringBuffer buffer1;
    rapidjson::Writer<rapidjson::StringBuffer> writer1(buffer1);
    obj_->serialize(writer1);

    obj_->name_ = "second";
    obj_->value_ = 77;

    rapidjson::StringBuffer buffer2;
    rapidjson::Writer<rapidjson::StringBuffer> writer2(buffer2);
    obj_->serialize(writer2);

    rapidjson::Document doc1;
    doc1.Parse(buffer1.GetString());
    EXPECT_STREQ(doc1["name"].GetString(), "test");
    EXPECT_EQ(doc1["value"].GetInt(), 42);

    rapidjson::Document doc2;
    doc2.Parse(buffer2.GetString());
    EXPECT_STREQ(doc2["name"].GetString(), "second");
    EXPECT_EQ(doc2["value"].GetInt(), 77);
}
