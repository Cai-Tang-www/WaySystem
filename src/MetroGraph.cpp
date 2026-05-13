#include "../include/MetroGraph.h"
#include "../include/Utils.h"
#include <algorithm>
#include <queue>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cmath> 
#include <set>   
#include <stdexcept>

using namespace std;

// 构造函数
MetroGraph::MetroGraph(MetroData& data) : data_(data) {
    buildGraph();
}

// 私有辅助函数：根据线路信息在图中添加站点和边
void MetroGraph::processLineForGraph(const Line& line) {
    if (line.stations.size() < 2) return;
    
    // 计算每站间的均摊价格
    double numSegments = (double)line.stations.size() - 1;
    // 站间价格 = 线路总票价 / 站间段数 (防止除以零)
    double segmentPrice = (numSegments > 0) ? line.fullPrice / numSegments : 0.0;
    
    // 遍历站点列表，添加双向边 (默认每站间耗时 2.0 分钟)
    for (size_t i = 0; i < line.stations.size() - 1; ++i) {
        int uId = data_.stationNameId.at(line.stations[i]);
        int vId = data_.stationNameId.at(line.stations[i+1]);

        // u -> v: {destId, distance (2.0), price, lineName}
        adjList[uuId].push_back({vId, 2.0, segmentPrice, line.name});
        // v -> u
        adjList[vId].push_back({uId, 2.0, segmentPrice, line.name});
    }
}

// 私有辅助函数：根据当前的 MetroData 重新构建邻接表
void MetroGraph::buildGraph() {
    // 1. 重新注册和分配站点 ID
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

    // 2. 初始化邻接表并添加乘车边
    adjList.assign(data_.nextStationId, std::vector<Edge>());
    for (const auto& linePair : data_.lines) {
        processLineForGraph(linePair.second);
    }

    // 3. 添加换乘边
    std::map<std::string, std::vector<int>> nameToIds;
    for (const auto& pair : data_.stationIdInfo) {
        nameToIds[pair.second.name].push_back(pair.first);
    }

    for (const auto& pair : nameToIds) {
        const auto& ids = pair.second;
        // 只有站点名称对应多个ID时，才是换乘站
        if (ids.size() > 1) {
            for (size_t i = 0; i < ids.size(); ++i) {
                for (size_t j = i + 1; j < ids.size(); ++j) {
                    int u = ids[i];
                    int v = ids[j];
                    
                    // Edge 结构体: {destId, distance (1.0), price (0.0), lineName}
                    adjList[u].push_back({v, TRANSFER_TIME, 0.0, "TRANSFER"});
                    adjList[v].push_back({u, TRANSFER_TIME, 0.0, "TRANSFER"});
                }
            }
        }
    }
}


// 1. 线路信息查询
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

// 2. 站点信息查询
void MetroGraph::queryStationInfo(const std::string& stationName) const {
    if (data_.stationNameId.find(stationName) == data_.stationNameId.end()) {
        Utils::printError("站点 [" + stationName + "] 不存在。");
        return;
    }

    std::cout << "\n================== 站点 [" << stationName << "] 信息 ==================" << std::endl;
    
    // 查找所有 ID (用于处理换乘站)
    std::map<std::string, std::vector<int>> nameToIds;
    for (const auto& pair : data_.stationIdInfo) {
        nameToIds[pair.second.name].push_back(pair.first);
    }

    if (nameToIds.count(stationName) == 0) return; 

    const auto& stationIds = nameToIds.at(stationName);

    // 统计所属线路
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
            // 排除换乘边，只考虑同线乘车边
            if (edge.lineName != "TRANSFER") {
                std::string neighborName = data_.stationIdInfo.at(edge.destId).name;
                // 仅打印一次相邻站点的名称
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


// 4. 线路增加
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
    buildGraph(); // 重建图以包含新线路

    Utils::printMessage("线路 [" + newLine.name + "] 已成功增加，请退出程序保存数据。");
    return true;
}

// 5. 线路信息维护（更新）
bool MetroGraph::updateLine(const std::string& oldLineName, const Line& newLine) {
    // 1. 检查旧线路是否存在
    if (data_.lines.find(oldLineName) == data_.lines.end()) {
        Utils::printError("旧线路 [" + oldLineName + "] 不存在，无法更新。");
        return false;
    }
    if (newLine.stations.size() < 2) {
        Utils::printError("新线路 [" + newLine.name + "] 站点数量少于2个，更新失败。");
        return false;
    }

    // 2. 移除旧线路
    data_.lines.erase(oldLineName);

    // 3. 添加新线路 (覆盖或替换)
    data_.lines[newLine.name] = newLine;
    
    // 4. 重建图（重新分配ID和边）
    buildGraph();

    Utils::printMessage("线路 [" + oldLineName + "] 已成功更新为 [" + newLine.name + "]，请退出程序保存数据。");
    return true;
}


// 6. 辅助功能：显示所有线路和站点
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


// 辅助函数：Dijkstra 最短路径算法 (新增 usePrice 参数)
std::vector<int> MetroGraph::dijkstra(int startId, int endId, std::vector<double>& results, bool usePrice) const {
    results.assign(data_.nextStationId, INF);
    std::vector<int> predecessors(data_.nextStationId, -1);
    
    // priority_queue 存储 {cost, vertexId}
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
            // 根据 usePrice 选择权值
            double weight = usePrice ? edge.price : edge.distance; 

            if (results[u] + weight < results[v]) {
                results[v] = results[u] + weight;
                predecessors[v] = u;
                pq.push({results[v], v});
            }
        }
    }

    // 路径重构
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

// 辅助函数：将ID路径转换为Route结构 (同时记录时间和价格)
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

        // 查找边信息
        bool edgeFound = false;
        for (const auto& edge : adjList[u]) {
            if (edge.destId == v) {
                edgeFound = true;
                std::string nextLineName = edge.lineName;
                
                // 1. 遇到换乘或线路切换
                if (nextLineName == "TRANSFER" || 
                    (currentLineName != "" && nextLineName != currentLineName)) {
                    
                    // 结束前一段的乘车部分
                    if (currentLineName != "") {
                        // 构建摘要
                        std::string lineSum = data_.stationIdInfo.at(currentSegmentStartId).name + " -> (" + currentLineName + ") -> " + data_.stationIdInfo.at(u).name;
                        summary += (summary.empty() ? "" : " -> ") + lineSum;
                        
                        // 结束当前 RouteSegment
                        route.segments.push_back({
                            data_.stationIdInfo.at(currentSegmentStartId).name,
                            data_.stationIdInfo.at(u).name,
                            currentLineName,
                            currentSegmentTime,
                            currentSegmentPrice 
                        });
                    }
                    
                    // 处理换乘
                    if (nextLineName == "TRANSFER") {
                        route.segments.push_back({
                            data_.stationIdInfo.at(u).name, 
                            data_.stationIdInfo.at(u).name, 
                            "TRANSFER",
                            TRANSFER_TIME, // 1.0 分钟
                            0.0 // 0.0 元
                        });
                        currentSegmentStartId = v; // 换乘后的新起点ID
                        currentSegmentTime = 0.0;
                        currentSegmentPrice = 0.0;
                    } else {
                        // 线路切换，重置段信息
                        currentLineName = nextLineName;
                        currentSegmentStartId = u;
                        currentSegmentTime = 0.0; 
                        currentSegmentPrice = 0.0;
                    }
                } else if (currentLineName == "") {
                    // 线路开始
                    currentLineName = nextLineName;
                    currentSegmentStartId = u;
                }
                
                // 累加时间和价格 (非换乘段)
                if (nextLineName != "TRANSFER") {
                    currentSegmentTime += edge.distance;
                    currentSegmentPrice += edge.price; 
                }

                break;
            }
        }
        if (!edgeFound) {
             // 错误处理：找不到边
             return {};
        }
    }

    // 结束最后一段
    if (pathIds.size() >= 2) {
        std::string finalStationName = data_.stationIdInfo.at(pathIds.back()).name;
        
        // 构建摘要
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


// 【DFS 辅助函数】查找多条非最优路径 (追踪时间和价格)
void MetroGraph::dfsFindRoutes(int currentId, int endId, std::vector<int>& currentPath, 
                               std::vector<bool>& visited, double currentTime, double currentPrice,
                               std::vector<Route>& allRoutes, int maxStops, double maxCost) const {
    
    currentPath.push_back(currentId);
    visited[currentId] = true;

    // 限制路径长度或时间，防止搜索爆炸
    if (currentPath.size() > maxStops || currentTime > maxCost) {
        visited[currentId] = false;
        currentPath.pop_back();
        return;
    }
    
    if (currentId == endId) {
        // 找到终点，转换为 Route 结构并保存
        Route newRoute = pathIdToRoute(currentPath, currentTime, currentPrice);
        allRoutes.push_back(newRoute);
        
        visited[currentId] = false;
        currentPath.pop_back();
        return;
    }

    for (const auto& edge : adjList[currentId]) {
        int nextId = edge.destId; 
        if (!visited[nextId]) {
            dfsFindRoutes(nextId, endId, currentPath, visited, 
                          currentTime + edge.distance, 
                          currentPrice + edge.price, // 追踪价格
                          allRoutes, maxStops, maxCost);
        }
    }

    visited[currentId] = false;
    currentPath.pop_back();
}


// 3. 乘车查询: 查找所有有效乘车方案，并给出最短方案 (新增 usePrice 参数)
std::vector<Route> MetroGraph::findRoutes(const std::string& start, const std::string& end, bool usePrice) const {
    // 检查起始和终点是否存在
    if (data_.stationNameId.find(start) == data_.stationNameId.end() || data_.stationNameId.find(end) == data_.stationNameId.end()) {
        Utils::printError("起始或终点站点不存在。");
        return {};
    }

    int startId = data_.stationNameId.at(start);
    int endId = data_.stationNameId.at(end);
    std::vector<Route> allRoutes;

    // 1. Dijkstra 算法找到最短路径 (按用户选择的维度)
    std::vector<double> shortestResults; 
    std::vector<int> shortestPathIds = dijkstra(startId, endId, shortestResults, usePrice);
    
    if (shortestPathIds.empty()) {
        Utils::printError("找不到从 [" + start + "] 到 [" + end + "] 的路径。");
        return {};
    }
    
    // 2. 重新运行 Dijkstra 获取另一种成本，以便完整记录 Route 信息
    double shortestCostValue = shortestResults[endId];
    double shortestPrice;
    double shortestTime;

    if (usePrice) {
        // shortestCostValue = 最短价格。需要再次运行Dijkstra获取最短时间。
        std::vector<double> timeResults;
        dijkstra(startId, endId, timeResults, false); // false = 按时间
        shortestPrice = shortestCostValue;
        shortestTime = timeResults[endId];
    } else {
        // shortestCostValue = 最短时间。需要再次运行Dijkstra获取最短价格。
        std::vector<double> priceResults;
        dijkstra(startId, endId, priceResults, true); // true = 按价格
        shortestTime = shortestCostValue;
        shortestPrice = priceResults[endId];
    }

    // 转换为 Route 结构，加入结果集
    Route shortestRoute = pathIdToRoute(shortestPathIds, shortestTime, shortestPrice);
    allRoutes.push_back(shortestRoute);

    // 3. DFS 查找其他非最优路径（基于最短时间限制）
    std::vector<int> currentPath;
    std::vector<bool> visited(data_.nextStationId, false);
    
    // DFS 限制
    int maxStops = shortestPathIds.size() + 5; 
    double maxTime = shortestTime * 1.5; 

/*我知道题目需要展示所有线路，但是dfs跑全图有点过于耗时
这里剪枝掉不需要路径
如果需要展示所有线路，将maxStops设置为INF即可
*/

    // 传入初始的时间和价格为 0
    dfsFindRoutes(startId, endId, currentPath, visited, 0.0, 0.0, allRoutes, maxStops, maxTime);

    // 4. 排序和去重 (按用户选择的成本排序)
    std::sort(allRoutes.begin(), allRoutes.end(), [&](const Route& a, const Route& b) {
        if (usePrice) {
            // 最短价格优先，价格相同时最短时间优先
            if (std::abs(a.totalPrice - b.totalPrice) > 1e-6) return a.totalPrice < b.totalPrice;
            return a.totalCost < b.totalCost;
        } else {
            // 最短时间优先，时间相同时最短价格优先
            if (std::abs(a.totalCost - b.totalCost) > 1e-6) return a.totalCost < b.totalCost;
            return a.totalPrice < b.totalPrice;
        }
    });

    // 简单的去重逻辑：基于站点序列
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

    for (size_t i = 0; i < allRoutes.size(); ++i) {
        Utils::printRoute(allRoutes[i], i + 1);
    }
    
    return allRoutes;
}