/**
 * @file RpcMetadataTest.cc
 * @brief Unit tests for the RpcMetadata class
 * @details Tests cover core RPC metadata functionality including gRPC connectivity state
 *          conversions between different representations (enum to string, grpc enum to internal enum).
 */

#include <grpc/impl/connectivity_state.h>
#include <gtest/gtest.h>

#include <cppforge/rpc/RpcMetadata.hpp>

using namespace cppforge::rpc;

/**
 * @brief Test fixture for RpcMetadataTest tests
 */
class RpcMetadataTest : public testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

/**
 * @brief Test grpcStateToString with GRPC_CHANNEL_IDLE
 * @details Verifies conversion of IDLE state to string representation
 */
TEST_F(RpcMetadataTest, GrpcStateToString_Idle)
{
    const auto result = RpcMetadata::grpcStateToString(GRPC_CHANNEL_IDLE);
    EXPECT_EQ(result, "IDLE");
}

/**
 * @brief Test grpcStateToString with GRPC_CHANNEL_CONNECTING
 * @details Verifies conversion of CONNECTING state to string representation
 */
TEST_F(RpcMetadataTest, GrpcStateToString_Connecting)
{
    const auto result = RpcMetadata::grpcStateToString(GRPC_CHANNEL_CONNECTING);
    EXPECT_EQ(result, "CONNECTING");
}

/**
 * @brief Test grpcStateToString with GRPC_CHANNEL_READY
 * @details Verifies conversion of READY state to string representation
 */
TEST_F(RpcMetadataTest, GrpcStateToString_Ready)
{
    const auto result = RpcMetadata::grpcStateToString(GRPC_CHANNEL_READY);
    EXPECT_EQ(result, "READY");
}

/**
 * @brief Test grpcStateToString with GRPC_CHANNEL_TRANSIENT_FAILURE
 * @details Verifies conversion of TRANSIENT_FAILURE state to string representation
 */
TEST_F(RpcMetadataTest, GrpcStateToString_TransientFailure)
{
    const auto result = RpcMetadata::grpcStateToString(GRPC_CHANNEL_TRANSIENT_FAILURE);
    EXPECT_EQ(result, "TRANSIENT_FAILURE");
}

/**
 * @brief Test grpcStateToString with GRPC_CHANNEL_SHUTDOWN
 * @details Verifies conversion of SHUTDOWN state to string representation
 */
TEST_F(RpcMetadataTest, GrpcStateToString_Shutdown)
{
    const auto result = RpcMetadata::grpcStateToString(GRPC_CHANNEL_SHUTDOWN);
    EXPECT_EQ(result, "SHUTDOWN");
}

/**
 * @brief Test grpcStateToString with unknown state
 * @details Verifies that unknown states return "UNKNOWN" string
 */
TEST_F(RpcMetadataTest, GrpcStateToString_Unknown)
{
    // Use an invalid state value to test unknown handling
    const auto result = RpcMetadata::grpcStateToString(static_cast<grpc_connectivity_state>(999));
    EXPECT_EQ(result, "UNKNOWN");
}

/**
 * @brief Test grpcStateToEnum with GRPC_CHANNEL_IDLE
 * @details Verifies conversion of gRPC IDLE state to internal enum
 */
TEST_F(RpcMetadataTest, GrpcStateToEnum_Idle)
{
    const auto result = RpcMetadata::grpcStateToEnum(GRPC_CHANNEL_IDLE);
    EXPECT_EQ(result, GrpcConnectivityState::IDLE);
}

/**
 * @brief Test grpcStateToEnum with GRPC_CHANNEL_CONNECTING
 * @details Verifies conversion of gRPC CONNECTING state to internal enum
 */
TEST_F(RpcMetadataTest, GrpcStateToEnum_Connecting)
{
    const auto result = RpcMetadata::grpcStateToEnum(GRPC_CHANNEL_CONNECTING);
    EXPECT_EQ(result, GrpcConnectivityState::CONNECTING);
}

/**
 * @brief Test grpcStateToEnum with GRPC_CHANNEL_READY
 * @details Verifies conversion of gRPC READY state to internal enum
 */
TEST_F(RpcMetadataTest, GrpcStateToEnum_Ready)
{
    const auto result = RpcMetadata::grpcStateToEnum(GRPC_CHANNEL_READY);
    EXPECT_EQ(result, GrpcConnectivityState::READY);
}

/**
 * @brief Test grpcStateToEnum with GRPC_CHANNEL_TRANSIENT_FAILURE
 * @details Verifies conversion of gRPC TRANSIENT_FAILURE state to internal enum
 */
TEST_F(RpcMetadataTest, GrpcStateToEnum_TransientFailure)
{
    const auto result = RpcMetadata::grpcStateToEnum(GRPC_CHANNEL_TRANSIENT_FAILURE);
    EXPECT_EQ(result, GrpcConnectivityState::TRANSIENT_FAILURE);
}

/**
 * @brief Test grpcStateToEnum with GRPC_CHANNEL_SHUTDOWN
 * @details Verifies conversion of gRPC SHUTDOWN state to internal enum
 */
TEST_F(RpcMetadataTest, GrpcStateToEnum_Shutdown)
{
    const auto result = RpcMetadata::grpcStateToEnum(GRPC_CHANNEL_SHUTDOWN);
    EXPECT_EQ(result, GrpcConnectivityState::SHUTDOWN);
}

/**
 * @brief Test grpcStateToEnum with unknown state
 * @details Verifies that unknown states default to IDLE enum value
 */
TEST_F(RpcMetadataTest, GrpcStateToEnum_Unknown)
{
    // Use an invalid state value to test default handling
    const auto result = RpcMetadata::grpcStateToEnum(static_cast<grpc_connectivity_state>(999));
    EXPECT_EQ(result, GrpcConnectivityState::IDLE);
}

/**
 * @brief Test grpcStateToString with GrpcConnectivityState::IDLE
 * @details Verifies conversion of internal IDLE enum to string
 */
TEST_F(RpcMetadataTest, InternalStateToString_Idle)
{
    const auto result = RpcMetadata::grpcStateToString(GrpcConnectivityState::IDLE);
    EXPECT_EQ(result, "IDLE");
}

/**
 * @brief Test grpcStateToString with GrpcConnectivityState::CONNECTING
 * @details Verifies conversion of internal CONNECTING enum to string
 */
TEST_F(RpcMetadataTest, InternalStateToString_Connecting)
{
    const auto result = RpcMetadata::grpcStateToString(GrpcConnectivityState::CONNECTING);
    EXPECT_EQ(result, "CONNECTING");
}

/**
 * @brief Test grpcStateToString with GrpcConnectivityState::READY
 * @details Verifies conversion of internal READY enum to string
 */
TEST_F(RpcMetadataTest, InternalStateToString_Ready)
{
    const auto result = RpcMetadata::grpcStateToString(GrpcConnectivityState::READY);
    EXPECT_EQ(result, "READY");
}

/**
 * @brief Test grpcStateToString with GrpcConnectivityState::TRANSIENT_FAILURE
 * @details Verifies conversion of internal TRANSIENT_FAILURE enum to string
 */
TEST_F(RpcMetadataTest, InternalStateToString_TransientFailure)
{
    const auto result = RpcMetadata::grpcStateToString(GrpcConnectivityState::TRANSIENT_FAILURE);
    EXPECT_EQ(result, "TRANSIENT_FAILURE");
}

/**
 * @brief Test grpcStateToString with GrpcConnectivityState::SHUTDOWN
 * @details Verifies conversion of internal SHUTDOWN enum to string
 */
TEST_F(RpcMetadataTest, InternalStateToString_Shutdown)
{
    const auto result = RpcMetadata::grpcStateToString(GrpcConnectivityState::SHUTDOWN);
    EXPECT_EQ(result, "SHUTDOWN");
}

/**
 * @brief Test round-trip conversion: gRPC state -> internal enum -> string
 * @details Verifies consistency across different conversion methods
 */
TEST_F(RpcMetadataTest, RoundTrip_GrpcToInternalToString)
{
    const grpc_connectivity_state grpcState = GRPC_CHANNEL_READY;
    const auto internalEnum = RpcMetadata::grpcStateToEnum(grpcState);
    const auto strFromInternal = RpcMetadata::grpcStateToString(internalEnum);
    const auto strFromGrpc = RpcMetadata::grpcStateToString(grpcState);

    EXPECT_EQ(strFromInternal, strFromGrpc);
    EXPECT_EQ(strFromInternal, "READY");
}

/**
 * @brief Test all gRPC connectivity states have consistent string representations
 * @details Verifies that converting through internal enum produces same result as direct conversion
 */
TEST_F(RpcMetadataTest, Consistency_AllStates)
{
    // Test IDLE
    {
        const auto direct = RpcMetadata::grpcStateToString(GRPC_CHANNEL_IDLE);
        const auto viaEnum = RpcMetadata::grpcStateToString(RpcMetadata::grpcStateToEnum(GRPC_CHANNEL_IDLE));
        EXPECT_EQ(direct, viaEnum);
    }

    // Test CONNECTING
    {
        const auto direct = RpcMetadata::grpcStateToString(GRPC_CHANNEL_CONNECTING);
        const auto viaEnum = RpcMetadata::grpcStateToString(RpcMetadata::grpcStateToEnum(GRPC_CHANNEL_CONNECTING));
        EXPECT_EQ(direct, viaEnum);
    }

    // Test READY
    {
        const auto direct = RpcMetadata::grpcStateToString(GRPC_CHANNEL_READY);
        const auto viaEnum = RpcMetadata::grpcStateToString(RpcMetadata::grpcStateToEnum(GRPC_CHANNEL_READY));
        EXPECT_EQ(direct, viaEnum);
    }

    // Test TRANSIENT_FAILURE
    {
        const auto direct = RpcMetadata::grpcStateToString(GRPC_CHANNEL_TRANSIENT_FAILURE);
        const auto viaEnum = RpcMetadata::grpcStateToString(RpcMetadata::grpcStateToEnum(GRPC_CHANNEL_TRANSIENT_FAILURE));
        EXPECT_EQ(direct, viaEnum);
    }

    // Test SHUTDOWN
    {
        const auto direct = RpcMetadata::grpcStateToString(GRPC_CHANNEL_SHUTDOWN);
        const auto viaEnum = RpcMetadata::grpcStateToString(RpcMetadata::grpcStateToEnum(GRPC_CHANNEL_SHUTDOWN));
        EXPECT_EQ(direct, viaEnum);
    }
}

/**
 * @brief Test all conversion methods return non-empty strings
 * @details Ensures no conversion method returns an empty string
 */
TEST_F(RpcMetadataTest, NonEmpty_Results)
{
    EXPECT_FALSE(RpcMetadata::grpcStateToString(GRPC_CHANNEL_IDLE).empty());
    EXPECT_FALSE(RpcMetadata::grpcStateToString(GRPC_CHANNEL_CONNECTING).empty());
    EXPECT_FALSE(RpcMetadata::grpcStateToString(GRPC_CHANNEL_READY).empty());
    EXPECT_FALSE(RpcMetadata::grpcStateToString(GRPC_CHANNEL_TRANSIENT_FAILURE).empty());
    EXPECT_FALSE(RpcMetadata::grpcStateToString(GRPC_CHANNEL_SHUTDOWN).empty());

    EXPECT_FALSE(RpcMetadata::grpcStateToString(GrpcConnectivityState::IDLE).empty());
    EXPECT_FALSE(RpcMetadata::grpcStateToString(GrpcConnectivityState::CONNECTING).empty());
    EXPECT_FALSE(RpcMetadata::grpcStateToString(GrpcConnectivityState::READY).empty());
    EXPECT_FALSE(RpcMetadata::grpcStateToString(GrpcConnectivityState::TRANSIENT_FAILURE).empty());
    EXPECT_FALSE(RpcMetadata::grpcStateToString(GrpcConnectivityState::SHUTDOWN).empty());
}
