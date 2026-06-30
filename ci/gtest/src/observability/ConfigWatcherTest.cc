/**
 * @file ConfigWatcherTest.cc
 * @brief Unit tests for the ConfigWatcher file-change detection subsystem.
 * @details Tests cover file change detection, callback invocation, start/stop, and non-existent file handling.
 */

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <thread>
#include <gtest/gtest.h>

#include <cppforge/observability/config/ConfigWatcher.hpp>

using namespace cppforge::observability::config;
namespace fs = std::filesystem;

/// @brief Test fixture for ConfigWatcher tests.
class ConfigWatcherTest : public testing::Test
{
  protected:
    void SetUp() override
    {
        tempDir_ = fs::temp_directory_path() / "cppforge_config_watcher_test";
        fs::create_directories(tempDir_);
    }

    void TearDown() override
    {
        std::error_code ec;
        fs::remove_all(tempDir_, ec);
    }

    /// @brief Helper to write content to a file.
    void writeFile(const fs::path &path, const std::string &content)
    {
        std::ofstream ofs(path);
        ofs << content;
        ofs.close();
    }

    fs::path tempDir_;
};

/**
 * @brief Test file change detection triggers callback.
 * @details Creates a temp file, starts the watcher, modifies the file, and verifies the callback fires.
 */
TEST_F(ConfigWatcherTest, FileChangeDetectionTriggersCallback)
{
    auto filePath = tempDir_ / "config.yml";
    writeFile(filePath, "initial: true\n");

    ConfigWatcher watcher(filePath.string());

    std::atomic<bool> callbackFired{false};
    std::string receivedContent;

    watcher.onChange([&](const std::string &content) {
        receivedContent = content;
        callbackFired.store(true, std::memory_order_release);
    });

    watcher.start(std::chrono::milliseconds(50));

    // Allow the watcher to read the initial content
    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    // Modify the file
    writeFile(filePath, "initial: false\nupdated: yes\n");

    // Wait for the watcher to detect the change
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
    while (!callbackFired.load(std::memory_order_acquire) && std::chrono::steady_clock::now() < deadline)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    watcher.stop();

    EXPECT_TRUE(callbackFired.load(std::memory_order_acquire));
    EXPECT_NE(receivedContent.find("updated: yes"), std::string::npos);
}

/**
 * @brief Test callback receives new file content.
 */
TEST_F(ConfigWatcherTest, CallbackReceivesNewContent)
{
    auto filePath = tempDir_ / "settings.json";
    writeFile(filePath, "{\"version\": 1}");

    ConfigWatcher watcher(filePath.string());

    std::string capturedContent;
    std::atomic<bool> fired{false};

    watcher.onChange([&](const std::string &content) {
        capturedContent = content;
        fired.store(true, std::memory_order_release);
    });

    watcher.start(std::chrono::milliseconds(50));
    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    writeFile(filePath, "{\"version\": 2, \"new_field\": true}");

    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
    while (!fired.load(std::memory_order_acquire) && std::chrono::steady_clock::now() < deadline)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    watcher.stop();

    EXPECT_TRUE(fired.load(std::memory_order_acquire));
    EXPECT_NE(capturedContent.find("\"version\": 2"), std::string::npos);
    EXPECT_NE(capturedContent.find("\"new_field\": true"), std::string::npos);
}

/**
 * @brief Test start and stop functionality.
 * @details Verifies that stop() halts the watcher and no further callbacks fire.
 */
TEST_F(ConfigWatcherTest, StartStopFunctionality)
{
    auto filePath = tempDir_ / "stop_test.yml";
    writeFile(filePath, "data: 1\n");

    ConfigWatcher watcher(filePath.string());

    std::atomic<int> callbackCount{0};
    watcher.onChange([&](const std::string & /*content*/) { callbackCount.fetch_add(1, std::memory_order_relaxed); });

    watcher.start(std::chrono::milliseconds(50));
    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    // Modify file to trigger callback
    writeFile(filePath, "data: 2\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    // Stop the watcher
    watcher.stop();

    int countAfterStop = callbackCount.load(std::memory_order_relaxed);
    EXPECT_GE(countAfterStop, 1);

    // Modify file again - callback should NOT fire
    writeFile(filePath, "data: 3\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    EXPECT_EQ(callbackCount.load(std::memory_order_relaxed), countAfterStop);
}

/**
 * @brief Test watching a non-existent file does not crash.
 * @details The watcher should handle a missing file gracefully (readFile returns empty string).
 */
TEST_F(ConfigWatcherTest, NonExistentFileHandledGracefully)
{
    auto filePath = tempDir_ / "does_not_exist.yml";

    ConfigWatcher watcher(filePath.string());

    std::atomic<bool> callbackFired{false};
    watcher.onChange([&](const std::string & /*content*/) { callbackFired.store(true, std::memory_order_release); });

    // Should not throw when starting with a non-existent file
    EXPECT_NO_THROW(watcher.start(std::chrono::milliseconds(50)));

    // Let it run a few cycles
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // No callback should fire since file content stays empty
    EXPECT_FALSE(callbackFired.load(std::memory_order_acquire));

    watcher.stop();
}

/**
 * @brief Test that creating a previously non-existent file triggers callback.
 */
TEST_F(ConfigWatcherTest, CreatingFileTriggersCallback)
{
    auto filePath = tempDir_ / "late_creation.yml";
    // File does not exist yet

    ConfigWatcher watcher(filePath.string());

    std::atomic<bool> callbackFired{false};
    watcher.onChange([&](const std::string & /*content*/) { callbackFired.store(true, std::memory_order_release); });

    watcher.start(std::chrono::milliseconds(50));
    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    // Now create the file
    writeFile(filePath, "new: content\n");

    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
    while (!callbackFired.load(std::memory_order_acquire) && std::chrono::steady_clock::now() < deadline)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    watcher.stop();

    EXPECT_TRUE(callbackFired.load(std::memory_order_acquire));
}

/**
 * @brief Test double start is idempotent.
 */
TEST_F(ConfigWatcherTest, DoubleStartIsIdempotent)
{
    auto filePath = tempDir_ / "double_start.yml";
    writeFile(filePath, "data: 1\n");

    ConfigWatcher watcher(filePath.string());

    EXPECT_NO_THROW(watcher.start(std::chrono::milliseconds(50)));
    EXPECT_NO_THROW(watcher.start(std::chrono::milliseconds(50)));

    watcher.stop();
}
