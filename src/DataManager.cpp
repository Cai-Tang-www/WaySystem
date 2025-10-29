#include "../include/DataManager.h"
#include "../include/Utils.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <limits>
#include <iomanip>


using namespace std;

// 构造函数：尝试加载数据
DataManager::DataManager(const string& filePath, MetroData& data)
    : filePath_(filePath), data_(data) {
    loadData();
    // 确保数据加载后，通知用户进行下一步操作
    if (!data_.lines.empty()) {
        Utils::printMessage("数据加载完成。已加载 " + to_string(data_.lines.size()) + " 条线路。");
    } else {
        Utils::printError("未找到数据文件或文件内容为空，请检查 'data/metro_data.txt' 文件。");
    }
}

// 析构函数：在程序结束时自动保存数据
DataManager::~DataManager() {
    saveData();
}

// 从文件读取数据到内存
void DataManager::loadData() {
    ifstream file(filePath_);
    if (!file.is_open()) {
        Utils::printError("无法打开数据文件: " + filePath_ + "。将以空数据启动。");
        return;
    }

    string lineStr;
    int lineCount = 0;
    while (getline(file, lineStr)) {
        lineCount++;
        if (lineStr.empty()) continue;

        vector<string> parts = Utils::split(lineStr, ';');
        
        // 数据格式应包含至少 5 部分：名称;票价;首班;末班;站点列表
        if (parts.size() < 5) {
            Utils::printError("数据格式错误 (缺少字段) - 第 " + to_string(lineCount) + " 行: " + lineStr);
            continue;
        }

        Line line;
        line.name = Utils::trim(parts[0]);
        line.firstTrain = Utils::trim(parts[2]);
        line.lastTrain = Utils::trim(parts[3]);
        
        // 尝试解析全程票价，并捕获 stod 异常
        try {
            // parts[1] 是票价字符串
            line.fullPrice = stod(Utils::trim(parts[1])); 
        } catch (const std::invalid_argument& e) {
            // [修复]：捕获 stod 错误并给出提示，而不是崩溃
            Utils::printError("数据格式错误 (票价非数字) - 第 " + to_string(lineCount) + " 行: '" + parts[1] + "'。此线路将被忽略。");
            continue; // 跳过此条错误数据
        } catch (const std::out_of_range& e) {
            Utils::printError("数据格式错误 (票价数字溢出) - 第 " + to_string(lineCount) + " 行。此线路将被忽略。");
            continue;
        }
        
        // 站点列表在 parts[4] 及之后
        string stationsStr = "";
        for (size_t i = 4; i < parts.size(); ++i) {
            stationsStr += parts[i];
            if (i < parts.size() - 1) stationsStr += ',';
        }
        line.stations = Utils::split(stationsStr, ',');

        if (line.stations.size() < 2) {
            Utils::printError("数据格式错误 (站点少于2个) - 第 " + to_string(lineCount) + " 行。此线路将被忽略。");
            continue;
        }

        data_.lines[line.name] = line;
    }
    file.close();
}

// 将内存数据保存到文件
void DataManager::saveData() const {
    ofstream file(filePath_);
    if (!file.is_open()) {
        Utils::printError("无法打开数据文件进行保存: " + filePath_);
        return;
    }
    
    for (const auto& pair : data_.lines) {
        const Line& line = pair.second;
        file << line.name << ";"
             << std::fixed << std::setprecision(1) << line.fullPrice << ";"
             << line.firstTrain << ";"
             << line.lastTrain << ";";
        
        for (size_t i = 0; i < line.stations.size(); ++i) {
            file << line.stations[i];
            if (i < line.stations.size() - 1) {
                file << ",";
            }
        }
        file << "\n";
    }

    Utils::printMessage("数据已成功保存至文件: " + filePath_);
    file.close();
}