# 嘶真有人来看啊

这是伟大的帝国理工学院的受苦学生的数据结构实践大作业，但是我没想到为什么老师会把这个东西标为“简单”难度，虽然实现上来讲不需要太多思考，但是主打一个麻烦

## 城市地铁管理和查询系统

**1．** **问题描述**

设计良好的数据结构存储某个城市所有的地铁线路，并提供相应的管理和查询功能（测试数据要求至少8条线路，建议以深圳、广州、上海、北京等大城市地铁为例）。

**2****．基本要求**

（1）    线路查询：查询某条线路的票价，首班时间，末班时间，所有途径站点；

（2）    站点信息查询：输入站点名称，显示站点所在线路信息，上一站点信息和下一站点信息；

（3）    乘车查询：输入起始站点和到达站点，显示两点间所有有效的乘车方案及票价，并给出最短乘车方案及票价信息；

（4）    线路增加功能：能够增加一条地铁线路信息；

（5）    线路信息维护功能：能够更改某一条地铁线的所有信息(票价( 全程票价)，首班时间，末班时间，所有途径站点名称)；

（6）    线路信息保存功能：能够保存所有地铁线路信息到文件；

（7）    线路信息读取功能：系统能够从文件中读取所有地铁线路信息；

（8）    设计良好的操作界面；

（9）    仿真图（选做）；

（10）  扩充了增加站点功能，可以不附属车站

（11）不算在主要功能内但已实现：换乘不冲突

（12）实现了模拟计时功能：当前设一站2.0min  

（13）碎片化路径结构体：可以极大程度节约程序运行时间和内存占用，剪枝避免重复搜索

（14）实现了可用价格最低或者时间最短的最短路径查找，我觉得就是新功能；



ps：本项目超40%借助AI生成，个人优化



接下来是代码解读

## 城市地铁管理和查询系统数据文件

格式:

LINE名称；票价；首发时间；末尾时间；站点1，站点2，站点3
LINE2名称；票价；首发时间；末尾时间；站点1，站点2，站点4

换乘站点的名称需确保一致；分隔符也是英文分号

version 1.0下仍未实现票价功能或耗时（时间权重）功能，请等待后续补充

### （我真是服了又要在cmd输出每次都有GBK和UTF-8的冲突）

### 故所有代码均在GB2312格式下编码，包括储存文件metro_data.txt



### 1. Structures.h (数据结构定义)
这个文件定义了项目中所有核心的数据结构，它们是构建地铁系统模型的基础。

- Station 结构体 :
  - 作用 : 表示一个地铁站点。
  - 成员 : id (唯一标识符，用于图的索引)、 name (站点名称)、 lineName (站点所属线路名称)。
  - 思路 : 将站点抽象为图的节点， id 是节点在邻接表中的索引。
- Line 结构体 :
  - 作用 : 表示一条地铁线路。
  - 成员 : name (线路名称)、 fullPrice (全程票价)、 firstTrain (首班时间)、 lastTrain (末班时间)、 stations (途经站点名称列表)。
  - 思路 : 存储线路的详细信息， stations 列表是构建线路内站点连接的关键。
- RouteSegment 结构体 :
  - 作用 : 表示乘车方案中的一个片段（例如：从A站到B站乘坐X号线）。
  - 成员 : startStation 、 endStation 、 lineName 、 cost (花费，通常是时间)。
  - 思路 : 用于描述乘车路径的中间步骤，方便用户理解。
- Route 结构体 :
  - 作用 : 表示一个完整的乘车方案。
  - 成员 : segments (多个 RouteSegment 组成的列表)、 totalCost (总花费)、 description (方案的文字描述)。
  - 思路 : 封装了从起点到终点的完整路径信息，包括所有换乘和乘车段。
- Edge 结构体 :
  - 作用 : 表示图中的一条边。
  - 成员 : destId (目标站点ID)、 distance (边的权值，通常是时间或距离)、 lineName (边所属的线路名称)。
  - 思路 : 这是邻接表的基本元素，用于存储从一个站点到另一个站点的连接信息。
- MetroData 结构体 :
  - 作用 : 全局数据容器，存储整个地铁系统的所有原始数据。
  - 成员 : lines (线路名称到 Line 对象的映射)、 stationNameId (站点名称到站点ID的映射)、 stationIdInfo (站点ID到 Station 信息的映射)、 nextStationId (下一个可用的站点ID，用于动态分配)。
  - 思路 : 集中管理所有数据，方便 DataManager 和 MetroGraph 访问和修改。
### 2. Utils.h 和 Utils.cpp (辅助工具函数)
这个文件提供了一些通用的辅助函数，用于字符串处理和信息输出。

- trim(const std::string& str) :
  - 作用 : 移除字符串首尾的空白字符。
  - 思路 : 在处理用户输入或从文件读取数据时，经常需要清理字符串。
- split(const std::string& str, char delimiter) :
  - 作用 : 根据指定分隔符分割字符串，返回一个字符串向量。
  - 思路 : 用于解析文件中的数据行，将一行数据分割成多个字段。
- printMessage(const std::string& msg) :
  - 作用 : 打印普通提示信息。
  - 思路 : 提供统一的信息输出格式。
- printError(const std::string& msg) :
  - 作用 : 打印错误信息。
  - 思路 : 提供统一的错误输出格式，方便调试和用户反馈。
- printRoute(const Route& route, int index) :
  - 作用 : 格式化打印一个乘车方案。
  - 思路 : 将 Route 结构体中的信息以清晰易读的方式展示给用户，包括换乘信息。
### 3. DataManager.h 和 DataManager.cpp (数据管理)
DataManager 类负责地铁系统数据的持久化，即从文件加载数据和将数据保存到文件。

- DataManager(const std::string& filePath, MetroData& data) (构造函数) :
  - 作用 : 初始化 DataManager ，指定数据文件路径，并引用 MetroData 对象。在构造时自动调用 loadData() 。
  - 思路 : 确保程序启动时数据能够自动加载。
- ~DataManager() (析构函数) :
  - 作用 : 在 DataManager 对象销毁时自动调用 saveData() 。
  - 思路 : 确保程序退出时数据能够自动保存，防止数据丢失。
- loadData() :
  - 作用 : 从 filePath_ 指定的文件中读取地铁线路数据，并填充到 data_ 中。
  - 思路 :
    1. 打开文件，如果失败则打印错误信息。
    2. 逐行读取文件内容。
    3. 使用 Utils::split 函数按分号 ; 分割每行数据，解析出线路名称、票价、首末班时间、站点列表。
    4. 站点列表可能由逗号 , 分隔，再次使用 Utils::split 解析。
    5. 将解析出的数据构建成 Line 对象，并存储到 data_.lines 中。
    6. 包含错误处理，例如票价解析失败、站点数量不足等。
- saveData() const :
  - 作用 : 将 data_ 中存储的地铁线路数据保存到 filePath_ 指定的文件中。
  - 思路 :
    1. 打开文件，如果失败则打印错误信息。
    2. 遍历 data_.lines 中的所有 Line 对象。
    3. 将每个 Line 对象的成员信息格式化为字符串，并写入文件，使用分号 ; 和逗号 , 作为分隔符，与 loadData 的格式保持一致。
### 4. MetroGraph.h 和 MetroGraph.cpp (地铁图核心)
MetroGraph 类是整个系统的核心，它将地铁网络抽象为图结构，并实现了各种查询算法。

- MetroGraph(MetroData& data) (构造函数) :
  - 作用 : 初始化 MetroGraph ，引用 MetroData 对象，并在构造时调用 buildGraph() 。
  - 思路 : 确保图结构在 MetroGraph 对象创建时就构建完成。
- data_ (成员变量) : 对 MetroData 的引用，用于访问和修改原始数据。
- adjList (成员变量) : std::vector<std::vector<Edge>> ，图的邻接表表示。每个元素是一个站点ID，其值是一个 Edge 向量，表示从该站点出发的所有边。
- buildGraph() (私有辅助函数) :
  - 作用 : 根据 MetroData 中的线路信息构建或重新构建图的邻接表。
  - 思路 :
    1. 站点ID注册 : 遍历所有线路的所有站点，为每个唯一的站点分配一个唯一的 id 。维护 stationNameId (名称到ID) 和 stationIdInfo (ID到信息) 映射。
    2. 邻接表大小调整 : 根据分配的 id 数量调整 adjList 的大小。
    3. 添加线路段 : 调用 processLineForGraph() 为每条线路添加站点间的连接边。
    4. 添加换乘边 : 遍历所有站点，如果同一个站点名称下有多个 id (表示多条线路经过此站)，则在这些 id 之间添加权值为 TRANSFER_TIME 的双向换乘边。
  - 算法类型 : 遍历、映射管理。
- processLineForGraph(const Line& line) (辅助函数) :
  - 作用 : 根据一条线路的信息，在图中添加相邻站点之间的边。
  - 思路 : 遍历线路中的站点列表，将相邻的两个站点作为一条边的起点和终点，添加双向边到 adjList 中，边的权值是固定的 travelTime 。
- dijkstra(int startId, int endId, std::vector<double>& distances) const (辅助函数) :
  - 作用 : 实现 Dijkstra 算法，查找从 startId 到 endId 的最短路径。
  - 算法类型 : Dijkstra 最短路径算法 。
  - 思路 :
    1. 初始化 : distances 向量存储从起点到各点的最短距离，初始化为 INF (无穷大)，起点距离为0。 predecessors 向量记录路径前驱。
    2. 优先队列 : 使用 std::priority_queue 存储 (distance, stationId) 对，按距离从小到大排序，每次取出距离最小的节点。
    3. 松弛操作 : 对于当前节点的每个邻居，如果通过当前节点到达邻居的距离更短，则更新邻居的距离和前驱，并将其加入优先队列。
    4. 路径重构 : 从 endId 开始，通过 predecessors 数组逆向回溯，构建出最短路径的站点ID序列。
- pathIdToRoute(const std::vector<int>& pathIds, double totalCost) const (辅助函数) :
  - 作用 : 将 Dijkstra 算法得到的站点ID路径转换为用户友好的 Route 结构。
  - 思路 : 遍历ID路径，识别连续的同线路乘车段和换乘点，将它们封装成 RouteSegment ，并构建 Route 的 description 。
- dfsFindRoutes(int currentId, int endId, std::vector<int>& currentPath, std::vector<bool>& visited, double currentTime, std::vector<Route>& allRoutes, int maxStops) const (DFS 辅助函数) :
  - 作用 : 使用深度优先搜索 (DFS) 查找从 currentId 到 endId 的多条非最优路径。
  - 算法类型 : 深度优先搜索 (DFS) 。
  - 思路 :
    1. 递归 : 这是一个递归函数，每次探索一个新节点。
    2. 路径记录 : currentPath 记录当前探索的路径。
    3. 访问标记 : visited 数组防止循环。
    4. 终止条件 :
       - 如果 currentId == endId ，找到一条完整路径，将其转换为 Route 并添加到 allRoutes 。
       - 如果路径长度超过 maxStops ，剪枝，避免无限循环和过长路径。
    5. 回溯 : 在递归返回时，移除当前节点，重置 visited 标记，以便探索其他分支。
- queryLineInfo(const std::string& lineName) const :
  - 作用 : 查询并显示指定线路的详细信息。
  - 思路 : 从 data_.lines 中查找线路，然后格式化打印其名称、票价、时间、站点列表等。
- queryStationInfo(const std::string& stationName) const :
  - 作用 : 查询并显示指定站点的详细信息，包括所属线路和相邻站点。
  - 思路 :
    1. 通过 data_.stationNameId 获取站点ID。
    2. 遍历 data_.lines 查找所有经过该站点的线路。
    3. 遍历 adjList[stationId] 获取所有相邻站点信息，过滤掉换乘边，只显示乘车连接。
- findRoutes(const std::string& start, const std::string& end) const :
  - 作用 : 查找从起始站点到目标站点的所有有效乘车方案，并给出最短方案。
  - 思路 :
    1. 最短路径 : 首先调用 dijkstra() 找到最短路径。
    2. 其他路径 : 然后调用 dfsFindRoutes() 查找其他非最优路径，通过限制 maxStops 来控制搜索深度。
    3. 结果整合与排序 : 将最短路径和其他路径整合，并按总耗时升序排序。
    4. 格式化输出 : 调用 Utils::printRoute 打印所有方案和最短方案总结。
- addLine(const Line& newLine) :
  - 作用 : 增加一条新线路。
  - 思路 : 检查线路是否已存在，如果不存在则添加到 data_.lines ，然后调用 buildGraph() 重新构建图以包含新线路。
- updateLine(const std::string& oldLineName, const Line& newLine) :
  - 作用 : 更新一条现有线路的信息。
  - 思路 : 检查旧线路是否存在，如果存在则从 data_.lines 中删除旧线路，添加新线路（可能名称已改变），然后调用 buildGraph() 重新构建图。
- displayAllInfo() const :
  - 作用 : 显示所有线路和站点的总览信息。
  - 思路 : 遍历 data_.lines 和 data_.stationNameId ，格式化打印所有线路的详细信息和站点ID映射。
### 5. main.cpp (主程序入口)
main.cpp 负责程序的启动、用户界面的交互、功能选择和调用 MetroGraph 的相应方法。

- displayMenu() :
  - 作用 : 显示主菜单，列出所有可用功能。
  - 思路 : 提供清晰的用户交互界面。
- clearInput() :
  - 作用 : 清除输入流的错误状态并忽略多余字符，防止输入错误导致程序崩溃或循环。
  - 思路 : 健壮性处理，确保用户输入不会影响后续的 cin 操作。
- handleAddLine(MetroGraph& graph) :
  - 作用 : 处理用户增加线路的输入和逻辑。
  - 思路 : 提示用户输入线路的各项信息，然后构建 Line 对象并调用 graph.addLine() 。包含异常处理，防止用户输入格式错误。
- handleUpdateLine(MetroGraph& graph) :
  - 作用 : 处理用户更新线路的输入和逻辑。
  - 思路 : 提示用户输入要更新的线路名称和新的线路信息，然后构建 Line 对象并调用 graph.updateLine() 。包含异常处理。
- main() 函数 :
  - 作用 : 程序的主入口。
  - 思路 :
    1. 数据初始化 : 创建 MetroData 对象。
    2. 数据管理器 : 创建 DataManager 对象，负责加载和保存数据。 std::make_unique 用于智能指针管理内存。
    3. 地铁图 : 创建 MetroGraph 对象，此时图结构已构建。
    4. 主循环 : do-while 循环显示菜单，根据用户选择调用 MetroGraph 的相应功能。
    5. 输入处理 : 使用 cin 获取用户选择，并调用 clearInput() 处理输入流。
    6. 功能调用 : switch 语句根据用户选择调用不同的处理函数或 MetroGraph 方法。
    7. 退出 : 当用户选择0时退出循环， DataManager 的析构函数会自动保存数据。





























扩展功能（待定）：

1. 站点间的真实距离/时间信息（目前简化为3.0）

2. 线路颜色

3. 票价计算函数（目前简化为线路全程票价）

4. 更多换乘信息 (例如：换乘通道所需时间)