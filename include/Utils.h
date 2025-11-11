#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <iostream>
#include "Structures.h"
// 辅助工具函数集合
namespace Utils {
    // 字符串修剪（去除首尾空白）
    std::string trim(const std::string& str);
    
    // 字符串分割
    std::vector<std::string> split(const std::string& str, char delimiter);

    // 打印程序运行时的提示信息
    void printMessage(const std::string& msg);
    
    // 打印错误信息
    void printError(const std::string& msg);

    // 格式化打印路线方案
    void printRoute(const Route& route, int index);
}

#endif // UTILS_H
