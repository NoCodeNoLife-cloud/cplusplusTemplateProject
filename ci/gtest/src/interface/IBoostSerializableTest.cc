/**
 * @file IBoostSerializableTest.cc
 * @brief Unit tests for IBoostSerializable interface
 * @details Tests cover serialization round-trip and error handling via a mock implementation.
 */

#include <sstream>
#include <gtest/gtest.h>

#include "interface/serialization/IBoostSerializable.hpp"

using namespace common::interface::serialization;

namespace
{
    /// @brief Mock implementation of IBoostSerializable for testing
    class MockBoostSerializable : public IBoostSerializable<MockBoostSerializable>
    {
    public:
        std::string name_;
        int value_ = 0;
        bool throwOnSerialize_ = false;

        MockBoostSerializable() = default;
        MockBoostSerializable(std::string name, int value)
            : name_(std::move(name)), value_(value) {}

        bool operator==(const MockBoostSerializable& other) const
        {
            return name_ == other.name_ && value_ == other.value_;
        }

        template <class Archive>
        void serializeImpl(Archive& archive, [[maybe_unused]] const unsigned int version)
        {
            archive& boost::serialization::make_nvp("name", name_);
            archive& boost::serialization::make_nvp("value", value_);
        }
    };
}

class IBoostSerializableTest : public testing::Test
{
protected:
    void SetUp() override
    {
        obj_.reset(new MockBoostSerializable("test", 42));
    }

    void TearDown() override
    {
        obj_.reset();
    }

    std::unique_ptr<MockBoostSerializable> obj_;
};

TEST_F(IBoostSerializableTest, SerializeToStream)
{
    std::ostringstream oss;
    bool result = obj_->serializeTo(oss);

    ASSERT_TRUE(result);
    EXPECT_FALSE(oss.str().empty());
}

TEST_F(IBoostSerializableTest, RoundTrip)
{
    std::ostringstream oss;
    ASSERT_TRUE(obj_->serializeTo(oss));

    MockBoostSerializable deserialized;
    std::istringstream iss(oss.str());
    bool result = deserialized.deserializeFrom(iss);

    ASSERT_TRUE(result);
    EXPECT_EQ(deserialized.name_, obj_->name_);
    EXPECT_EQ(deserialized.value_, obj_->value_);
}

TEST_F(IBoostSerializableTest, MultipleFieldsRoundTrip)
{
    obj_->name_ = "hello";
    obj_->value_ = 99;

    std::ostringstream oss;
    ASSERT_TRUE(obj_->serializeTo(oss));

    MockBoostSerializable deserialized;
    std::istringstream iss(oss.str());
    ASSERT_TRUE(deserialized.deserializeFrom(iss));

    EXPECT_EQ(deserialized.name_, "hello");
    EXPECT_EQ(deserialized.value_, 99);
}

TEST_F(IBoostSerializableTest, SerializeTwice)
{
    std::ostringstream oss1;
    ASSERT_TRUE(obj_->serializeTo(oss1));

    obj_->name_ = "second";
    obj_->value_ = 77;

    std::ostringstream oss2;
    ASSERT_TRUE(obj_->serializeTo(oss2));

    EXPECT_NE(oss1.str(), oss2.str());

    MockBoostSerializable d1;
    std::istringstream iss1(oss1.str());
    ASSERT_TRUE(d1.deserializeFrom(iss1));
    EXPECT_EQ(d1.name_, "test");
    EXPECT_EQ(d1.value_, 42);

    MockBoostSerializable d2;
    std::istringstream iss2(oss2.str());
    ASSERT_TRUE(d2.deserializeFrom(iss2));
    EXPECT_EQ(d2.name_, "second");
    EXPECT_EQ(d2.value_, 77);
}
