#include <gtest/gtest.h>
#include <unordered_map>
#include <sstream>

namespace {
std::unordered_map<std::string, std::string> g_map;
}

// GTest 事件机制
//全局事件、测试套件、测试用例
class GlobalTestEnv : public testing::Environment {
public:
    void SetUp() override {
        g_map.insert(std::make_pair("key1", "hello"));
        g_map.insert(std::make_pair("key2", "world"));
        std::cout << "GTest 全局事件初始化完成" << std::endl;
    }
    void TearDown() override {
        g_map.clear();
        std::stringstream ss;
        ss << "GTest 全局事件清理完成" << ", map.size() = " << g_map.size() << std::endl;
        std::cout << ss.str() << std::endl;
    }
};

//测试套件、测试用例
class SuiteAndTestEnv : public testing::Test {
public:
    static void SetUpTestSuite() {
        g_map.insert(std::make_pair("key3", "C++"));
        g_map.insert(std::make_pair("key4", "CMake"));
        std::cout << "测试套件初始化完成" << std::endl;
    }
    static void TearDownTestSuite() {
        g_map.erase("key3");
        g_map.erase("key4");
        std::cout << "测试套件清理完成" << std::endl;
    }

protected:
    void SetUp() override {
        g_map.insert(std::make_pair("key5", "gtest"));
        std::cout << "测试用例初始化完成，map.size() = " << g_map.size() << std::endl;
    }
    void TearDown() override {
        g_map.erase("key5");
        std::stringstream ss;
        ss << "测试用例清理完成：" << ", map.size() = " << g_map.size() << std::endl;
        std::cout << ss.str() << std::endl;
    }
};

TEST(GtestTest, TestAsserts) { //测试名称、测试用例名称（同一个测试中，测试用例不能重复），不需要加引号
    int a = 1, b = 2;
    bool flag = false;
    ASSERT_NE(a, b);
    ASSERT_EQ(a + 1, b);
    ASSERT_GT(b, a);
    ASSERT_FALSE(flag);
    ASSERT_EQ(g_map.size(), 2);
}

TEST_F(SuiteAndTestEnv, TestSuite1) { //测试套件名称、测试用例名称
    ASSERT_EQ(g_map.at("key1"), "hello");
    ASSERT_EQ(g_map.at("key2"), "world");
    ASSERT_EQ(g_map.at("key3"), "C++");
    ASSERT_EQ(g_map.at("key4"), "CMake");
    ASSERT_EQ(g_map.at("key5"), "gtest");
}

TEST_F(SuiteAndTestEnv, TestSuite2) {
    ASSERT_EQ(g_map.at("key1"), "hello");
    ASSERT_EQ(g_map.at("key2"), "world");
    ASSERT_EQ(g_map.at("key3"), "C++");
    ASSERT_EQ(g_map.at("key4"), "CMake");
    ASSERT_EQ(g_map.at("key5"), "gtest");
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    testing::AddGlobalTestEnvironment(new GlobalTestEnv); // 将测试全局事件添加到测试
    return RUN_ALL_TESTS();
}
