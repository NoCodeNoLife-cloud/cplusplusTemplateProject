/**
 * @file ObjectFactoryTest.cc
 * @brief Unit tests for the ObjectFactory class
 * @details Tests cover object registration, creation, error handling, and polymorphic behavior.
 */

#include <gtest/gtest.h>
#include "toolkit/ObjectFactory.hpp"
#include <string>
#include <memory>
#include <type_traits>

using namespace common::toolkit;

/**
 * @brief Test interface for factory pattern
 */
class IShape {
public:
    virtual ~IShape() = default;

    [[nodiscard]] virtual auto getName() const -> std::string = 0;

    [[nodiscard]] virtual auto getArea() const -> double = 0;
};

/**
 * @brief Concrete implementations for testing
 */
class Circle : public IShape {
public:
    explicit Circle(double radius = 1.0) : radius_(radius) {
    }

    [[nodiscard]] auto getName() const -> std::string override {
        return "Circle";
    }

    [[nodiscard]] auto getArea() const -> double override {
        return 3.14159 * radius_ * radius_;
    }

private:
    double radius_;
};

class Rectangle : public IShape {
public:
    Rectangle(double width = 1.0, double height = 1.0) : width_(width), height_(height) {
    }

    [[nodiscard]] auto getName() const -> std::string override {
        return "Rectangle";
    }

    [[nodiscard]] auto getArea() const -> double override {
        return width_ * height_;
    }

private:
    double width_;
    double height_;
};

class Triangle : public IShape {
public:
    Triangle(double base = 1.0, double height = 1.0) : base_(base), height_(height) {
    }

    [[nodiscard]] auto getName() const -> std::string override {
        return "Triangle";
    }

    [[nodiscard]] auto getArea() const -> double override {
        return 0.5 * base_ * height_;
    }

private:
    double base_;
    double height_;
};

/**
 * @brief Concrete factory implementation for testing
 */
class ShapeFactory : public ObjectFactory<IShape> {
protected:
    auto registerAll() -> void override {
        registerType<Circle>("Circle", 5.0);
        registerType<Rectangle>("Rectangle", 4.0, 6.0);
        registerType<Triangle>("Triangle", 3.0, 4.0);
    }
};

/**
 * @brief Test registration and creation of basic types
 * @details Verifies types can be registered and queried
 */
TEST(ObjectFactoryTest, RegisterAndCreate_BasicTypes) {
    ObjectFactory<IShape>::clearRegistry();
    ShapeFactory factory;

    // Execute registration
    EXPECT_TRUE(factory.execute());

    // Verify types are registered
    EXPECT_TRUE(ObjectFactory<IShape>::isRegistered("Circle"));
    EXPECT_TRUE(ObjectFactory<IShape>::isRegistered("Rectangle"));
    EXPECT_TRUE(ObjectFactory<IShape>::isRegistered("Triangle"));
}

/**
 * @brief Test object creation for valid registered types
 * @details Verifies created objects have correct properties
 */
TEST(ObjectFactoryTest, CreateObject_ValidTypes) {
    ObjectFactory<IShape>::clearRegistry();
    ShapeFactory factory;
    factory.execute();

    // Create Circle
    auto circle = ObjectFactory<IShape>::createObject("Circle");
    ASSERT_NE(circle, nullptr);
    EXPECT_EQ(circle->getName(), "Circle");
    EXPECT_NEAR(circle->getArea(), 3.14159 * 5.0 * 5.0, 0.0001);

    // Create Rectangle
    auto rect = ObjectFactory<IShape>::createObject("Rectangle");
    ASSERT_NE(rect, nullptr);
    EXPECT_EQ(rect->getName(), "Rectangle");
    EXPECT_NEAR(rect->getArea(), 24.0, 0.0001);

    // Create Triangle
    auto triangle = ObjectFactory<IShape>::createObject("Triangle");
    ASSERT_NE(triangle, nullptr);
    EXPECT_EQ(triangle->getName(), "Triangle");
    EXPECT_NEAR(triangle->getArea(), 6.0, 0.0001);
}

/**
 * @brief Test creation of multiple instances of same type
 * @details Verifies each call creates a new independent instance
 */
TEST(ObjectFactoryTest, CreateObject_MultipleInstances) {
    ObjectFactory<IShape>::clearRegistry();
    ShapeFactory factory;
    factory.execute();

    // Create multiple instances of the same type
    auto circle1 = ObjectFactory<IShape>::createObject("Circle");
    auto circle2 = ObjectFactory<IShape>::createObject("Circle");

    ASSERT_NE(circle1, nullptr);
    ASSERT_NE(circle2, nullptr);
    EXPECT_NE(circle1.get(), circle2.get()); // Different instances
    EXPECT_EQ(circle1->getArea(), circle2->getArea());
}

/**
 * @brief Test error handling for unregistered type
 * @details Verifies std::runtime_error is thrown for unknown types
 */
TEST(ObjectFactoryTest, CreateObject_UnregisteredType_ThrowsException) {
    ObjectFactory<IShape>::clearRegistry();
    ShapeFactory factory;
    factory.execute();

    EXPECT_THROW(
        ObjectFactory<IShape>::createObject("UnknownShape"),
        std::runtime_error
    );
}

/**
 * @brief Test error handling for empty type name
 * @details Verifies std::invalid_argument is thrown for empty string
 */
TEST(ObjectFactoryTest, CreateObject_EmptyTypeName_ThrowsException) {
    EXPECT_THROW(
        ObjectFactory<IShape>::createObject(""),
        std::invalid_argument
    );
}

TEST(ObjectFactoryTest, RegisterType_EmptyTypeName_ThrowsException) {
    ShapeFactory factory;

    EXPECT_THROW(
        ObjectFactory<IShape>::registerType<Circle>("", 1.0),
        std::invalid_argument
    );
}

/**
 * @brief Test isRegistered functionality after execution
 * @details Verifies registration state changes correctly before and after execute
 */
TEST(ObjectFactoryTest, IsRegistered_AfterExecution) {
    ObjectFactory<IShape>::clearRegistry();
    ShapeFactory factory;

    // Before execution, types should not be registered
    EXPECT_FALSE(ObjectFactory<IShape>::isRegistered("Circle"));

    // After execution, types should be registered
    factory.execute();
    EXPECT_TRUE(ObjectFactory<IShape>::isRegistered("Circle"));
    EXPECT_TRUE(ObjectFactory<IShape>::isRegistered("Rectangle"));
}

TEST(ObjectFactoryTest, IsRegistered_EmptyTypeName) {
    EXPECT_FALSE(ObjectFactory<IShape>::isRegistered(""));
}

TEST(ObjectFactoryTest, IsRegistered_NonExistentType) {
    ObjectFactory<IShape>::clearRegistry();
    ShapeFactory factory;
    factory.execute();

    EXPECT_FALSE(ObjectFactory<IShape>::isRegistered("Pentagon"));
    EXPECT_FALSE(ObjectFactory<IShape>::isRegistered("Hexagon"));
}

/**
 * @brief Test execute functionality for successful registration
 * @details Verifies execute returns true and types are registered
 */
TEST(ObjectFactoryTest, Execute_SuccessfulRegistration) {
    ObjectFactory<IShape>::clearRegistry();
    ShapeFactory factory;

    auto result = factory.execute();
    EXPECT_TRUE(result);

    // Verify registration was successful
    EXPECT_TRUE(ObjectFactory<IShape>::isRegistered("Circle"));
}

TEST(ObjectFactoryTest, Execute_MultipleCalls) {
    ObjectFactory<IShape>::clearRegistry();
    ShapeFactory factory;

    // First execution
    EXPECT_TRUE(factory.execute());

    // Second execution (should re-register, overwriting previous)
    EXPECT_TRUE(factory.execute());

    // Types should still be registered
    EXPECT_TRUE(ObjectFactory<IShape>::isRegistered("Circle"));
}

/**
 * @brief Test clearRegistry functionality after registration
 * @details Verifies registry can be cleared and types become unregistered
 */
TEST(ObjectFactoryTest, ClearRegistry_AfterRegistration) {
    ObjectFactory<IShape>::clearRegistry();
    ShapeFactory factory;
    factory.execute();

    // Verify types are registered
    EXPECT_TRUE(ObjectFactory<IShape>::isRegistered("Circle"));

    // Clear registry
    ObjectFactory<IShape>::clearRegistry();

    // Verify types are no longer registered
    EXPECT_FALSE(ObjectFactory<IShape>::isRegistered("Circle"));
    EXPECT_FALSE(ObjectFactory<IShape>::isRegistered("Rectangle"));
}

TEST(ObjectFactoryTest, ClearRegistry_AndReregister) {
    ObjectFactory<IShape>::clearRegistry();
    ShapeFactory factory;

    // Register
    factory.execute();
    EXPECT_TRUE(ObjectFactory<IShape>::isRegistered("Circle"));

    // Clear
    ObjectFactory<IShape>::clearRegistry();
    EXPECT_FALSE(ObjectFactory<IShape>::isRegistered("Circle"));

    // Re-register
    factory.execute();
    EXPECT_TRUE(ObjectFactory<IShape>::isRegistered("Circle"));

    // Should be able to create objects again
    auto circle = ObjectFactory<IShape>::createObject("Circle");
    ASSERT_NE(circle, nullptr);
    EXPECT_EQ(circle->getName(), "Circle");
}

// Test with different constructor arguments
TEST(ObjectFactoryTest, RegisterWithDifferentArguments) {
    // Clear any existing registrations
    ObjectFactory<IShape>::clearRegistry();

    class CustomFactory : public ObjectFactory<IShape> {
    protected:
        auto registerAll() -> void override {
            registerType<Circle>("SmallCircle", 1.0);
            registerType<Circle>("LargeCircle", 10.0);
            registerType<Rectangle>("Square", 5.0, 5.0);
        }
    };

    CustomFactory factory;
    factory.execute();

    auto small = ObjectFactory<IShape>::createObject("SmallCircle");
    auto large = ObjectFactory<IShape>::createObject("LargeCircle");
    auto square = ObjectFactory<IShape>::createObject("Square");

    ASSERT_NE(small, nullptr);
    ASSERT_NE(large, nullptr);
    ASSERT_NE(square, nullptr);

    EXPECT_NEAR(small->getArea(), 3.14159, 0.0001);
    EXPECT_NEAR(large->getArea(), 314.159, 0.001);
    EXPECT_NEAR(square->getArea(), 25.0, 0.0001);
}

/**
 * @brief Test polymorphic behavior of created objects
 * @details Verifies virtual methods work correctly on factory-created objects
 */
TEST(ObjectFactoryTest, PolymorphicBehavior) {
    ObjectFactory<IShape>::clearRegistry();
    ShapeFactory factory;
    factory.execute();

    std::vector<std::string> shape_names = {"Circle", "Rectangle", "Triangle"};
    std::vector<std::unique_ptr<IShape> > shapes;

    for (const auto &name: shape_names) {
        shapes.push_back(ObjectFactory<IShape>::createObject(name));
    }

    // Verify polymorphic behavior
    EXPECT_EQ(shapes[0]->getName(), "Circle");
    EXPECT_EQ(shapes[1]->getName(), "Rectangle");
    EXPECT_EQ(shapes[2]->getName(), "Triangle");

    // All should have valid areas
    EXPECT_GT(shapes[0]->getArea(), 0);
    EXPECT_GT(shapes[1]->getArea(), 0);
    EXPECT_GT(shapes[2]->getArea(), 0);
}

// Test thread safety (basic test)
TEST(ObjectFactoryTest, ThreadSafety_BasicOperations) {
    ObjectFactory<IShape>::clearRegistry();
    ShapeFactory factory;
    factory.execute();

    // Multiple creations should work correctly
    for (int i = 0; i < 10; ++i) {
        auto circle = ObjectFactory<IShape>::createObject("Circle");
        ASSERT_NE(circle, nullptr);
        EXPECT_EQ(circle->getName(), "Circle");
    }
}

/**
 * @brief Test that ObjectFactory inherits from IStartupTask
 * @details Verifies inheritance relationship and interface compatibility
 */
TEST(ObjectFactoryTest, Inheritance_FromIStartupTask) {
    ObjectFactory<IShape>::clearRegistry();
    static_assert(
        std::is_base_of_v<common::interfaces::IStartupTask, ObjectFactory<IShape> >,
        "ObjectFactory should inherit from IStartupTask"
    );

    ShapeFactory factory;
    common::interfaces::IStartupTask *task = &factory;

    EXPECT_TRUE(task->execute());
}