#ifndef METROGRAPH_H
#define METROGRAPH_H

#include "Structures.h"
#include <vector>
#include <string>
#include <map>
#include <algorithm> 
// MetroGraph 类使用图结构（邻接表）存储地铁网络，并实现核心查询算法。
class MetroGraph {
public:
    // 构造函数，初始化图
    MetroGraph(MetroData& data);

    // 1. 线路查询: 查询某条线路的所有信息
    void queryLineInfo(const std::string& lineName) const;

    // 2. 站点信息查询: 显示站点所在线路、上一站、下一站
    void queryStationInfo(const std::string& stationName) const;

    // 3. 乘车查询: 查找所有有效乘车方案，并给出最短方案 (新增 usePrice 参数)
    std::vector<Route> findRoutes(const std::string& start, const std::string& end, bool usePrice) const;

    // 4. 线路增加
    bool addLine(const Line& newLine);

    // 5. 线路信息维护（更新）
    bool updateLine(const std::string& oldLineName, const Line& newLine);

    // 6. 辅助功能：显示所有线路和站点
    void displayAllInfo() const;

    MetroData& getData() { return data_; }
    const MetroData& getData() const { return data_; }
    
private:
    MetroData& data_;
    std::vector<std::vector<Edge>> adjList; // 图的邻接表

    // 私有辅助函数：
    // 初始化/更新邻接表
    void buildGraph();
    
    // 辅助函数：根据线路信息在图中添加站点和边
    void processLineForGraph(const Line& line);

    // Dijkstra 最短路径算法 (新增 usePrice 参数)
    std::vector<int> dijkstra(int startId, int endId, std::vector<double>& results, bool usePrice) const;
    
    // 将ID路径转换为Route结构 (新增 totalPrice 参数)
    Route pathIdToRoute(const std::vector<int>& pathIds, double totalCost, double totalPrice) const;

    // DFS 查找多条非最优路径 (新增 currentPrice 参数)
    void dfsFindRoutes(int currentId, int endId, std::vector<int>& currentPath, 
                       std::vector<bool>& visited, double currentTime, double currentPrice,
                       std::vector<Route>& allRoutes, int maxStops, double maxCost) const;
};

#endif // METROGRAPH_H