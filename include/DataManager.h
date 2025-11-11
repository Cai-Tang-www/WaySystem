#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <string>
#include "Structures.h" // 确保引用了自定义结构体

// DataManager 类负责所有数据的持久化操作：从文件加载和保存到文件。
class DataManager {
public:
    // 构造函数：初始化文件路径和数据引用，并自动加载数据
    DataManager(const std::string& filePath, MetroData& data);

    // 析构函数：在对象销毁时自动保存数据
    ~DataManager();

    // 从文件读取数据到内存 (无参数，使用成员变量 filePath_ 和 data_)
    void loadData();

    // 将内存数据保存到文件 (无参数，使用成员变量 filePath_ 和 data_)
    void saveData() const;

private:
    std::string filePath_; // 数据文件的路径
    MetroData& data_;      // 对主数据结构的引用

};

#endif // DATAMANAGER_H