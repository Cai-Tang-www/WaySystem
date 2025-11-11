#include <iostream>
#include <limits>
#include <memory>
#include <string>
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
    cout << "  3. 乘车查询 (最短路径及所有有效方案) 【支持时间和价格】" << endl; 
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

// 4. 线路增加功能 (完整实现)
void handleAddLine(MetroGraph& graph) {
    string name, fullPriceStr, firstTrain, lastTrain, stationsStr;
    Line newLine;

    cout << "请输入新线路名称 (例如: 3号线): ";
    getline(cin, name);
    newLine.name = Utils::trim(name);

    cout << "请输入全程票价 (例如: 5.0): ";
    getline(cin, fullPriceStr);
    try {
        newLine.fullPrice = stod(Utils::trim(fullPriceStr));
    } catch (const exception& e) {
        Utils::printError("票价输入无效。线路添加失败。");
        return;
    }

    cout << "请输入首班车时间 (例如: 06:00): ";
    getline(cin, firstTrain);
    newLine.firstTrain = Utils::trim(firstTrain);

    cout << "请输入末班车时间 (例如: 23:30): ";
    getline(cin, lastTrain);
    newLine.lastTrain = Utils::trim(lastTrain);

    cout << "请输入所有途径站点，用逗号分隔 (例如: 体育西,珠江新城,广州塔): ";
    getline(cin, stationsStr);
    newLine.stations = Utils::split(stationsStr, ',');

    graph.addLine(newLine);
}

// 5. 线路更新功能 (完整实现)
void handleUpdateLine(MetroGraph& graph) {
    string oldName, name, fullPriceStr, firstTrain, lastTrain, stationsStr;
    Line newLine;

    cout << "请输入要维护的旧线路名称 (例如: 1号线): ";
    getline(cin, oldName);
    string trimmedOldName = Utils::trim(oldName);

    cout << "请输入新线路名称 (可与旧名称相同): ";
    getline(cin, name);
    newLine.name = Utils::trim(name);

    cout << "请输入全程票价 (例如: 5.0): ";
    getline(cin, fullPriceStr);
    try {
        newLine.fullPrice = stod(Utils::trim(fullPriceStr));
    } catch (const exception& e) {
        Utils::printError("票价输入无效。线路更新失败。");
        return;
    }

    cout << "请输入首班车时间 (例如: 06:00): ";
    getline(cin, firstTrain);
    newLine.firstTrain = Utils::trim(firstTrain);

    cout << "请输入末班车时间 (例如: 23:30): ";
    getline(cin, lastTrain);
    newLine.lastTrain = Utils::trim(lastTrain);

    cout << "请输入所有途径站点，用逗号分隔 (例如: 体育西,珠江新城,广州塔): ";
    getline(cin, stationsStr);
    newLine.stations = Utils::split(stationsStr, ',');

    graph.updateLine(trimmedOldName, newLine);
}

int main() {
    // 定义数据文件路径
    string dataFilePath = "data/metro_data.txt";

    // 初始化数据管理器和图
    MetroData metroData;
    DataManager dataManager(dataFilePath, metroData);
    MetroGraph metroGraph(metroData); 
    
    int choice = -1;
    while (choice != 0) {
        displayMenu();
        if (!(cin >> choice)) {
            clearInput();
            Utils::printError("输入无效，请重新选择。");
            continue;
        } clearInput();
        
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

            case 3: { // 乘车查询 (新增查询维度选择)
                cout << "请输入起始站点名称: ";
                getline(cin, inputStr1);
                inputStr1 = Utils::trim(inputStr1);

                cout << "请输入到达站点名称: ";
                getline(cin, inputStr2);
                inputStr2 = Utils::trim(inputStr2);
                
                int queryType = 0;
                cout << "请选择查询维度 (1: 最短时间, 2: 最短价格): ";
                if (!(cin >> queryType) || (queryType != 1 && queryType != 2)) {
                    clearInput();
                    Utils::printError("选择无效，默认为最短时间。");
                    queryType = 1;
                } clearInput();
                
                bool usePrice = (queryType == 2);
                
                metroGraph.findRoutes(inputStr1, inputStr2, usePrice);
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
                Utils::printError("无效选择，请重新输入 0-6。");
                break;
        }
    }

    return 0; 
}