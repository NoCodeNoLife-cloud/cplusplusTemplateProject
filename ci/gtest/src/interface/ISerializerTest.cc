/**
 * @file ISerializerTest.cc
 * @brief Unit tests for ISerializer interface
 * @details Tests cover serialization and deserialization via a mock implementation.
 */

#include <sstream>
#include <string>
#include <unordered_map>
#include <gtest/gtest.h>

#include "interface/serialize/ISerializer.hpp"

using namespace common::interfaces::serialize;

namespace
{
    /// @brief Simple data type for serialization testing
    struct Person
    {
        std::string name;
        int age = 0;

        bool operator==(const Person& other) const
        {
            return name == other.name && age == other.age;
        }
    };

    /// @brief Mock serializer that stores data in memory (stringstream)
    class MockPersonSerializer : public ISerializer<Person>
    {
    public:
        void serialize(const Person& obj, const std::string& filename) override
        {
            storage_[filename] = obj;
        }

        Person deserialize(const std::string& filename) override
        {
            return storage_.at(filename);
        }

    private:
        std::unordered_map<std::string, Person> storage_;
    };
}

class ISerializerTest : public testing::Test
{
protected:
    void SetUp() override
    {
        serializer_.reset(new MockPersonSerializer());
    }

    void TearDown() override
    {
        serializer_.reset();
    }

    std::unique_ptr<MockPersonSerializer> serializer_;
};

TEST_F(ISerializerTest, SerializeAndDeserializePreservesData)
{
    Person alice{"Alice", 30};

    serializer_->serialize(alice, "alice.dat");
    Person result = serializer_->deserialize("alice.dat");

    EXPECT_EQ(result, alice);
}

TEST_F(ISerializerTest, MultipleSerializationsWithDifferentKeys)
{
    Person alice{"Alice", 30};
    Person bob{"Bob", 25};

    serializer_->serialize(alice, "alice.dat");
    serializer_->serialize(bob, "bob.dat");

    EXPECT_EQ(serializer_->deserialize("alice.dat"), alice);
    EXPECT_EQ(serializer_->deserialize("bob.dat"), bob);
}

TEST_F(ISerializerTest, OverwriteExistingKey)
{
    Person original{"Original", 10};
    Person updated{"Updated", 20};

    serializer_->serialize(original, "data.dat");
    serializer_->serialize(updated, "data.dat");

    Person result = serializer_->deserialize("data.dat");
    EXPECT_EQ(result, updated);
}

TEST_F(ISerializerTest, SerializeDoesNotThrow)
{
    Person p{"Test", 1};
    EXPECT_NO_THROW(serializer_->serialize(p, "test.dat"));
}

TEST_F(ISerializerTest, DeserializeThrowsForMissingKey)
{
    EXPECT_THROW(serializer_->deserialize("nonexistent.dat"), std::out_of_range);
}
