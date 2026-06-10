/**
 * @file AuthRpcParamTest.cc
 * @brief Unit tests for the server AuthRpcParam class
 * @details Tests cover default construction, parameter construction, Builder pattern,
 *          getters/setters, YAML deserialization, and parameter validation.
 */

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

#include "../../server/src/auth/AuthRpcParam.hpp"

using namespace server_app::auth;

class ServerAuthRpcParamTest : public testing::Test
{
protected:
    std::string tmp_yaml_;

    void SetUp() override
    {
        const auto tmpDir = std::filesystem::temp_directory_path() / "AuthRpcParamTest";
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

TEST_F(ServerAuthRpcParamTest, DefaultConstruction)
{
    const AuthRpcParam param;
    EXPECT_EQ(param.maxConnectionIdleMs(), 60 * 60 * 1000);
    EXPECT_EQ(param.maxConnectionAgeMs(), 2 * 60 * 60 * 1000);
    EXPECT_EQ(param.maxConnectionAgeGraceMs(), 5 * 60 * 1000);
    EXPECT_EQ(param.keepaliveTimeMs(), 30 * 1000);
    EXPECT_EQ(param.keepaliveTimeoutMs(), 5 * 1000);
    EXPECT_EQ(param.keepalivePermitWithoutCalls(), 1);
    EXPECT_EQ(param.serverAddress(), "0.0.0.0:50051");
}

TEST_F(ServerAuthRpcParamTest, ParameterConstruction)
{
    const AuthRpcParam param(1000, 2000, 300, 4000, 5000, 0, "127.0.0.1:8080");
    EXPECT_EQ(param.maxConnectionIdleMs(), 1000);
    EXPECT_EQ(param.maxConnectionAgeMs(), 2000);
    EXPECT_EQ(param.maxConnectionAgeGraceMs(), 300);
    EXPECT_EQ(param.keepaliveTimeMs(), 4000);
    EXPECT_EQ(param.keepaliveTimeoutMs(), 5000);
    EXPECT_EQ(param.keepalivePermitWithoutCalls(), 0);
    EXPECT_EQ(param.serverAddress(), "127.0.0.1:8080");
}

TEST_F(ServerAuthRpcParamTest, ParameterConstruction_ZeroGrace)
{
    EXPECT_NO_THROW(AuthRpcParam(1000, 2000, 0, 4000, 5000, 0, "localhost:50051"));
}

TEST_F(ServerAuthRpcParamTest, ParameterConstruction_ZeroIdleThrows)
{
    EXPECT_THROW(AuthRpcParam(0, 2000, 300, 4000, 5000, 0, "localhost:50051"), std::invalid_argument);
}

TEST_F(ServerAuthRpcParamTest, ParameterConstruction_NegativeAgeThrows)
{
    EXPECT_THROW(AuthRpcParam(1000, -1, 300, 4000, 5000, 0, "localhost:50051"), std::invalid_argument);
}

TEST_F(ServerAuthRpcParamTest, ParameterConstruction_InvalidPermitThrows)
{
    EXPECT_THROW(AuthRpcParam(1000, 2000, 300, 4000, 5000, 2, "localhost:50051"), std::invalid_argument);
}

TEST_F(ServerAuthRpcParamTest, ParameterConstruction_EmptyAddressThrows)
{
    EXPECT_THROW(AuthRpcParam(1000, 2000, 300, 4000, 5000, 0, ""), std::invalid_argument);
}

TEST_F(ServerAuthRpcParamTest, GettersAndSetters)
{
    AuthRpcParam param;
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

TEST_F(ServerAuthRpcParamTest, Builder_DefaultValues)
{
    const auto param = AuthRpcParam::builder().build();
    EXPECT_EQ(param.maxConnectionIdleMs(), 60 * 60 * 1000);
    EXPECT_EQ(param.serverAddress(), "0.0.0.0:50051");
}

TEST_F(ServerAuthRpcParamTest, Builder_CustomValues)
{
    const auto param = AuthRpcParam::builder()
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

TEST_F(ServerAuthRpcParamTest, Builder_InvalidParamThrows)
{
    EXPECT_THROW(AuthRpcParam::builder().keepaliveTimeMs(0).build(), std::invalid_argument);
}

TEST_F(ServerAuthRpcParamTest, Builder_Chaining)
{
    const auto param = AuthRpcParam::builder()
        .maxConnectionIdleMs(100)
        .keepaliveTimeMs(200)
        .serverAddress("localhost:1")
        .build();
    EXPECT_EQ(param.maxConnectionIdleMs(), 100);
    EXPECT_EQ(param.keepaliveTimeMs(), 200);
}

TEST_F(ServerAuthRpcParamTest, DeserializeFromYaml_Full)
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
    AuthRpcParam param;
    param.deserializeFromYamlFile(tmp_yaml_);
    EXPECT_EQ(param.maxConnectionIdleMs(), 5000);
    EXPECT_EQ(param.keepaliveTimeMs(), 3000);
    EXPECT_EQ(param.keepalivePermitWithoutCalls(), 0);
    EXPECT_EQ(param.serverAddress(), "10.0.0.1:9090");
}

TEST_F(ServerAuthRpcParamTest, DeserializeFromYaml_Partial)
{
    writeYaml(R"(
grpc:
  keepaliveTimeMs: 7777
  serverAddress: "localhost:1234"
)");
    AuthRpcParam param;
    param.deserializeFromYamlFile(tmp_yaml_);
    EXPECT_EQ(param.keepaliveTimeMs(), 7777);
    EXPECT_EQ(param.serverAddress(), "localhost:1234");
    EXPECT_EQ(param.maxConnectionIdleMs(), 60 * 60 * 1000);
}

TEST_F(ServerAuthRpcParamTest, DeserializeFromYaml_NonExistentFile)
{
    AuthRpcParam param;
    EXPECT_THROW(param.deserializeFromYamlFile("nonexistent_file_xyz.yaml"), std::runtime_error);
}

TEST_F(ServerAuthRpcParamTest, DeserializeFromYaml_BinaryFile)
{
    {
        std::ofstream f(tmp_yaml_, std::ios::binary);
        const char junk[] = "\xFF\xFE\x00\x01\x02\x03";
        f.write(junk, sizeof(junk) - 1);
    }
    AuthRpcParam param;
    EXPECT_THROW(param.deserializeFromYamlFile(tmp_yaml_), std::runtime_error);
}

TEST_F(ServerAuthRpcParamTest, DeserializeFromYaml_EmptyFile)
{
    writeYaml("");
    AuthRpcParam param;
    EXPECT_NO_THROW(param.deserializeFromYamlFile(tmp_yaml_));
    EXPECT_EQ(param.maxConnectionIdleMs(), 60 * 60 * 1000);
}

TEST_F(ServerAuthRpcParamTest, DeserializeFromYaml_NoGrpcSection)
{
    writeYaml("other: data\nhere: true\n");
    AuthRpcParam param;
    EXPECT_NO_THROW(param.deserializeFromYamlFile(tmp_yaml_));
    EXPECT_EQ(param.serverAddress(), "0.0.0.0:50051");
}

TEST_F(ServerAuthRpcParamTest, YAML_RoundTrip)
{
    AuthRpcParam original(1000, 2000, 300, 4000, 5000, 0, "test:8080");
    YAML::Node node = YAML::convert<server_app::auth::AuthRpcParam>::encode(original);
    AuthRpcParam decoded;
    YAML::convert<server_app::auth::AuthRpcParam>::decode(node, decoded);
    EXPECT_EQ(decoded.maxConnectionIdleMs(), 1000);
    EXPECT_EQ(decoded.serverAddress(), "test:8080");
}

TEST_F(ServerAuthRpcParamTest, YAML_RoundTripDefaults)
{
    const AuthRpcParam original;
    YAML::Node node = YAML::convert<server_app::auth::AuthRpcParam>::encode(original);
    AuthRpcParam decoded;
    YAML::convert<server_app::auth::AuthRpcParam>::decode(node, decoded);
    EXPECT_EQ(decoded.maxConnectionIdleMs(), 60 * 60 * 1000);
    EXPECT_EQ(decoded.serverAddress(), "0.0.0.0:50051");
}
