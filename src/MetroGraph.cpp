#include "../include/MetroGraph.h"
#include "../include/Utils.h"
#include <algorithm>
#include <queue>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cmath> // 用于 std::abs
#include <set>   // 用于 queryStationInfo

using namespace std;

// 构造函数
MetroGraph::MetroGraph(MetroData& data) : data_(data) {
    buildGraph();
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
                newStation.lineName = line.name; // 使用遇到的第一条线路名称

                data_.stationNameId[stationName] = newStation.id;
                data_.stationIdInfo[newStation.id] = newStation;
                data_.nextStationId++;
            }
        }
    }

    // 2. 重新调整邻接表大小
    adjList.assign(data_.nextStationId, std::vector<Edge>());

    // 3. 添加线路段（站点之间的连接）
    for (const auto& pair : data_.lines) {
        processLineForGraph(pair.second);
    }

    // 4. 添加换乘边 (在同一个站点的不同线路之间添加虚拟边)
    std::map<std::string, std::vector<int>> stationIdsInSameLoc;
    for (const auto& linePair : data_.lines) {
        for (const auto& stationName : linePair.second.stations) {
            stationIdsInSameLoc[stationName].push_back(data_.stationNameId.at(stationName));
        }
    }

    for (const auto& stationPair : stationIdsInSameLoc) {
        const std::vector<int>& ids = stationPair.second;
        if (ids.size() > 1) { // 存在多条线路经过该站点
            for (size_t i = 0; i < ids.size(); ++i) {
                for (size_t j = i + 1; j < ids.size(); ++j) {
                    // 添加双向换乘边，权值为 TRANSFER_TIME
                    adjList[ids[i]].push_back({ids[j], TRANSFER_TIME, "TRANSFER"});
                    adjList[ids[j]].push_back({ids[i], TRANSFER_TIME, "TRANSFER"});
                }
            }
        }
    }
}

// 辅助函数：根据线路信息在图中添加站点和边
void MetroGraph::processLineForGraph(const Line& line) {
    for (size_t i = 0; i < line.stations.size() - 1; ++i) {
        int u = data_.stationNameId.at(line.stations[i]);
        int v = data_.stationNameId.at(line.stations[i + 1]);
        // 假设站点间的距离/时间是固定的（例如 2.0 分钟）
        double travelTime = 2.0; 

        // 添加双向边 (使用正确的 Edge 成员: destId, distance)
        adjList[u].push_back({v, travelTime, line.name});
        adjList[v].push_back({u, travelTime, line.name});
    }
}


// 辅助函数：Dijkstra 最短路径算法
std::vector<int> MetroGraph::dijkstra(int startId, int endId, std::vector<double>& distances) const {
    distances.assign(data_.nextStationId, INF);
    std::vector<int> predecessors(data_.nextStationId, -1);
    std::priority_queue<std::pair<double, int>, 
                        std::vector<std::pair<double, int>>, 
                        std::greater<std::pair<double, int>>> pq;

    distances[startId] = 0.0;
    pq.push({0.0, startId});

    while (!pq.empty()) {
        double d = pq.top().first;
        int u = pq.top().second;
        pq.pop();

        if (d > distances[u]) continue;
        if (u == endId) break;

        for (const auto& edge : adjList[u]) {
            int v = edge.destId; 
            double weight = edge.distance; 

            if (distances[u] + weight < distances[v]) {
                distances[v] = distances[u] + weight;
                predecessors[v] = u;
                pq.push({distances[v], v});
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

// 辅助函数：将ID路径转换为Route结构
Route MetroGraph::pathIdToRoute(const std::vector<int>& pathIds, double totalCost) const {
    Route route;
    route.totalCost = totalCost;
    std::string summary = "";

    if (pathIds.size() < 2) return route;

    std::string currentLineName = "";
    int currentSegmentStartId = pathIds[0];
    double currentSegmentTime = 0.0; // 记录当前线路段的累计时间

    for (size_t i = 0; i < pathIds.size() - 1; ++i) {
        int u = pathIds[i];
        int v = pathIds[i + 1];

        // 查找边信息
        for (const auto& edge : adjList[u]) {
            if (edge.destId == v) {
                std::string nextLineName = edge.lineName;
                
                // 1. 遇到换乘或线路切换
                if (nextLineName == "TRANSFER" || 
                    (currentLineName != "" && nextLineName != currentLineName)) {
                    
                    // 结束前一段的乘车部分
                    if (currentLineName != "") {
                        std::string lineSum = data_.stationIdInfo.at(currentSegmentStartId).name + " -> (" + currentLineName + ") -> " + data_.stationIdInfo.at(u).name;
                        summary += (summary.empty() ? "" : " -> ") + lineSum;
                        
                        route.segments.push_back({
                            data_.stationIdInfo.at(currentSegmentStartId).name,
                            data_.stationIdInfo.at(u).name,
                            currentLineName,
                            currentSegmentTime 
                        });
                    }
                    
                    // 处理换乘
                    if (nextLineName == "TRANSFER") {
                        route.segments.push_back({
                            data_.stationIdInfo.at(u).name, 
                            data_.stationIdInfo.at(u).name, 
                            "TRANSFER",
                            TRANSFER_TIME // 换乘时间
                        });
                        currentSegmentStartId = v; // 换乘后的新起点
                        currentSegmentTime = 0.0;
                        // 换乘后，下一段的线路名称需要从 v 往下一跳的边确定，这里先不更新 currentLineName
                    } else {
                        // 线路切换，重置段信息
                        currentLineName = nextLineName;
                        currentSegmentStartId = u;
                        currentSegmentTime = 0.0; // 从 u 往 v 开始计算
                    }
                } else if (currentLineName == "") {
                    // 线路开始
                    currentLineName = nextLineName;
                    currentSegmentStartId = u;
                }
                
                // 累加时间 (非换乘段)
                if (nextLineName != "TRANSFER") {
                    currentSegmentTime += edge.distance;
                }

                break;
            }
        }
    }

    // 结束最后一段
    if (pathIds.size() >= 2) {
        std::string finalStationName = data_.stationIdInfo.at(pathIds.back()).name;
        std::string lineSum = data_.stationIdInfo.at(currentSegmentStartId).name + " -> (" + currentLineName + ") -> " + finalStationName;
        summary += (summary.empty() ? "" : " -> ") + lineSum;
        route.segments.push_back({
            data_.stationIdInfo.at(currentSegmentStartId).name,
            finalStationName,
            currentLineName,
            currentSegmentTime
        });
    }

    route.description = summary;
    return route;
}


// 【DFS 辅助函数】查找多条非最优路径
void MetroGraph::dfsFindRoutes(int currentId, int endId, std::vector<int>& currentPath, 
                               std::vector<bool>& visited, double currentTime, 
                               std::vector<Route>& allRoutes, int maxStops) const {
    
    currentPath.push_back(currentId);
    visited[currentId] = true;

    if (currentPath.size() > maxStops) {
        visited[currentId] = false;
        currentPath.pop_back();
        return;
    }
    
    if (currentId == endId) {
        Route newRoute = pathIdToRoute(currentPath, currentTime);
        allRoutes.push_back(newRoute);
        
        visited[currentId] = false;
        currentPath.pop_back();
        return;
    }

    for (const auto& edge : adjList[currentId]) {
        int nextId = edge.destId; 
        if (!visited[nextId]) {
            dfsFindRoutes(nextId, endId, currentPath, visited, currentTime + edge.distance, allRoutes, maxStops);
        }
    }

    visited[currentId] = false;
    currentPath.pop_back();
}


// 1. 线路查询: 查询某条线路的所有信息
void MetroGraph::queryLineInfo(const std::string& lineName) const {
    if (data_.lines.find(lineName) == data_.lines.end()) {
        Utils::printError("找不到线路: " + lineName);
        return;
    }
    
    const Line& line = data_.lines.at(lineName);
    std::cout << "\n================ 线路信息查询 ==================" << std::endl;
    std::cout << "线路名称: " << line.name << std::endl;
    std::cout << "全程票价: " << std::fixed << std::setprecision(1) << line.fullPrice << " RMB" << std::endl;
    std::cout << "首班时间: " << line.firstTrain << ", 末班时间: " << line.lastTrain << std::endl;
    std::cout << "总站点数: " << line.stations.size() << "站" << std::endl;
    
    std::cout << "途径站点: ";
    for (size_t i = 0; i < line.stations.size(); ++i) {
        std::cout << line.stations[i] << (i < line.stations.size() - 1 ? " -> " : "");
    }
    std::cout << "\n==============================================\n" << std::endl;
}

// 2. 站点信息查询: 显示站点所在线路、上一站、下一站
void MetroGraph::queryStationInfo(const std::string& stationName) const {
    if (data_.stationNameId.find(stationName) == data_.stationNameId.end()) {
        Utils::printError("找不到站点: " + stationName);
        return;
    }

    int stationId = data_.stationNameId.at(stationName);
    
    std::cout << "\n================ 站点信息查询 ==================" << std::endl;
    std::cout << "站点名称: " << stationName << std::endl;
    
    // 查找所有经过该站点的线路
    std::set<std::string> lines;
    for (const auto& pair : data_.lines) {
        for (const auto& s : pair.second.stations) {
            if (s == stationName) {
                lines.insert(pair.first);
                break;
            }
        }
    }

    std::cout << "所属线路: ";
    if (lines.empty()) {
        std::cout << "无 (数据错误)" << std::endl;
    } else {
        bool first = true;
        for (const auto& line : lines) {
            if (!first) std::cout << ", ";
            std::cout << line;
            first = false;
        }
        std::cout << std::endl;
    }
    
    // 查找相邻站点（通过邻接表）
    std::cout << "相邻站点:" << std::endl;
    for (const auto& edge : adjList[stationId]) {
        // 假设我们只显示乘车连接的相邻站点，忽略虚拟换乘边
        if (edge.lineName != "TRANSFER") { 
            std::cout << "  - " << data_.stationIdInfo.at(edge.destId).name 
                      << " (线路: " << edge.lineName << ", 耗时: " << std::fixed << std::setprecision(1) << edge.distance << " 分钟)" << std::endl;
        }
    }

    std::cout << "==============================================\n" << std::endl;
}


// 3. 乘车查询: 查找所有有效乘车方案，并给出最短方案 
std::vector<Route> MetroGraph::findRoutes(const std::string& start, const std::string& end) const {
    std::vector<Route> resultRoutes;
    if (data_.stationNameId.find(start) == data_.stationNameId.end() ||
        data_.stationNameId.find(end) == data_.stationNameId.end()) {
        Utils::printError("起始或到达站点名称错误。");
        return resultRoutes;
    }

    int startId = data_.stationNameId.at(start);
    int endId = data_.stationNameId.at(end);
    
    // 步骤 1: 使用 Dijkstra 找到最短路径
    std::vector<double> distances;
    std::vector<int> shortestPathIds = dijkstra(startId, endId, distances);
    
    if (shortestPathIds.empty()) {
        Utils::printError("未找到有效乘车路线。");
        return resultRoutes;
    }

    Route shortestRoute = pathIdToRoute(shortestPathIds, distances[endId]);
    resultRoutes.push_back(shortestRoute); 

    // 步骤 2: 使用 DFS 查找其他有效路径
    int maxStops = shortestPathIds.size() + 5; // 限制 DFS 搜索的路径长度
    std::vector<Route> allOtherRoutes;
    std::vector<int> currentPath;
    std::vector<bool> visited(data_.nextStationId, false); 

    dfsFindRoutes(startId, endId, currentPath, visited, 0.0, allOtherRoutes, maxStops);

    // 将其他路径添加到结果列表（排重）
    for (const auto& route : allOtherRoutes) {
        if (std::abs(route.totalCost - shortestRoute.totalCost) > 0.1) {
            resultRoutes.push_back(route);
        }
    }

    // 排序：按总耗时升序排列所有有效方案
    std::sort(resultRoutes.begin(), resultRoutes.end(), [](const Route& a, const Route& b) {
        return a.totalCost < b.totalCost;
    });

    // 步骤 3: 格式化输出 (先所有方案，再最短总结)
    
    // 打印所有有效方案
    std::cout << "\n================ 所有有效乘车方案 (" << resultRoutes.size() << " 个) ================" << std::endl;
    for (size_t i = 0; i < resultRoutes.size(); ++i) {
        Utils::printRoute(resultRoutes[i], i + 1); 
    }
    std::cout << "\n==============================================\n" << std::endl;

    // 打印最短乘车方案总结 (即第一个方案)
    const Route& finalShortestRoute = resultRoutes[0];
    std::cout << "\n================ 最短乘车方案总结 ===============" << std::endl;
    std::cout << "最优方案：" << finalShortestRoute.description << std::endl;
    std::cout << "总耗时：" << std::fixed << std::setprecision(1) << finalShortestRoute.totalCost << " 分钟" << std::endl;
    std::cout << "==============================================\n" << std::endl;

    return resultRoutes;
}


// 4. 线路增加 【已实现】
bool MetroGraph::addLine(const Line& newLine) {
    if (data_.lines.count(newLine.name)) {
        Utils::printError("线路 '" + newLine.name + "' 已存在，添加失败。");
        return false;
    }

    data_.lines[newLine.name] = newLine;
    // 重新构建图以包含新线路
    buildGraph(); 

    Utils::printMessage("线路 '" + newLine.name + "' 已成功添加，请退出程序保存数据。");
    return true;
}

// 5. 线路信息维护（更新） 
bool MetroGraph::updateLine(const std::string& oldLineName, const Line& newLine) {
    if (data_.lines.find(oldLineName) == data_.lines.end()) {
        Utils::printError("线路 '" + oldLineName + "' 不存在，无法更新。");
        return false;
    }

    // 1. 删除旧线路
    data_.lines.erase(oldLineName);
    
    // 2. 如果新线路名与旧线路名不同，检查是否重复
    if (oldLineName != newLine.name && data_.lines.count(newLine.name)) {
        Utils::printError("新线路名 '" + newLine.name + "' 已存在，请使用其他名称。");
        // 恢复旧线路 (简单起见，不恢复 ID 映射)
        return false;
    }

    // 3. 添加新线路 (覆盖或替换)
    data_.lines[newLine.name] = newLine;
    
    // 4. 重建图（重新分配ID和边）
    buildGraph();

    Utils::printMessage("线路 '" + oldLineName + "' 已成功更新为 '" + newLine.name + "'，请退出程序保存数据。");
    return true;
}


// 6. 辅助功能：显示所有线路和站点 【已修正】
void MetroGraph::displayAllInfo() const {
    std::cout << "\n================== 地铁系统总览 ==================" << std::endl;
    std::cout << "总线路数: " << data_.lines.size() << ", 总站点数: " << data_.stationNameId.size() << std::endl;
    
    std::cout << "\n--- 所有线路信息 ---" << std::endl;
    for (const auto& pair : data_.lines) {
        const Line& line = pair.second;
        std::cout << "[" << line.name << "] 票价:" << line.fullPrice << " | 站点: " << line.stations.size() << "站" << std::endl;
        std::cout << "  途径: ";
        for (size_t i = 0; i < line.stations.size(); ++i) { 
            std::cout << line.stations[i];
            if (i < line.stations.size() - 1) {
                std::cout << " -> ";
            }
        }
        std::cout << std::endl;
    }

    std::cout << "\n--- 站点ID映射 ---" << std::endl;
    int count = 0;
    for (const auto& pair : data_.stationNameId) {
        std::cout << pair.first << "(" << pair.second << ") ";
        count++;
        if (count % 5 == 0) { 
            std::cout << "\n";
        } 
    }
    if (count % 5 != 0 && count < 100) {
        std::cout << "\n";
    }
}