#include "../include/DataManager.h"
#include "../include/Utils.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <limits>
#include <iomanip>
#include <vector>
#include <stdexcept>


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
    ifstreeam file(filePath_);
    if (!file.is_open()) {
        Utils::printError("无法打开数据文件: " + filePath_ + "。将以空数据启动。");
        return;
    }

    string lineStr;
    int lineCount = 0;
    while (getline(file, lineStr)) {
        lineCount++;
        if (lineStr.empty() || Utils::trim(lineStr).empty()) continue;

        vector<string> parts = Utils::split(lineStr, ';');
        if (parts.size() < 5) {
            Utils::printError("数据格式错误 (字段少于5个) - 第 " + to_string(lineCount) + " 行。此线路将被忽略。");
            continue;
        }
        
        Line line;
        line.name = parts[0];
        line.firstTrain = parts[2];
        line.lastTrain = parts[3];

        // 尝试转换票价
        try {
            line.fullPrice = stod(parts[1]);
        } catch (const std::invalid_argument& e) {
            Utils::printError("数据格式错误 (票价非数字) - 第 " + to_string(lineCount) + " 行。此线路将被忽略。");
            continue;
        } catch (const std::out_of_range& e) {
            Utils::printError("数据格式错误 (票价数字溢出) - 第 " + to_string(lineCount) + " 行。此线路将被忽略。");
            continue;
        }
        
        // 站点列表在 parts[4] 及之后
        string stationsStr = "";
        for (size_t i = 4; i < parts.size(); ++i) {
            stationsStr += parts[i];
            if (i < parts.size() - 1) stationsStr += ';'; 
        }
        // 假设站点内部仍以逗号分隔
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
        
        // 站点列表 (以逗号分隔，并作为记录的最后一个字段)
        for (size_t i = 0; i < line.stations.size(); ++i) {
            file << line.stations[i];
            if (i < line.stations.size() - 1) {
                file << ",";
            }
        }
        file << "\n";
    }
    file.close();
    Utils::printMessage("数据已自动保存到文件: " + filePath_);
}