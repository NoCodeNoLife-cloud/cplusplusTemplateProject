/**
 * @file ISerializerTest.cc
 * @brief Unit tests for ISerializer interface
 * @details Tests cover serialization and deserialization via a mock implementation.
 */

#include <sstream>
#include <string>
#include <unordered_map>
#include <gtest/gtest.h>

#include <cppforge/interface/serialization/ISerializer.hpp>

using namespace cppforge::interface::serialization;

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

/// @brief Test fixture for ISerializer tests.
class ISerializerTest : public testing::Test
{
protected:
    void SetUp() override
    {
        serializer_ = std::make_unique<MockPersonSerializer>();
    }

    void TearDown() override
    {
        serializer_.reset();
    }

    std::unique_ptr<MockPersonSerializer> serializer_;
};

/** @brief Tests serialize then deserialize preserves data.
 *  @details Verifies Person data is identical after a round-trip through the serializer.
 */
TEST_F(ISerializerTest, SerializeAndDeserializePreservesData)
{
    const Person alice{"Alice", 30};

    serializer_->serialize(alice, "alice.dat");
    const Person result = serializer_->deserialize("alice.dat");

    EXPECT_EQ(result, alice);
}

/** @brief Tests multiple serializations with distinct keys.
 *  @details Verifies each key retrieves its own Person independently.
 */
TEST_F(ISerializerTest, MultipleSerializationsWithDifferentKeys)
{
    const Person alice{"Alice", 30};
    const Person bob{"Bob", 25};

    serializer_->serialize(alice, "alice.dat");
    serializer_->serialize(bob, "bob.dat");

    EXPECT_EQ(serializer_->deserialize("alice.dat"), alice);
    EXPECT_EQ(serializer_->deserialize("bob.dat"), bob);
}

/** @brief Tests overwriting existing key replaces stored data.
 *  @details Verifies deserialize returns the latest value for a reused key.
 */
TEST_F(ISerializerTest, OverwriteExistingKey)
{
    const Person original{"Original", 10};
    const Person updated{"Updated", 20};

    serializer_->serialize(original, "data.dat");
    serializer_->serialize(updated, "data.dat");

    const Person result = serializer_->deserialize("data.dat");
    EXPECT_EQ(result, updated);
}

/** @brief Tests serialize does not throw for valid input.
 *  @details Verifies serializing a valid Person object completes without exception.
 */
TEST_F(ISerializerTest, SerializeDoesNotThrow)
{
    const Person p{"Test", 1};
    EXPECT_NO_THROW(serializer_->serialize(p, "test.dat"));
}

/** @brief Tests deserialize throws for missing key.
 *  @details Verifies std::out_of_range is thrown when filename does not exist.
 */
TEST_F(ISerializerTest, DeserializeThrowsForMissingKey)
{
    EXPECT_THROW(serializer_->deserialize("nonexistent.dat"), std::out_of_range);
}
