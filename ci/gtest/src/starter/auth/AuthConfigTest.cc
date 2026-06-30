/**
 * @file AuthConfigTest.cc
 * @brief Unit tests for the unified AuthConfig class
 * @details Merged tests from server and client AuthRpcParam test suites,
 *          covering all 7 configuration parameters, Builder pattern, YAML
 *          deserialization, parameter validation, and edge cases.
 */

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

#include <cppforge/starter/auth/AuthConfig.hpp>

using namespace cppforge::starter::auth;

/// @brief Test fixture for all AuthConfig tests
class AuthConfigTest : public testing::Test
{
protected:
    std::string tmp_yaml_;

    void SetUp() override
    {
        const auto tmpDir = std::filesystem::temp_directory_path() / "AuthConfigTest";
        std::filesystem::create_directories(tmpDir);
        tmp_yaml_ = (tmpDir / "test_config.yaml").string();
    }

    void TearDown() override
    {
        std::error_code ec;
        std::filesystem::remove(tmp_yaml_, ec);
    }

    void writeYaml(const std::string& content)
    {
        std::ofstream f(tmp_yaml_);
        f << content;
    }
};

// ══════════════════════════════════════════════════════════════════════════
//  Server-side parameter tests
// ══════════════════════════════════════════════════════════════════════════

TEST_F(AuthConfigTest, DefaultConstruction)
{
    const AuthConfig param;
    EXPECT_EQ(param.maxConnectionIdleMs(), 60 * 60 * 1000);
    EXPECT_EQ(param.maxConnectionAgeMs(), 2 * 60 * 60 * 1000);
    EXPECT_EQ(param.maxConnectionAgeGraceMs(), 5 * 60 * 1000);
    EXPECT_EQ(param.keepaliveTimeMs(), 30 * 1000);
    EXPECT_EQ(param.keepaliveTimeoutMs(), 5 * 1000);
    EXPECT_EQ(param.keepalivePermitWithoutCalls(), 1);
    EXPECT_EQ(param.serverAddress(), "0.0.0.0:50051");
}

TEST_F(AuthConfigTest, ParameterConstruction)
{
    const AuthConfig param(1000, 2000, 300, 4000, 5000, 0, "127.0.0.1:8080");
    EXPECT_EQ(param.maxConnectionIdleMs(), 1000);
    EXPECT_EQ(param.maxConnectionAgeMs(), 2000);
    EXPECT_EQ(param.maxConnectionAgeGraceMs(), 300);
    EXPECT_EQ(param.keepaliveTimeMs(), 4000);
    EXPECT_EQ(param.keepaliveTimeoutMs(), 5000);
    EXPECT_EQ(param.keepalivePermitWithoutCalls(), 0);
    EXPECT_EQ(param.serverAddress(), "127.0.0.1:8080");
}

TEST_F(AuthConfigTest, ParameterConstruction_ZeroGrace)
{
    EXPECT_NO_THROW(AuthConfig(1000, 2000, 0, 4000, 5000, 0, "localhost:50051"));
}

TEST_F(AuthConfigTest, ParameterConstruction_ZeroIdleThrows)
{
    EXPECT_THROW(AuthConfig(0, 2000, 300, 4000, 5000, 0, "localhost:50051"), std::invalid_argument);
}

TEST_F(AuthConfigTest, ParameterConstruction_NegativeAgeThrows)
{
    EXPECT_THROW(AuthConfig(1000, -1, 300, 4000, 5000, 0, "localhost:50051"), std::invalid_argument);
}

TEST_F(AuthConfigTest, ParameterConstruction_InvalidPermitThrows)
{
    EXPECT_THROW(AuthConfig(1000, 2000, 300, 4000, 5000, 2, "localhost:50051"), std::invalid_argument);
}

TEST_F(AuthConfigTest, ParameterConstruction_EmptyAddressThrows)
{
    EXPECT_THROW(AuthConfig(1000, 2000, 300, 4000, 5000, 0, ""), std::invalid_argument);
}

TEST_F(AuthConfigTest, GettersAndSetters)
{
    AuthConfig param;
    param.maxConnectionIdleMs(5000);
    param.maxConnectionAgeMs(10000);
    param.maxConnectionAgeGraceMs(1000);
    param.keepaliveTimeMs(3000);
    param.keepaliveTimeoutMs(2000);
    param.keepalivePermitWithoutCalls(0);
    param.serverAddress("10.0.0.1:9090");

    EXPECT_EQ(param.maxConnectionIdleMs(), 5000);
    EXPECT_EQ(param.maxConnectionAgeMs(), 10000);
    EXPECT_EQ(param.maxConnectionAgeGraceMs(), 1000);
    EXPECT_EQ(param.keepaliveTimeMs(), 3000);
    EXPECT_EQ(param.keepaliveTimeoutMs(), 2000);
    EXPECT_EQ(param.keepalivePermitWithoutCalls(), 0);
    EXPECT_EQ(param.serverAddress(), "10.0.0.1:9090");
}

TEST_F(AuthConfigTest, Builder_DefaultValues)
{
    const auto param = AuthConfig::builder().build();
    EXPECT_EQ(param.maxConnectionIdleMs(), 60 * 60 * 1000);
    EXPECT_EQ(param.serverAddress(), "0.0.0.0:50051");
}

TEST_F(AuthConfigTest, Builder_CustomValues)
{
    const auto param = AuthConfig::builder()
        .maxConnectionIdleMs(1500)
        .maxConnectionAgeMs(3000)
        .maxConnectionAgeGraceMs(500)
        .keepaliveTimeMs(2000)
        .keepaliveTimeoutMs(1000)
        .keepalivePermitWithoutCalls(0)
        .serverAddress("0.0.0.0:9999")
        .build();

    EXPECT_EQ(param.maxConnectionIdleMs(), 1500);
    EXPECT_EQ(param.keepaliveTimeMs(), 2000);
    EXPECT_EQ(param.keepalivePermitWithoutCalls(), 0);
    EXPECT_EQ(param.serverAddress(), "0.0.0.0:9999");
}

TEST_F(AuthConfigTest, Builder_InvalidParamThrows)
{
    EXPECT_THROW(AuthConfig::builder().keepaliveTimeMs(0).build(), std::invalid_argument);
}

TEST_F(AuthConfigTest, Builder_Chaining)
{
    const auto param = AuthConfig::builder()
        .maxConnectionIdleMs(100)
        .keepaliveTimeMs(200)
        .serverAddress("localhost:1")
        .build();
    EXPECT_EQ(param.maxConnectionIdleMs(), 100);
    EXPECT_EQ(param.keepaliveTimeMs(), 200);
}

// ══════════════════════════════════════════════════════════════════════════
//  YAML deserialization tests
// ══════════════════════════════════════════════════════════════════════════

TEST_F(AuthConfigTest, DeserializeFromYaml_Full)
{
    writeYaml(R"(
grpc:
  maxConnectionIdleMs: 5000
  maxConnectionAgeMs: 10000
  maxConnectionAgeGraceMs: 1000
  keepaliveTimeMs: 3000
  keepaliveTimeoutMs: 2000
  keepalivePermitWithoutCalls: 0
  serverAddress: "10.0.0.1:9090"
)");
    AuthConfig param;
    param.deserializeFromYamlFile(tmp_yaml_);
    EXPECT_EQ(param.maxConnectionIdleMs(), 5000);
    EXPECT_EQ(param.keepaliveTimeMs(), 3000);
    EXPECT_EQ(param.keepalivePermitWithoutCalls(), 0);
    EXPECT_EQ(param.serverAddress(), "10.0.0.1:9090");
}

TEST_F(AuthConfigTest, DeserializeFromYaml_Partial)
{
    writeYaml(R"(
grpc:
  keepaliveTimeMs: 7777
  serverAddress: "localhost:1234"
)");
    AuthConfig param;
    param.deserializeFromYamlFile(tmp_yaml_);
    EXPECT_EQ(param.keepaliveTimeMs(), 7777);
    EXPECT_EQ(param.serverAddress(), "localhost:1234");
    EXPECT_EQ(param.maxConnectionIdleMs(), 60 * 60 * 1000);
}

TEST_F(AuthConfigTest, DeserializeFromYaml_NonExistentFile)
{
    AuthConfig param;
    EXPECT_THROW(param.deserializeFromYamlFile("nonexistent_file_xyz.yaml"), std::runtime_error);
}

TEST_F(AuthConfigTest, DeserializeFromYaml_BinaryFile)
{
    {
        std::ofstream f(tmp_yaml_, std::ios::binary);
        const char junk[] = "\xFF\xFE\x00\x01\x02\x03";
        f.write(junk, sizeof(junk) - 1);
    }
    AuthConfig param;
    EXPECT_THROW(param.deserializeFromYamlFile(tmp_yaml_), std::runtime_error);
}

TEST_F(AuthConfigTest, DeserializeFromYaml_EmptyFile)
{
    writeYaml("");
    AuthConfig param;
    EXPECT_NO_THROW(param.deserializeFromYamlFile(tmp_yaml_));
    EXPECT_EQ(param.maxConnectionIdleMs(), 60 * 60 * 1000);
}

TEST_F(AuthConfigTest, DeserializeFromYaml_NoGrpcSection)
{
    writeYaml("other: data\nhere: true\n");
    AuthConfig param;
    EXPECT_NO_THROW(param.deserializeFromYamlFile(tmp_yaml_));
    EXPECT_EQ(param.serverAddress(), "0.0.0.0:50051");
}

TEST_F(AuthConfigTest, YAML_RoundTrip)
{
    AuthConfig original(1000, 2000, 300, 4000, 5000, 0, "test:8080");
    YAML::Node node = YAML::convert<cppforge::starter::auth::AuthConfig>::encode(original);
    AuthConfig decoded;
    YAML::convert<cppforge::starter::auth::AuthConfig>::decode(node, decoded);
    EXPECT_EQ(decoded.maxConnectionIdleMs(), 1000);
    EXPECT_EQ(decoded.serverAddress(), "test:8080");
}

TEST_F(AuthConfigTest, YAML_RoundTripDefaults)
{
    const AuthConfig original;
    YAML::Node node = YAML::convert<cppforge::starter::auth::AuthConfig>::encode(original);
    AuthConfig decoded;
    YAML::convert<cppforge::starter::auth::AuthConfig>::decode(node, decoded);
    EXPECT_EQ(decoded.maxConnectionIdleMs(), 60 * 60 * 1000);
    EXPECT_EQ(decoded.serverAddress(), "0.0.0.0:50051");
}

// ══════════════════════════════════════════════════════════════════════════
//  Client-side specific tests
// ══════════════════════════════════════════════════════════════════════════

TEST_F(AuthConfigTest, ClientDefaultConstruction)
{
    const AuthConfig param;
    EXPECT_EQ(param.keepaliveTimeMs(), 30 * 1000);
    EXPECT_EQ(param.keepaliveTimeoutMs(), 5 * 1000);
    EXPECT_EQ(param.keepalivePermitWithoutCalls(), 1);
    EXPECT_EQ(param.serverAddress(), "0.0.0.0:50051");
}

TEST_F(AuthConfigTest, CustomConstruction_UsingBuilder)
{
    const auto param = AuthConfig::builder()
        .keepaliveTimeMs(10000)
        .keepaliveTimeoutMs(3000)
        .keepalivePermitWithoutCalls(0)
        .serverAddress("192.168.1.1:8080")
        .build();

    EXPECT_EQ(param.keepaliveTimeMs(), 10000);
    EXPECT_EQ(param.keepaliveTimeoutMs(), 3000);
    EXPECT_EQ(param.keepalivePermitWithoutCalls(), 0);
    EXPECT_EQ(param.serverAddress(), "192.168.1.1:8080");
}

TEST_F(AuthConfigTest, KeepaliveTimeMsSetter)
{
    AuthConfig param;

    param.keepaliveTimeMs(15000);
    EXPECT_EQ(param.keepaliveTimeMs(), 15000);

    param.keepaliveTimeMs(0);
    EXPECT_EQ(param.keepaliveTimeMs(), 0);
}

TEST_F(AuthConfigTest, KeepaliveTimeoutMsSetter)
{
    AuthConfig param;

    param.keepaliveTimeoutMs(8000);
    EXPECT_EQ(param.keepaliveTimeoutMs(), 8000);

    param.keepaliveTimeoutMs(1000);
    EXPECT_EQ(param.keepaliveTimeoutMs(), 1000);
}

TEST_F(AuthConfigTest, KeepalivePermitWithoutCallsSetter)
{
    AuthConfig param;

    param.keepalivePermitWithoutCalls(0);
    EXPECT_EQ(param.keepalivePermitWithoutCalls(), 0);

    param.keepalivePermitWithoutCalls(1);
    EXPECT_EQ(param.keepalivePermitWithoutCalls(), 1);
}

TEST_F(AuthConfigTest, ServerAddressSetter)
{
    AuthConfig param;

    param.serverAddress("10.0.0.1:50051");
    EXPECT_EQ(param.serverAddress(), "10.0.0.1:50051");

    param.serverAddress("example.com:443");
    EXPECT_EQ(param.serverAddress(), "example.com:443");
}

TEST_F(AuthConfigTest, BuilderDefaultValues)
{
    const auto param = AuthConfig::builder().build();

    EXPECT_EQ(param.keepaliveTimeMs(), 30 * 1000);
    EXPECT_EQ(param.keepaliveTimeoutMs(), 5 * 1000);
    EXPECT_EQ(param.keepalivePermitWithoutCalls(), 1);
    EXPECT_EQ(param.serverAddress(), "0.0.0.0:50051");
}

TEST_F(AuthConfigTest, BuilderSingleField)
{
    const auto param = AuthConfig::builder()
        .serverAddress("10.0.0.1:50051")
        .build();

    EXPECT_EQ(param.serverAddress(), "10.0.0.1:50051");
    EXPECT_EQ(param.keepaliveTimeMs(), 30 * 1000);
    EXPECT_EQ(param.keepaliveTimeoutMs(), 5 * 1000);
    EXPECT_EQ(param.keepalivePermitWithoutCalls(), 1);
}

TEST_F(AuthConfigTest, BuilderAllFields)
{
    const auto param = AuthConfig::builder()
        .keepaliveTimeMs(10000)
        .keepaliveTimeoutMs(3000)
        .keepalivePermitWithoutCalls(0)
        .serverAddress("192.168.1.1:8080")
        .build();

    EXPECT_EQ(param.keepaliveTimeMs(), 10000);
    EXPECT_EQ(param.keepaliveTimeoutMs(), 3000);
    EXPECT_EQ(param.keepalivePermitWithoutCalls(), 0);
    EXPECT_EQ(param.serverAddress(), "192.168.1.1:8080");
}

TEST_F(AuthConfigTest, BuilderChaining)
{
    auto& ref1 = AuthConfig::builder().keepaliveTimeMs(10000);
    auto& ref2 = ref1.keepaliveTimeoutMs(5000);
    auto& ref3 = ref2.keepalivePermitWithoutCalls(0);
    const auto& ref4 = ref3.serverAddress("127.0.0.1:50051");

    const auto param = ref4.build();
    EXPECT_EQ(param.keepaliveTimeMs(), 10000);
    EXPECT_EQ(param.keepaliveTimeoutMs(), 5000);
    EXPECT_EQ(param.keepalivePermitWithoutCalls(), 0);
    EXPECT_EQ(param.serverAddress(), "127.0.0.1:50051");
}

TEST_F(AuthConfigTest, BuilderIndependentBuilds)
{
    auto builder = AuthConfig::builder()
        .keepaliveTimeMs(10000);

    const auto param1 = builder.build();
    static_cast<void>(builder.keepaliveTimeMs(20000));
    const auto param2 = builder.build();

    EXPECT_EQ(param1.keepaliveTimeMs(), 10000);
    EXPECT_EQ(param2.keepaliveTimeMs(), 20000);
}

TEST_F(AuthConfigTest, CopyableNotAssignable)
{
    EXPECT_TRUE(std::is_copy_constructible_v<AuthConfig>);
    EXPECT_FALSE(std::is_copy_assignable_v<AuthConfig>);
}