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

using namespace cppforge::thread;

/// @brief Test fixture for AutoJoinThread tests.
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

/**
 * @brief Verify a moved-from AutoJoinThread is not joinable
 * @details Constructs a thread, move-constructs another from it, then
 *          asserts the source thread is no longer joinable.
 */
TEST_F(AutoJoinThreadTest, MovedFrom_NotJoinable)
{
    AutoJoinThread t([] {});
    EXPECT_TRUE(t.joinable());
    AutoJoinThread moved(std::move(t));
    EXPECT_FALSE(t.joinable());
    moved.join();
}

/**
 * @brief Verify thread becomes joinable after construction with a callable
 * @details Creates an AutoJoinThread with a lambda that sleeps, then
 *          checks joinable() returns true before joining.
 */
TEST_F(AutoJoinThreadTest, ConstructWithCallable_Joinable)
{
    AutoJoinThread t([] { std::this_thread::sleep_for(std::chrono::milliseconds(10)); });
    EXPECT_TRUE(t.joinable());
    t.join();
}

/**
 * @brief Verify join() blocks until the thread completes execution
 * @details Uses an atomic flag set by the thread after a sleep; after
 *          join() returns the flag must be true, confirming the thread
 *          finished before join() unblocked.
 */
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

/**
 * @brief Verify calling join() on an already-joined thread is a no-op
 * @details Joins a thread once, verifies it is no longer joinable, then
 *          joins again to confirm no crash or state change occurs.
 */
TEST_F(AutoJoinThreadTest, Join_AlreadyJoined_NoOp)
{
    AutoJoinThread t([] { std::this_thread::sleep_for(std::chrono::milliseconds(5)); });
    t.join();
    EXPECT_FALSE(t.joinable());
    t.join();
    EXPECT_FALSE(t.joinable());
}

/**
 * @brief Verify detach() releases ownership and thread continues running
 * @details Detaches a thread, then allows the AutoJoinThread to go out of
 *          scope; the detached thread should still complete and set the
 *          flag, proving no auto-join occurred.
 */
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

/**
 * @brief Verify move constructor transfers thread ownership to new instance
 * @details Move-constructs t2 from t1; t1 becomes non-joinable, t2 owns
 *          the thread and can join it, confirming ownership transfer.
 */
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

/**
 * @brief Verify move assignment joins the current thread then transfers
 * @details t2 initially owns a thread; move-assigning t1 to t2 should
 *          join t2's original thread (flag2 becomes true), then transfer
 *          t1's thread to t2.
 */
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

/**
 * @brief Verify swap() exchanges thread contents between two instances
 * @details After swapping, t1's join() waits for t2's original thread and
 *          vice versa, proving the underlying thread handles were exchanged.
 */
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

/**
 * @brief Verify destructor automatically joins the running thread
 * @details Creates a thread inside a nested scope; when the
 *          AutoJoinThread goes out of scope, its destructor joins the
 *          thread, so the flag is set by the time we check it outside.
 */
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

/**
 * @brief Verify joinable() reflects the thread lifecycle state
 * @details Checks that a newly constructed thread is joinable, becomes
 *          non-joinable after join(), and that a moved-from thread is
 *          also non-joinable.
 */
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

/**
 * @brief Verify native_handle() is non-default after thread construction
 * @details Checks that native_handle() returns a valid (non-default)
 *          handle immediately after constructing AutoJoinThread with a
 *          callable.
 */
TEST_F(AutoJoinThreadTest, NativeHandle_ValidAfterConstruction)
{
    AutoJoinThread t([] {});
    EXPECT_NE(t.native_handle(), std::thread::native_handle_type());
    t.join();
}

/**
 * @brief Verify construction with arguments forwarded to the callable
 * @details Passes additional arguments (2, 3) to the callable; the thread
 *          sums them inside and asserts the expected result.
 */
TEST_F(AutoJoinThreadTest, ConstructWithArgs)
{
    AutoJoinThread t([](int a, int b) { EXPECT_EQ(a + b, 5); }, 2, 3);
    t.join();
}

/**
 * @brief Verify detach() on a moved-from thread is a safe no-op
 * @details After move-constructing, calls detach() on the now-empty
 *          source; asserts no exception is thrown and joinable() remains
 *          false.
 */
TEST_F(AutoJoinThreadTest, DetachMovedFrom_NoOp)
{
    AutoJoinThread t([] {});
    AutoJoinThread moved(std::move(t));
    EXPECT_FALSE(t.joinable());
    EXPECT_NO_THROW(t.detach());
    EXPECT_FALSE(t.joinable());
    moved.join();
}

/**
 * @brief Verify multiple move assignments correctly manage ownership
 * @details Moves t2 into t1, then t3 into t1; t1 ends up owning t3's
 *          thread while t3 becomes non-joinable, and t1 remains joinable.
 */
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
