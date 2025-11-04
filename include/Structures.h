#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <string>
#include <vector>
#include <map>
#include <limits>

// 定义无穷大，用于最短路径算法
const double INF = std::numeric_limits<double>::infinity();
// 定义换乘时间（模拟）
const double TRANSFER_TIME = 3.0; // 换乘需要3分钟

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

// 乘车方案结构体，片段实现分段储存
struct RouteSegment {
    std::string startStation;
    std::string endStation;
    std::string lineName;
    double cost; // 花费（可能是时间或距离）
};

// 完整的乘车方案
struct Route {
    std::vector<RouteSegment> segments;
    double totalCost;
    std::string description; // 方案描述 (例如: 1号线 -> 4号线)
};

// 邻接表中的边（图的边）
struct Edge {
    int destId;      // 目标站点ID
    double distance; // 边的权值（距离或时间）
    std::string lineName; // 边所属的线路名称
};

// 全局数据容器
struct MetroData {
    std::map<std::string, Line> lines;        // 线路名称 -> Line 对象
    std::map<std::string, int> stationNameId; // 站点名称 -> 站点ID
    std::map<int, Station> stationIdInfo;     // 站点ID -> 站点信息
    int nextStationId = 0;                    // 下一个可用的站点ID
};

#endif // STRUCTURES_H