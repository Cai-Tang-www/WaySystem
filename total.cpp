// #include <iostream>
// #include <fstream>
// #include <sstream>
// #include <string>
// #include <vector>
// #include <map>
// #include <limits>
// #include <algorithm>
// #include <queue>
// #include <iomanip>
// #include <cmath>
// #include <set>
// #include <stdexcept>
#include <bits/stdc++.h>
// ========== 结构体和常量 ==========

// 定义无穷大，用于最短路径算法
const double INF = std::numeric_limits<double>::infinity();
// 定义换乘时间（1.0 分钟）
const double TRANSFER_TIME = 1.0; 

// 站点信息结构体
struct Station {
    int id;               // 站点唯一ID (用于图的索引)
    std::string name;     // 站点名称
    std::string lineName; // 站点所属线路名称 (例如: 1号线)
};

// 线路信息结构体
struct Line {
    std::string name;       // 线路名称 (例如: 1号线)
    double fullPrice;       // 全程票价 (RMB)
    std::string firstTrain; // 首班时间
    std::string lastTrain;  // 末班时间
    std::vector<std::string> stations; // 途经站点名称列表
};

// 图的边结构体
struct Edge {
    int destId;               // 目标站点ID
    double distance;          // 耗时/距离 (分钟)
    double price;             // 价格 (元) 
    std::string lineName;     // 边的所属线路 ("TRANSFER"表示换乘)
};

// 乘车方案结构体，片段实现分段储存
struct RouteSegment {
    std::string startStation;
    std::string endStation;
    std::string lineName;
    double cost; // 花费（时间）
    double price; // 价格 
};

// 完整的乘车方案
struct Route {
    std::vector<RouteSegment> segments;
    double totalCost; // 总耗时
    double totalPrice; // 总价格 
    std::string description;
};

// 全局数据容器
struct MetroData {
    std::map<std::string, Line> lines;        // 线路信息 (按名称索引)
    std::map<std::string, int> stationNameId; // 站点名称 -> ID 映射
    std::map<int, Station> stationIdInfo;     // 站点 ID -> 站点信息
    int nextStationId = 0;                    // 下一个可用的站点 ID
};

// ========== 辅助工具函数声明 ==========

namespace Utils {
    std::string trim(const std::string& str);
    std::vector<std::string> split(const std::string& str, char delimiter);
    void printMessage(const std::string& msg);
    void printError(const std::string& msg);
    void printRoute(const Route& route, int index);
}

// ========== DataManager 类声明 ==========

class DataManager {
public:
    DataManager(const std::string& filePath, MetroData& data);
    ~DataManager();
    void loadData();
    void saveData() const;

private:
    std::string filePath_; 
    MetroData& data_;      

};

// ========== MetroGraph 类声明 ==========

class MetroGraph {
public:
    MetroGraph(MetroData& data);

    void queryLineInfo(const std::string& lineName) const;
    void queryStationInfo(const std::string& stationName) const;
    std::vector<Route> findRoutes(const std::string& start, const std::string& end, bool usePrice) const;
    bool addLine(const Line& newLine);
    bool updateLine(const std::string& oldLineName, const Line& newLine);
    void displayAllInfo() const;

    MetroData& getData() { return data_; }
    const MetroData& getData() const { return data_; }
    
private:
    MetroData& data_;
    std::vector<std::vector<Edge>> adjList; 

    void buildGraph();
    void processLineForGraph(const Line& line);
    std::vector<int> dijkstra(int startId, int endId, std::vector<double>& results, bool usePrice) const;
    Route pathIdToRoute(const std::vector<int>& pathIds, double totalCost, double totalPrice) const;
    void dfsFindRoutes(int currentId, int endId, std::vector<int>& currentPath, 
                       std::vector<bool>& visited, double currentTime, double currentPrice,
                       std::vector<Route>& allRoutes, int maxStops, double maxCost) const;
};


// ========== 辅助工具函数实现 (Utils.cpp) ==========

namespace Utils {
    std::string trim(const std::string& str) {
        const auto strBegin = str.find_first_not_of(" \t\n\r");
        if (strBegin == std::string::npos) return ""; 
        const auto strEnd = str.find_last_not_of(" \t\n\r");
        const auto strRange = strEnd - strBegin + 1;
        return str.substr(strBegin, strRange);
    }

    std::vector<std::string> split(const std::string& str, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(str);
        while (std::getline(tokenStream, token, delimiter)) {
            tokens.push_back(trim(token));
        }
        return tokens;
    }

    void printMessage(const std::string& msg) {
        std::cout << "\n[INFO] " << msg << std::endl;
    }
    
    void printError(const std::string& msg) {
        std::cerr << "\n[ERROR] " << msg << std::endl;
    }

    void printRoute(const Route& route, int index) {
        std::cout << "\n==============================================" << std::endl;
        std::cout << "方案 " << index << "：" << route.description << std::endl;
        std::cout << "总耗时：" << std::fixed << std::setprecision(1) << route.totalCost << " 分钟 | ";
        std::cout << "总价格：" << std::fixed << std::setprecision(1) << route.totalPrice << " 元" << std::endl;
        std::cout << "----------------------------------------------" << std::endl;
        
        for (size_t i = 0; i < route.segments.size(); ++i) {
            const auto& segment = route.segments[i];
            
            if (i > 0) {
                const auto& prevSegment = route.segments[i-1];
                if (prevSegment.lineName != segment.lineName && segment.lineName != "TRANSFER") {
                    std::cout << "  (换乘提示) 在 [" << prevSegment.endStation 
                              << "] 换乘至 [" << segment.lineName << "]" << std::endl;
                }
            }

            if (segment.lineName == "TRANSFER") {
                 std::cout << "  [换乘] 在 [" << segment.startStation << "] 完成跨线换乘。"
                           << " (耗时: " << std::fixed << std::setprecision(1) << segment.cost
                           << " 分钟, 价格: " << std::fixed << std::setprecision(1) << segment.price << " 元)" << std::endl;
            } else {
                std::cout << "  [" << segment.lineName << "] 从 [" << segment.startStation 
                          << "] 乘车到 [" << segment.endStation << "]"
                          << " (耗时: " << std::fixed << std::setprecision(1) << segment.cost 
                          << " 分钟, 价格: " << std::fixed << std::setprecision(1) << segment.price << " 元)" << std::endl;
            }
        }
    }
}


// ========== DataManager 类实现 (DataManager.cpp) ==========

using namespace std;

DataManager::DataManager(const string& filePath, MetroData& data)
    : filePath_(filePath), data_(data) {
    loadData();
    if (!data_.lines.empty()) {
        Utils::printMessage("数据加载完成。已加载 " + to_string(data_.lines.size()) + " 条线路。");
    } else {
        Utils::printError("未找到数据文件或文件内容为空，请检查 'data/metro_data.txt' 文件。");
    }
}

DataManager::~DataManager() {
    saveData();
}

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

        try {
            line.fullPrice = stod(parts[1]);
        } catch (const std::invalid_argument& e) {
            Utils::printError("数据格式错误 (票价非数字) - 第 " + to_string(lineCount) + " 行。此线路将被忽略。");
            continue;
        } catch (const std::out_of_range& e) {
            Utils::printError("数据格式错误 (票价数字溢出) - 第 " + to_string(lineCount) + " 行。此线路将被忽略。");
            continue;
        }
        
        string stationsStr = "";
        for (size_t i = 4; i < parts.size(); ++i) {
            stationsStr += parts[i];
            if (i < parts.size() - 1) stationsStr += ';'; 
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
    file.close();
    Utils::printMessage("数据已自动保存到文件: " + filePath_);
}


// ========== MetroGraph 类实现 (MetroGraph.cpp) ==========

MetroGraph::MetroGraph(MetroData& data) : data_(data) {
    buildGraph();
}

void MetroGraph::processLineForGraph(const Line& line) {
    if (line.stations.size() < 2) return;
    
    // 计算每站间的均摊价格
    double numSegments = (double)line.stations.size() - 1;
    double segmentPrice = (numSegments > 0) ? line.fullPrice / numSegments : 0.0;
    
    for (size_t i = 0; i < line.stations.size() - 1; ++i) {
        int uId = data_.stationNameId.at(line.stations[i]);
        int vId = data_.stationNameId.at(line.stations[i+1]);

        // 默认每站间耗时 2.0 分钟
        adjList[uId].push_back({vId, 2.0, segmentPrice, line.name});
        adjList[vId].push_back({uId, 2.0, segmentPrice, line.name});
    }
}

void MetroGraph::buildGraph() {
    data_.stationNameId.clear();
    data_.stationIdInfo.clear();
    data_.nextStationId = 0;

    for (const auto& linePair : data_.lines) {
        const Line& line = linePair.second;
        for (const std::string& stationName : line.stations) {
            if (data_.stationNameId.find(stationName) == data_.stationNameId.end()) {
                Station newStation;
                newStation.id = data_.nextStationId;
                newStation.name = stationName;
                newStation.lineName = line.name; 

                data_.stationNameId[stationName] = newStation.id;
                data_.stationIdInfo[newStation.id] = newStation;
                data_.nextStationId++;
            }
        }
    }

    adjList.assign(data_.nextStationId, std::vector<Edge>());
    for (const auto& linePair : data_.lines) {
        processLineForGraph(linePair.second);
    }

    // 添加换乘边 (1.0 分钟, 0.0 元)
    std::map<std::string, std::vector<int>> nameToIds;
    for (const auto& pair : data_.stationIdInfo) {
        nameToIds[pair.second.name].push_back(pair.first);
    }

    for (const auto& pair : nameToIds) {
        const auto& ids = pair.second;
        if (ids.size() > 1) {
            for (size_t i = 0; i < ids.size(); ++i) {
                for (size_t j = i + 1; j < ids.size(); ++j) {
                    int u = ids[i];
                    int v = ids[j];
                    
                    // {destId, distance (1.0), price (0.0), lineName}
                    adjList[u].push_back({v, TRANSFER_TIME, 0.0, "TRANSFER"});
                    adjList[v].push_back({u, TRANSFER_TIME, 0.0, "TRANSFER"});
                }
            }
        }
    }
}


void MetroGraph::queryLineInfo(const std::string& lineName) const {
    if (data_.lines.find(lineName) == data_.lines.end()) {
        Utils::printError("线路 [" + lineName + "] 不存在。");
        return;
    }

    const Line& line = data_.lines.at(lineName);
    std::cout << "\n================== 线路 [" << lineName << "] 信息 ==================" << std::endl;
    std::cout << "  - 总票价: " << std::fixed << std::setprecision(1) << line.fullPrice << " 元" << std::endl;
    std::cout << "  - 首班车: " << line.firstTrain << std::endl;
    std::cout << "  - 末班车: " << line.lastTrain << std::endl;
    std::cout << "  - 站点总数: " << line.stations.size() << " 站" << std::endl;
    std::cout << "  - 途径站点: ";
    for (size_t i = 0; i < line.stations.size(); ++i) {
        std::cout << line.stations[i] << (i == line.stations.size() - 1 ? "" : " -> ");
    }
    std::cout << "\n=======================================================" << std::endl;
}

void MetroGraph::queryStationInfo(const std::string& stationName) const {
    if (data_.stationNameId.find(stationName) == data_.stationNameId.end()) {
        Utils::printError("站点 [" + stationName + "] 不存在。");
        return;
    }

    std::cout << "\n================== 站点 [" << stationName << "] 信息 ==================" << std::endl;
    
    std::map<std::string, std::vector<int>> nameToIds;
    for (const auto& pair : data_.stationIdInfo) {
        nameToIds[pair.second.name].push_back(pair.first);
    }

    if (nameToIds.count(stationName) == 0) return; 

    const auto& stationIds = nameToIds.at(stationName);

    std::set<std::string> lines;
    for (int id : stationIds) {
        lines.insert(data_.stationIdInfo.at(id).lineName);
    }
    
    std::cout << "  - 所属线路: ";
    bool first = true;
    for (const auto& line : lines) {
        if (!first) std::cout << ", ";
        std::cout << line;
        first = false;
    }
    std::cout << "\n  - 是否为换乘站: " << (stationIds.size() > 1 ? "是" : "否") << std::endl;
    
    std::cout << "  - 相邻站点信息 (乘车边):" << std::endl;
    std::set<std::string> neighbors;

    for (int uId : stationIds) {
        for (const auto& edge : adjList[uId]) {
            if (edge.lineName != "TRANSFER") {
                std::string neighborName = data_.stationIdInfo.at(edge.destId).name;
                if (neighbors.find(neighborName) == neighbors.end()) {
                    std::cout << "    - 站点 [" << neighborName << "] 途经线路 [" << edge.lineName << "]" << std::endl;
                    neighbors.insert(neighborName);
                }
            }
        }
    }
    if (neighbors.empty()) {
        std::cout << "    - 无相邻乘车站点。" << std::endl;
    }

    std::cout << "=======================================================" << std::endl;
}


bool MetroGraph::addLine(const Line& newLine) {
    if (data_.lines.find(newLine.name) != data_.lines.end()) {
        Utils::printError("线路 [" + newLine.name + "] 已存在，请使用维护功能。");
        return false;
    }
    if (newLine.stations.size() < 2) {
        Utils::printError("新线路 [" + newLine.name + "] 站点数量少于2个。");
        return false;
    }

    data_.lines[newLine.name] = newLine;
    buildGraph(); 

    Utils::printMessage("线路 [" + newLine.name + "] 已成功增加，请退出程序保存数据。");
    return true;
}

bool MetroGraph::updateLine(const std::string& oldLineName, const Line& newLine) {
    if (data_.lines.find(oldLineName) == data_.lines.end()) {
        Utils::printError("旧线路 [" + oldLineName + "] 不存在，无法更新。");
        return false;
    }
    if (newLine.stations.size() < 2) {
        Utils::printError("新线路 [" + newLine.name + "] 站点数量少于2个，更新失败。");
        return false;
    }

    data_.lines.erase(oldLineName);
    data_.lines[newLine.name] = newLine;
    
    buildGraph();

    Utils::printMessage("线路 [" + oldLineName + "] 已成功更新为 [" + newLine.name + "]，请退出程序保存数据。");
    return true;
}

void MetroGraph::displayAllInfo() const {
    std::cout << "\n================== 地铁系统总览 ==================" << std::endl;
    std::cout << "总线路数: " << data_.lines.size() << ", 总站点数: " << data_.stationNameId.size() << std::endl;
    
    std::cout << "\n--- 所有线路信息 ---" << std::endl;
    for (const auto& pair : data_.lines) {
        const Line& line = pair.second;
        std::cout << "[" << line.name << "] 票价:" << std::fixed << std::setprecision(1) << line.fullPrice << " 元 | 站点: " << line.stations.size() << "站" << std::endl;
        std::cout << "  途径: ";
        for (size_t i = 0; i < line.stations.size(); ++i) { 
            std::cout << line.stations[i] << (i == line.stations.size() - 1 ? "" : " -> ");
        }
        std::cout << std::endl;
    }
    std::cout << "==================================================" << std::endl;
}

std::vector<int> MetroGraph::dijkstra(int startId, int endId, std::vector<double>& results, bool usePrice) const {
    results.assign(data_.nextStationId, INF);
    std::vector<int> predecessors(data_.nextStationId, -1);
    
    std::priority_queue<std::pair<double, int>, 
                        std::vector<std::pair<double, int>>, 
                        std::greater<std::pair<double, int>>> pq;

    results[startId] = 0.0;
    pq.push({0.0, startId});

    while (!pq.empty()) {
        double d = pq.top().first;
        int u = pq.top().second;
        pq.pop();

        if (d > results[u]) continue;
        if (u == endId) break;

        for (const auto& edge : adjList[u]) {
            int v = edge.destId; 
            double weight = usePrice ? edge.price : edge.distance; 

            if (results[u] + weight < results[v]) {
                results[v] = results[u] + weight;
                predecessors[v] = u;
                pq.push({results[v], v});
            }
        }
    }

    std::vector<int> path;
    int current = endId;
    while (current != -1) {
        path.push_back(current);
        if (current == startId) break;
        current = predecessors[current];
    }
    std::reverse(path.begin(), path.end());
    
    if (path.empty() || path[0] != startId) {
        return {};
    }
    return path;
}

Route MetroGraph::pathIdToRoute(const std::vector<int>& pathIds, double totalCost, double totalPrice) const {
    Route route;
    route.totalCost = totalCost;
    route.totalPrice = totalPrice; 
    std::string summary = "";

    if (pathIds.size() < 2) return route;

    std::string currentLineName = "";
    int currentSegmentStartId = pathIds[0];
    double currentSegmentTime = 0.0; 
    double currentSegmentPrice = 0.0; 

    for (size_t i = 0; i < pathIds.size() - 1; ++i) {
        int u = pathIds[i];
        int v = pathIds[i + 1];

        bool edgeFound = false;
        for (const auto& edge : adjList[u]) {
            if (edge.destId == v) {
                edgeFound = true;
                std::string nextLineName = edge.lineName;
                
                if (nextLineName == "TRANSFER" || 
                    (currentLineName != "" && nextLineName != currentLineName)) {
                    
                    if (currentLineName != "") {
                        std::string lineSum = data_.stationIdInfo.at(currentSegmentStartId).name + " -> (" + currentLineName + ") -> " + data_.stationIdInfo.at(u).name;
                        summary += (summary.empty() ? "" : " -> ") + lineSum;
                        
                        route.segments.push_back({
                            data_.stationIdInfo.at(currentSegmentStartId).name,
                            data_.stationIdInfo.at(u).name,
                            currentLineName,
                            currentSegmentTime,
                            currentSegmentPrice 
                        });//添加路径新元素
                    }
                    
                    if (nextLineName == "TRANSFER") {//换乘
                        route.segments.push_back({
                            data_.stationIdInfo.at(u).name, 
                            data_.stationIdInfo.at(u).name, 
                            "TRANSFER",
                            TRANSFER_TIME, 
                            0.0 
                        });
                        currentSegmentStartId = v; 
                        currentSegmentTime = 0.0;
                        currentSegmentPrice = 0.0;
                    } else {
                        currentLineName = nextLineName;
                        currentSegmentStartId = u;
                        currentSegmentTime = 0.0; 
                        currentSegmentPrice = 0.0;
                    }
                } else if (currentLineName == "") {
                    currentLineName = nextLineName;
                    currentSegmentStartId = u;
                }
                
                if (nextLineName != "TRANSFER") {
                    currentSegmentTime += edge.distance;
                    currentSegmentPrice += edge.price; 
                }

                break;
            }
        }
        if (!edgeFound) {
             return {};
        }
    }

    if (pathIds.size() >= 2) {
        std::string finalStationName = data_.stationIdInfo.at(pathIds.back()).name;
        
        std::string lineSum = data_.stationIdInfo.at(currentSegmentStartId).name + " -> (" + currentLineName + ") -> " + finalStationName;
        summary += (summary.empty() ? "" : " -> ") + lineSum;
        
        route.segments.push_back({
            data_.stationIdInfo.at(currentSegmentStartId).name,
            finalStationName,
            currentLineName,
            currentSegmentTime,
            currentSegmentPrice 
        });
    }

    route.description = summary;
    return route;
}

void MetroGraph::dfsFindRoutes(int currentId, int endId, std::vector<int>& currentPath, 
                               std::vector<bool>& visited, double currentTime, double currentPrice,
                               std::vector<Route>& allRoutes, int maxStops, double maxCost) const {
    
    currentPath.push_back(currentId);
    visited[currentId] = true;

    if (currentPath.size() > maxStops || currentTime > maxCost) {
        visited[currentId] = false;
        currentPath.pop_back();
        return;
    }
    
    if (currentId == endId) {
        Route newRoute = pathIdToRoute(currentPath, currentTime, currentPrice);
        // 只有当路径有效（非空）时才添加
        if (!newRoute.segments.empty()) {
            allRoutes.push_back(newRoute);
        }
        
        visited[currentId] = false;
        currentPath.pop_back();
        return;
    }

    for (const auto& edge : adjList[currentId]) {
        int nextId = edge.destId; 
        if (!visited[nextId]) {
            dfsFindRoutes(nextId, endId, currentPath, visited, 
                          currentTime + edge.distance, 
                          currentPrice + edge.price, 
                          allRoutes, maxStops, maxCost);
        }
    }

    visited[currentId] = false;
    currentPath.pop_back();
}

std::vector<Route> MetroGraph::findRoutes(const std::string& start, const std::string& end, bool usePrice) const {
    if (data_.stationNameId.find(start) == data_.stationNameId.end() || data_.stationNameId.find(end) == data_.stationNameId.end()) {
        Utils::printError("起始或终点站点不存在。");
        return {};
    }

    int startId = data_.stationNameId.at(start);
    int endId = data_.stationNameId.at(end);
    std::vector<Route> allRoutes;

    // 1. Dijkstra 找到最短路径 (按用户选择的维度)
    std::vector<double> shortestResults; 
    std::vector<int> shortestPathIds = dijkstra(startId, endId, shortestResults, usePrice);
    
    if (shortestPathIds.empty()) {
        Utils::printError("找不到从 [" + start + "] 到 [" + end + "] 的路径。");
        return {};
    }
    
    double shortestCostValue = shortestResults[endId];
    double shortestPrice;
    double shortestTime;

    // 2. 重新运行 Dijkstra 获取另一种成本，以便完整记录 Route 信息
    if (usePrice) {
        std::vector<double> timeResults;
        dijkstra(startId, endId, timeResults, false); 
        shortestPrice = shortestCostValue;
        shortestTime = timeResults[endId];
    } else {
        std::vector<double> priceResults;
        dijkstra(startId, endId, priceResults, true); 
        shortestTime = shortestCostValue;
        shortestPrice = priceResults[endId];
    }

    Route shortestRoute = pathIdToRoute(shortestPathIds, shortestTime, shortestPrice);
    allRoutes.push_back(shortestRoute);

    // 3. DFS 查找其他非最优路径（基于最短时间限制）
    std::vector<int> currentPath;
    std::vector<bool> visited(data_.nextStationId, false);
    
    int maxStops = shortestPathIds.size() + 5; 
    double maxTime = shortestTime * 1.5; 
    /*我知道题目需要展示所有线路，但是dfs跑全图有点过于耗时
    这里剪枝掉不需要所有路径
    如果需要展示所有线路，将maxStops和maxTime设置为INF即可
    */

    dfsFindRoutes(startId, endId, currentPath, visited, 0.0, 0.0, allRoutes, maxStops, maxTime);

    // 4. 排序和去重
    std::sort(allRoutes.begin(), allRoutes.end(), [&](const Route& a, const Route& b) {
        if (usePrice) {
            if (std::abs(a.totalPrice - b.totalPrice) > 1e-6) return a.totalPrice < b.totalPrice;
            return a.totalCost < b.totalCost;
        } else {
            if (std::abs(a.totalCost - b.totalCost) > 1e-6) return a.totalCost < b.totalCost;
            return a.totalPrice < b.totalPrice;
        }
    });

    std::vector<Route> uniqueRoutes;
    std::set<std::vector<int>> pathSet;
    for (const auto& route : allRoutes) {
        std::vector<int> idPath;
        if (route.segments.empty()) continue;

        idPath.push_back(data_.stationNameId.at(route.segments.front().startStation));
        for (const auto& seg : route.segments) {
            if (seg.lineName != "TRANSFER") {
                 idPath.push_back(data_.stationNameId.at(seg.endStation));
            }
        }
        
        if (!idPath.empty() && pathSet.find(idPath) == pathSet.end()) {
            pathSet.insert(idPath);
            uniqueRoutes.push_back(route);
        }
    }
    allRoutes = uniqueRoutes;


    // 5. 格式化输出
    std::cout << "\n================ 乘车查询结果 (" << (usePrice ? "最短价格优先" : "最短时间优先") << ") ================" << std::endl;
    std::cout << "起点: " << start << ", 终点: " << end << std::endl;
    std::cout << "共找到 " << allRoutes.size() << " 种有效方案。" << std::endl;
    
    if (usePrice) {
        std::cout << "最短价格方案: " << std::fixed << std::setprecision(1) << shortestPrice << " 元, 耗时: " << shortestTime << " 分钟。" << std::endl;
    } else {
        std::cout << "最短时间方案: " << std::fixed << std::setprecision(1) << shortestTime << " 分钟, 价格: " << shortestPrice << " 元。" << std::endl;
    }
//这里还是写一个最大输出控制吧不然看着都害怕
//最多输出5条线路
    int maxRoutesToPrint = 20;
    for (size_t i = 0; i < allRoutes.size(); ++i) {
        if(i >= maxRoutesToPrint) {
            std::cout << "最多仅展示 " << maxRoutesToPrint << " 条线路。" << std::endl;
            std::cout<<"在756行可以更改最大展示数，可以全部输出，但是我随便跑了一下200条还是太恐怖点"<<std::endl;
            break;
        }
        Utils::printRoute(allRoutes[i], i + 1);
    }
    
    return allRoutes;
}


// ========== 主程序 (main.cpp) ==========

using namespace std;

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

void clearInput() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

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
    string dataFilePath = "data/metro_data.txt";

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
            case 1: 
                cout << "请输入要查询的线路名称 (例如: 1号线): ";
                cin >> inputStr1; clearInput();
                metroGraph.queryLineInfo(inputStr1);
                break;

            case 2: 
                cout << "请输入要查询的站点名称 (例如: 会展中心): ";
                cin >> inputStr1; clearInput();
                metroGraph.queryStationInfo(inputStr1);
                break;

            case 3: { 
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

            case 4: 
                handleAddLine(metroGraph);
                break;

            case 5: 
                handleUpdateLine(metroGraph);
                break;
                
            case 6: 
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