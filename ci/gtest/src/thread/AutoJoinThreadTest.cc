/**
 * @file AutoJoinThreadTest.cc
 * @brief Unit tests for AutoJoinThread class
 * @details Tests cover construction, move semantics, join, detach operations
 *          and RAII auto-join behavior in destructor.
 */

#include <atomic>
#include <chrono>
#include <thread>
#include <gtest/gtest.h>

#include "thread/AutoJoinThread.hpp"

using namespace common::thread;

class AutoJoinThreadTest : public testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

TEST_F(AutoJoinThreadTest, MovedFrom_NotJoinable)
{
    AutoJoinThread t([] {});
    EXPECT_TRUE(t.joinable());
    AutoJoinThread moved(std::move(t));
    EXPECT_FALSE(t.joinable());
    moved.join();
}

TEST_F(AutoJoinThreadTest, ConstructWithCallable_Joinable)
{
    AutoJoinThread t([] { std::this_thread::sleep_for(std::chrono::milliseconds(10)); });
    EXPECT_TRUE(t.joinable());
    t.join();
}

TEST_F(AutoJoinThreadTest, Join_WaitsForCompletion)
{
    {
        std::atomic<bool> flag{false};
        AutoJoinThread t([&flag]
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            flag = true;
        });
        t.join();
        EXPECT_TRUE(flag);
    }
}

TEST_F(AutoJoinThreadTest, Join_AlreadyJoined_NoOp)
{
    AutoJoinThread t([] { std::this_thread::sleep_for(std::chrono::milliseconds(5)); });
    t.join();
    EXPECT_FALSE(t.joinable());
    t.join();
    EXPECT_FALSE(t.joinable());
}

TEST_F(AutoJoinThreadTest, Detach_ReleasesOwnership)
{
    {
        std::atomic<bool> flag{false};
        {
            AutoJoinThread t([&flag]
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(30));
                flag = true;
            });
            t.detach();
            EXPECT_FALSE(t.joinable());
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        EXPECT_TRUE(flag);
    }
}

TEST_F(AutoJoinThreadTest, MoveConstructor_TransfersOwnership)
{
    {
        std::atomic<bool> flag{false};
        AutoJoinThread t1([&flag]
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            flag = true;
        });
        EXPECT_TRUE(t1.joinable());

        AutoJoinThread t2(std::move(t1));
        EXPECT_FALSE(t1.joinable());
        EXPECT_TRUE(t2.joinable());
        t2.join();
        EXPECT_TRUE(flag);
    }
}

TEST_F(AutoJoinThreadTest, MoveAssignment_JoinsCurrentAndTransfers)
{
    {
        std::atomic<bool> flag1{false};
        std::atomic<bool> flag2{false};

        AutoJoinThread t1([&flag1]
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            flag1 = true;
        });
        AutoJoinThread t2([&flag2]
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            flag2 = true;
        });

        // Move assignment joins t2's original thread (flag2 becomes true),
        // then transfers t1's thread ownership to t2
        t2 = std::move(t1);
        EXPECT_FALSE(t1.joinable());
        EXPECT_TRUE(t2.joinable());
        EXPECT_TRUE(flag2); // t2's original thread was joined during move
        t2.join();
        EXPECT_TRUE(flag1);
    }
}

TEST_F(AutoJoinThreadTest, Swap_ExchangesContents)
{
    {
        std::atomic<bool> flag1{false};
        std::atomic<bool> flag2{false};

        AutoJoinThread t1([&flag1] { flag1 = true; });
        AutoJoinThread t2([&flag2] { flag2 = true; });

        t1.swap(t2);

        t1.join();
        EXPECT_TRUE(flag2);
        t2.join();
        EXPECT_TRUE(flag1);
    }
}

TEST_F(AutoJoinThreadTest, Destructor_AutoJoins)
{
    std::atomic<bool> flag{false};
    {
        AutoJoinThread t([&flag]
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            flag = true;
        });
    }
    EXPECT_TRUE(flag);
}

TEST_F(AutoJoinThreadTest, Joinable_ReturnsCorrectState)
{
    AutoJoinThread t([] {});
    EXPECT_TRUE(t.joinable());
    t.join();
    EXPECT_FALSE(t.joinable());

    // Also check moved-from is non-joinable
    AutoJoinThread t2([] {});
    AutoJoinThread moved(std::move(t2));
    EXPECT_FALSE(t2.joinable());
    moved.join();
}

TEST_F(AutoJoinThreadTest, NativeHandle_ValidAfterConstruction)
{
    AutoJoinThread t([] {});
    EXPECT_NE(t.native_handle(), std::thread::native_handle_type());
    t.join();
}

TEST_F(AutoJoinThreadTest, ConstructWithArgs)
{
    AutoJoinThread t([](int a, int b) { EXPECT_EQ(a + b, 5); }, 2, 3);
    t.join();
}

TEST_F(AutoJoinThreadTest, DetachMovedFrom_NoOp)
{
    AutoJoinThread t([] {});
    AutoJoinThread moved(std::move(t));
    EXPECT_FALSE(t.joinable());
    EXPECT_NO_THROW(t.detach());
    EXPECT_FALSE(t.joinable());
    moved.join();
}

TEST_F(AutoJoinThreadTest, MultipleMoveAssignments)
{
    AutoJoinThread t1([] {});
    AutoJoinThread t2([] { std::this_thread::sleep_for(std::chrono::milliseconds(5)); });
    AutoJoinThread t3([] { std::this_thread::sleep_for(std::chrono::milliseconds(5)); });
    t1 = std::move(t2);
    t1 = std::move(t3);
    EXPECT_TRUE(t1.joinable());
    EXPECT_FALSE(t3.joinable());
    t1.join();
}
