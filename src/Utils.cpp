#include "../include/Utils.h"
#include <algorithm>
#include <iomanip>
#include "../include/Structures.h"
#include <sstream>
#include <iostream>

namespace Utils {
    // 字符串修剪（去除首尾空白）
    std::string trim(const std::string& str) {
        const auto strBegin = str.find_first_not_of(" \t\n\r");
        if (strBegin == std::string::npos) return ""; 
        const auto strEnd = str.find_last_not_of(" \t\n\r");
        const auto strRange = strEnd - strBegin + 1;
        return str.substr(strBegin, strRange);
    }

    // 字符串分割
    std::vector<std::string> split(const std::string& str, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(str);
        while (std::getline(tokenStream, token, delimiter)) {
            tokens.push_back(trim(token));
        }
        return tokens;
    }

    // 打印程序运行时的提示信息
    void printMessage(const std::string& msg) {
        std::cout << "\n[INFO] " << msg << std::endl;
    }
    
    // 打印错误信息
    void printError(const std::string& msg) {
        std::cerr << "\n[ERROR] " << msg << std::endl;
    }

    // 格式化打印路线方案 (更新为包含价格和新的换乘信息)
    void printRoute(const Route& route, int index) {
        std::cout << "\n==============================================" << std::endl;
        std::cout << "方案 " << index << "：" << route.description << std::endl;
        // 打印总耗时和总价格
        std::cout << "总耗时：" << std::fixed << std::setprecision(1) << route.totalCost << " 分钟 | ";
        std::cout << "总价格：" << std::fixed << std::setprecision(1) << route.totalPrice << " 元" << std::endl;
        std::cout << "----------------------------------------------" << std::endl;
        
        for (size_t i = 0; i < route.segments.size(); ++i) {
            const auto& segment = route.segments[i];
            
            // 打印换乘提示（仅当线路切换时）
            if (i > 0) {
                const auto& prevSegment = route.segments[i-1];
                if (prevSegment.lineName != segment.lineName && segment.lineName != "TRANSFER") {
                    std::cout << "  (换乘提示) 在 [" << prevSegment.endStation 
                              << "] 换乘至 [" << segment.lineName << "]" << std::endl;
                }
            }

            // 打印段信息
            if (segment.lineName == "TRANSFER") {
                // 打印换乘段
                 std::cout << "  [换乘] 在 [" << segment.startStation << "] 完成跨线换乘。"
                           << " (耗时: " << std::fixed << std::setprecision(1) << segment.cost
                           << " 分钟, 价格: " << std::fixed << std::setprecision(1) << segment.price << " 元)" << std::endl;
            } else {
                // 打印乘车段
                std::cout << "  [" << segment.lineName << "] 从 [" << segment.startStation 
                          << "] 乘车到 [" << segment.endStation << "]"
                          << " (耗时: " << std::fixed << std::setprecision(1) << segment.cost 
                          << " 分钟, 价格: " << std::fixed << std::setprecision(1) << segment.price << " 元)" << std::endl;
            }
        }
    }
}