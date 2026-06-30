#include <gtest/gtest.h>
#include <cppforge/di/ServiceCollection.hpp>
#include <cppforge/di/ServiceProvider.hpp>
#include <cppforge/di/ApplicationContext.hpp>

using namespace cppforge::di;

// Test interfaces and implementations
class ITestService
{
public:
    virtual ~ITestService() = default;
    virtual int getValue() const = 0;
};

class TestServiceA : public ITestService
{
public:
    int getValue() const override { return 42; }
};

class TestServiceB : public ITestService
{
public:
    int getValue() const override { return 100; }
};

class AnotherService
{
public:
    std::string getName() const { return "AnotherService"; }
};

// Test ServiceCollection
TEST(DITest, ServiceCollection_AddSingleton)
{
    ServiceCollection collection;
    collection.addSingleton<ITestService, TestServiceA>();
    auto provider = collection.buildServiceProvider();
    
    EXPECT_TRUE(provider->isRegistered(typeid(ITestService)));
}

TEST(DITest, ServiceCollection_AddTransient)
{
    ServiceCollection collection;
    collection.addTransient<ITestService, TestServiceA>();
    auto provider = collection.buildServiceProvider();
    
    EXPECT_TRUE(provider->isRegistered(typeid(ITestService)));
}

// Test ServiceProvider - Singleton
TEST(DITest, ServiceProvider_Singleton_ReturnsSameInstance)
{
    ServiceCollection collection;
    collection.addSingleton<ITestService, TestServiceA>();
    auto provider = collection.buildServiceProvider();
    
    auto service1 = provider->getService<ITestService>();
    auto service2 = provider->getService<ITestService>();
    
    EXPECT_EQ(service1.get(), service2.get());
    EXPECT_EQ(service1->getValue(), 42);
}

// Test ServiceProvider - Transient
TEST(DITest, ServiceProvider_Transient_ReturnsDifferentInstances)
{
    ServiceCollection collection;
    collection.addTransient<ITestService, TestServiceA>();
    auto provider = collection.buildServiceProvider();
    
    auto service1 = provider->getService<ITestService>();
    auto service2 = provider->getService<ITestService>();
    
    EXPECT_NE(service1.get(), service2.get());
}

// Test ServiceProvider - Multiple services
TEST(DITest, ServiceProvider_MultipleServices)
{
    ServiceCollection collection;
    collection.addSingleton<ITestService, TestServiceA>();
    collection.addSingleton<AnotherService, AnotherService>();
    auto provider = collection.buildServiceProvider();
    
    auto testService = provider->getService<ITestService>();
    auto anotherService = provider->getService<AnotherService>();
    
    EXPECT_EQ(testService->getValue(), 42);
    EXPECT_EQ(anotherService->getName(), "AnotherService");
}

// Test ServiceProvider - Not registered
TEST(DITest, ServiceProvider_NotRegistered_Throws)
{
    ServiceCollection collection;
    auto provider = collection.buildServiceProvider();
    
    EXPECT_THROW(provider->getService<ITestService>(), std::runtime_error);
}

// Test ServiceProvider - isRegistered
TEST(DITest, ServiceProvider_IsRegistered)
{
    ServiceCollection collection;
    collection.addSingleton<ITestService, TestServiceA>();
    auto provider = collection.buildServiceProvider();
    
    EXPECT_TRUE(provider->isRegistered(typeid(ITestService)));
    EXPECT_FALSE(provider->isRegistered(typeid(AnotherService)));
}

// Test ApplicationContext
TEST(DITest, ApplicationContext_RunAndShutdown)
{
    ServiceCollection collection;
    collection.addSingleton<ITestService, TestServiceA>();
    auto provider = collection.buildServiceProvider();
    
    ApplicationContext context(provider);
    
    bool startupCalled = false;
    bool shutdownCalled = false;
    
    context.onStartup([&startupCalled]() { startupCalled = true; });
    context.onShutdown([&shutdownCalled]() { shutdownCalled = true; });
    
    context.run();
    EXPECT_TRUE(startupCalled);
    
    context.shutdown();
    EXPECT_TRUE(shutdownCalled);
}

TEST(DITest, ApplicationContext_GetService)
{
    ServiceCollection collection;
    collection.addSingleton<ITestService, TestServiceA>();
    auto provider = collection.buildServiceProvider();
    
    ApplicationContext context(provider);
    auto service = context.getService<ITestService>();
    
    EXPECT_EQ(service->getValue(), 42);
}

// Test with factory function
TEST(DITest, ServiceCollection_WithFactory)
{
    ServiceCollection collection;
    collection.addSingleton<ITestService>([]() {
        return std::make_shared<TestServiceB>();
    });
    auto provider = collection.buildServiceProvider();
    
    auto service = provider->getService<ITestService>();
    EXPECT_EQ(service->getValue(), 100);
}