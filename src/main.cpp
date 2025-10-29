#include <iostream>
#include <limits>
#include <memory>
#include "../include/Structures.h"
#include "../include/DataManager.h"
#include "../include/MetroGraph.h"
#include "../include/Utils.h"



using namespace std;

// 显示主菜单
void displayMenu() {
    cout << "\n\n================ 城市地铁查询系统 ===================" << endl;
    cout << "  1. 线路查询 (票价/时间/站点)" << endl;
    cout << "  2. 站点信息查询 (所在线路/相邻站点)" << endl;
    cout << "  3. 乘车查询 (最短路径及所有有效方案)" << endl;
    cout << "  4. 线路增加" << endl;
    cout << "  5. 线路信息维护/更新" << endl;
    cout << "  6. 显示所有线路/站点总览" << endl;
    cout << "  0. 退出系统 (自动保存数据)" << endl;
    cout << "=====================================================" << endl;
    cout << "请选择功能 (0-6): ";
}

// 清除输入流中的错误状态和多余字符
void clearInput() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

// 4. 线路增加功能
void handleAddLine(MetroGraph& graph) {
    string name, fullPriceStr, firstTrain, lastTrain, stationsStr;
    
    cout << "\n--- 增加新地铁线路 ---" << endl;
    cout << "请输入线路名称 (如: 12号线): ";
    cin >> name; clearInput();

    cout << "请输入全程票价 (例如: 8.5): ";
    getline(cin, fullPriceStr);

    cout << "请输入首班时间 (例如: 06:15): ";
    getline(cin, firstTrain);

    cout << "请输入末班时间 (例如: 23:45): ";
    getline(cin, lastTrain);

    cout << "请输入途经所有站点，以逗号分隔 (例如: 站点A,站点B,站点C): ";
    getline(cin, stationsStr);

    try {
        Line newLine;
        newLine.name = name;
        newLine.fullPrice = stod(fullPriceStr);
        newLine.firstTrain = firstTrain;
        newLine.lastTrain = lastTrain;
        newLine.stations = Utils::split(stationsStr, ',');

        graph.addLine(newLine);

    } catch (const std::exception& e) {
        Utils::printError("输入格式错误，请检查票价和站点格式: " + string(e.what()));
    }
}

// 5. 线路信息维护/更新功能
void handleUpdateLine(MetroGraph& graph) {
    string oldName;
    cout << "\n--- 线路信息维护/更新 ---" << endl;
    cout << "请输入要更新的线路名称: ";
    cin >> oldName; clearInput();

    // [修复 1]：使用公共访问器 getData() 访问底层数据 data_
    // 原代码 (报错): if (graph.data_.lines.find(oldName) == graph.data_.lines.end()) {
    if (graph.getData().lines.find(oldName) == graph.getData().lines.end()) {
        Utils::printError("未找到线路: " + oldName);
        return;
    }

    string newName = oldName;
    string fullPriceStr, firstTrain, lastTrain, stationsStr;
    
    // 允许修改线路名称
    cout << "请输入新的线路名称 (留空则保持原名 '" << oldName << "'): ";
    getline(cin, newName);
    if (Utils::trim(newName).empty()) newName = oldName;

    cout << "请输入全程票价 (例如: 8.5): ";
    getline(cin, fullPriceStr);

    cout << "请输入首班时间 (例如: 06:15): ";
    getline(cin, firstTrain);

    cout << "请输入末班时间 (例如: 23:45): ";
    getline(cin, lastTrain);

    cout << "请输入途经所有站点，以逗号分隔 (例如: 站点A,站点B,站点C): ";
    getline(cin, stationsStr);

    try {
        Line newLine;
        newLine.name = newName;
        newLine.fullPrice = stod(fullPriceStr);
        newLine.firstTrain = firstTrain;
        newLine.lastTrain = lastTrain;
        newLine.stations = Utils::split(stationsStr, ',');

        graph.updateLine(oldName, newLine);

    } catch (const std::exception& e) {
        Utils::printError("输入格式错误，请检查票价和站点格式: " + string(e.what()));
    }
}

int main() {
    // 数据容器
    MetroData metroData;
    // 文件路径
    const std::string DATA_FILE = "data/metro_data.txt";

    auto dataManager = std::make_unique<DataManager>(DATA_FILE, metroData);

    // 2. 初始化地铁图结构
    MetroGraph metroGraph(metroData);

    int choice;
    do {
        displayMenu();
        if (!(cin >> choice)) {
            clearInput();
            choice = -1; // 无效输入
        }
        
        string inputStr1, inputStr2;

        switch (choice) {
            case 1: // 线路查询
                cout << "请输入要查询的线路名称 (例如: 1号线): ";
                cin >> inputStr1; clearInput();
                metroGraph.queryLineInfo(inputStr1);
                break;

            case 2: // 站点信息查询
                cout << "请输入要查询的站点名称 (例如: 会展中心): ";
                cin >> inputStr1; clearInput();
                metroGraph.queryStationInfo(inputStr1);
                break;

            case 3: { // 乘车查询
                cout << "请输入起始站点名称: ";
                cin >> inputStr1; clearInput();
                cout << "请输入到达站点名称: ";
                cin >> inputStr2; clearInput();
                
                // 仅查找最短路径
                metroGraph.findRoutes(inputStr1, inputStr2);
                break;
            }

            case 4: // 线路增加
                handleAddLine(metroGraph);
                break;

            case 5: // 线路信息维护/更新
                handleUpdateLine(metroGraph);
                break;
                
            case 6: // 显示所有线路/站点总览
                metroGraph.displayAllInfo();
                break;

            case 0:
                Utils::printMessage("正在退出系统...");
                break;

            default:
                Utils::printError("无效的选择，请重新输入 (0-6)。");
                break;
        }

    } while (choice != 0);

    // dataManager 离开作用域时，析构函数自动调用 saveData()

    return 0;
}