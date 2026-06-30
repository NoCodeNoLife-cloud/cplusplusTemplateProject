/**
 * @file MetricsTest.cc
 * @brief Unit tests for the metrics subsystem.
 * @details Tests cover counter, gauge, histogram operations, and Prometheus serialization.
 */

#include <string>
#include <gtest/gtest.h>

#include <cppforge/observability/metrics/Counter.hpp>
#include <cppforge/observability/metrics/Gauge.hpp>
#include <cppforge/observability/metrics/Histogram.hpp>
#include <cppforge/observability/metrics/PrometheusExporter.hpp>

using namespace cppforge::observability::metrics;

/// @brief Test fixture for Counter tests.
class MetricsCounterTest : public testing::Test
{
  protected:
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @brief Test counter initial value is zero.
 */
TEST_F(MetricsCounterTest, InitialValueIsZero)
{
    Counter counter;
    EXPECT_DOUBLE_EQ(counter.get(), 0.0);
}

/**
 * @brief Test counter increment.
 */
TEST_F(MetricsCounterTest, Increment)
{
    Counter counter;
    counter.increment();
    EXPECT_DOUBLE_EQ(counter.get(), 1.0);
    counter.increment(5.0);
    EXPECT_DOUBLE_EQ(counter.get(), 6.0);
}

/// @brief Test fixture for Gauge tests.
class MetricsGaugeTest : public testing::Test
{
  protected:
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @brief Test gauge initial value is zero.
 */
TEST_F(MetricsGaugeTest, InitialValueIsZero)
{
    Gauge gauge;
    EXPECT_DOUBLE_EQ(gauge.get(), 0.0);
}

/**
 * @brief Test gauge set value.
 */
TEST_F(MetricsGaugeTest, SetValue)
{
    Gauge gauge;
    gauge.set(42.5);
    EXPECT_DOUBLE_EQ(gauge.get(), 42.5);
    gauge.set(-10.0);
    EXPECT_DOUBLE_EQ(gauge.get(), -10.0);
}

/// @brief Test fixture for Histogram tests.
class MetricsHistogramTest : public testing::Test
{
  protected:
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @brief Test histogram observe and count.
 */
TEST_F(MetricsHistogramTest, ObserveAndCount)
{
    Histogram hist({1.0, 5.0, 10.0});
    hist.observe(0.5);
    hist.observe(3.0);
    hist.observe(7.0);
    hist.observe(15.0);

    EXPECT_EQ(hist.count(), 4u);
    EXPECT_DOUBLE_EQ(hist.sum(), 25.5);
}

/**
 * @brief Test histogram cumulative counts.
 */
TEST_F(MetricsHistogramTest, CumulativeCounts)
{
    Histogram hist({1.0, 5.0, 10.0});
    hist.observe(0.5);  // bucket 0
    hist.observe(3.0);  // bucket 1
    hist.observe(7.0);  // bucket 2
    hist.observe(15.0); // bucket 3 (+Inf)

    auto cumulative = hist.cumulativeCounts();
    ASSERT_EQ(cumulative.size(), 4u);
    EXPECT_EQ(cumulative[0], 1u); // <= 1.0
    EXPECT_EQ(cumulative[1], 2u); // <= 5.0
    EXPECT_EQ(cumulative[2], 3u); // <= 10.0
    EXPECT_EQ(cumulative[3], 4u); // +Inf
}

/// @brief Test fixture for PrometheusExporter tests.
class MetricsPrometheusExporterTest : public testing::Test
{
  protected:
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @brief Test Prometheus counter serialization.
 */
TEST_F(MetricsPrometheusExporterTest, CounterSerialization)
{
    PrometheusExporter exporter;
    exporter.registerCounter("http_requests_total", "Total HTTP requests");
    exporter.incrementCounter("http_requests_total", 42.0);

    std::string output = exporter.serialize();
    EXPECT_NE(output.find("# HELP http_requests_total Total HTTP requests"), std::string::npos);
    EXPECT_NE(output.find("# TYPE http_requests_total counter"), std::string::npos);
    EXPECT_NE(output.find("http_requests_total 42"), std::string::npos);
}

/**
 * @brief Test Prometheus gauge serialization.
 */
TEST_F(MetricsPrometheusExporterTest, GaugeSerialization)
{
    PrometheusExporter exporter;
    exporter.registerGauge("temperature", "Current temperature");
    exporter.setGauge("temperature", 23.5);

    std::string output = exporter.serialize();
    EXPECT_NE(output.find("# HELP temperature Current temperature"), std::string::npos);
    EXPECT_NE(output.find("# TYPE temperature gauge"), std::string::npos);
    EXPECT_NE(output.find("temperature 23.5"), std::string::npos);
}

/**
 * @brief Test Prometheus histogram serialization.
 */
TEST_F(MetricsPrometheusExporterTest, HistogramSerialization)
{
    PrometheusExporter exporter;
    exporter.registerHistogram("request_duration", "Request duration in seconds", {0.1, 0.5, 1.0});
    exporter.observeHistogram("request_duration", 0.3);

    std::string output = exporter.serialize();
    EXPECT_NE(output.find("# HELP request_duration Request duration in seconds"), std::string::npos);
    EXPECT_NE(output.find("# TYPE request_duration histogram"), std::string::npos);
    EXPECT_NE(output.find("request_duration_bucket{le=\"0.1\"} 0"), std::string::npos);
    EXPECT_NE(output.find("request_duration_bucket{le=\"0.5\"} 1"), std::string::npos);
    EXPECT_NE(output.find("request_duration_count 1"), std::string::npos);
}

/**
 * @brief Test Prometheus serialization with no metrics.
 */
TEST_F(MetricsPrometheusExporterTest, EmptySerialization)
{
    PrometheusExporter exporter;
    std::string output = exporter.serialize();
    EXPECT_TRUE(output.empty());
}
