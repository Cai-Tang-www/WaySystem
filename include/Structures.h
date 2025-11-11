#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <string>
#include <vector>
#include <map>
#include <limits>

// 定义无穷大，用于最短路径算法
const double INF = std::numeric_limits<double>::infinity();
// 定义换乘时间（修改为 1.0 分钟）
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
    double price;             // 价格 (元) 【新增】
    std::string lineName;     // 边的所属线路 ("TRANSFER"表示换乘)
};

// 乘车方案结构体，片段实现分段储存
struct RouteSegment {
    std::string startStation;
    std::string endStation;
    std::string lineName;
    double cost; // 花费（时间）
    double price; // 价格 【新增】
};

// 完整的乘车方案
struct Route {
    std::vector<RouteSegment> segments;
    double totalCost; // 总耗时
    double totalPrice; // 总价格 【新增】
    std::string description;
};

// 全局数据容器
struct MetroData {
    std::map<std::string, Line> lines;        // 线路信息 (按名称索引)
    std::map<std::string, int> stationNameId; // 站点名称 -> ID 映射
    std::map<int, Station> stationIdInfo;     // 站点 ID -> 站点信息
    int nextStationId = 0;                    // 下一个可用的站点 ID
};

#endif // STRUCTURES_H