#include <iostream>
#include <optional>
#include <string>

#include "src/data_structure/tree/AVLTree.hpp"
#include "src/data_structure/tree/BinarySearchTree.hpp"

using namespace common::data_structure::tree;

void demonstrateAVLTreeFindValue() {
    std::cout << "=== AVLTree findValue 方法演示 ===" << std::endl;
    
    AVLTree<int> tree;
    tree.insert(50);
    tree.insert(30);
    tree.insert(70);
    tree.insert(20);
    tree.insert(40);
    tree.insert(60);
    tree.insert(80);
    
    // 使用新的 findValue 方法
    auto result1 = tree.findValue(40);
    if (result1.has_value()) {
        std::cout << "找到值: " << result1.value() << std::endl;
    } else {
        std::cout << "未找到值" << std::endl;
    }
    
    auto result2 = tree.findValue(999);
    if (result2.has_value()) {
        std::cout << "找到值: " << result2.value() << std::endl;
    } else {
        std::cout << "未找到值 999" << std::endl;
    }
    
    std::cout << std::endl;
}

void demonstrateBinarySearchTreeFindValue() {
    std::cout << "=== BinarySearchTree findValue 方法演示 ===" << std::endl;
    
    BinarySearchTree<std::string> tree;
    tree.insert("apple");
    tree.insert("banana");
    tree.insert("cherry");
    tree.insert("date");
    tree.insert("elderberry");
    
    // 使用新的 findValue 方法
    auto result1 = tree.findValue("cherry");
    if (result1.has_value()) {
        std::cout << "找到值: " << result1.value() << std::endl;
    } else {
        std::cout << "未找到值" << std::endl;
    }
    
    auto result2 = tree.findValue("fig");
    if (result2.has_value()) {
        std::cout << "找到值: " << result2.value() << std::endl;
    } else {
        std::cout << "未找到值 fig" << std::endl;
    }
    
    std::cout << std::endl;
}

void demonstrateOptionalUsage() {
    std::cout << "=== std::optional 使用示例 ===" << std::endl;
    
    AVLTree<int> tree;
    tree.insert(100);
    tree.insert(200);
    tree.insert(300);
    
    // 可以直接在条件语句中使用
    if (auto result = tree.findValue(200); result.has_value()) {
        std::cout << "值存在: " << result.value() << std::endl;
    }
    
    // 可以使用 value_or 提供默认值
    int value = tree.findValue(999).value_or(-1);
    std::cout << "不存在的值，使用默认值: " << value << std::endl;
    
    // 可以链式调用
    auto doubled = tree.findValue(100).transform([](int v) { return v * 2; });
    if (doubled.has_value()) {
        std::cout << "原始值的两倍: " << doubled.value() << std::endl;
    }
    
    std::cout << std::endl;
}

int main() {
    demonstrateAVLTreeFindValue();
    demonstrateBinarySearchTreeFindValue();
    demonstrateOptionalUsage();
    
    std::cout << "所有演示完成！" << std::endl;
    return 0;
}
