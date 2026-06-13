#ifndef STORE_H
#define STORE_H

#include <QList>
#include <memory>
#include <vector>
#include "entity/unitdata.h"
#include "entity/unit.h"
#include <QGraphicsScene>
#include "gui/storeslotitem.h"

class StoreRefreshButton;

class Store
{
public:
    static constexpr int STORE_SIZE = 5;

    Store();
    ~Store() = default;

    void addUnit(std::unique_ptr<Unit> unit, int index); // 将单位添加到商店的指定格子（接管所有权），index范围为0到STORE_SIZE-1
    void removeUnit(int index); // 从商店中移除并销毁指定格子的单位
    void refresh(); // 刷新商店，销毁当前所有单位
    std::unique_ptr<Unit> buyUnit(int index); // 购买商店中指定格子的单位（交出所有权），如果购买失败则返回nullptr
    bool hasUnitAt(int index) const; // 检查商店中指定格子是否有单位，返回true表示有单位，false表示没有单位
    int level() const { return m_level; } // 获取商店当前的等级，等级可能影响商店中出现的单位质量和种类
    void setLevel(int level) { m_level = level; } // 设置商店的等级，通常在游戏进程中根据玩家的表现或阶段提升商店等级
    Unit* getUnitAt(int index) const; // 获取商店中指定格子的单位指针（只读观察），如果索引无效或该格子没有单位则返回nullptr
    void buildDisplay(QGraphicsScene* scene, double radius); // 构建商店的图形界面，将商店格子和单位显示在场景中
    StoreSlotItem* slotItem(int index) const; // 获取商店中指定格子的图形项指针，方便在界面上更新该格子的显示状态
    StoreRefreshButton* refreshButton() const { return m_refreshButton; } // 获取刷新按钮的指针，方便连接信号
    void refreshDisplay(); // 刷新商店的图形界面

private:
    std::vector<std::unique_ptr<Unit>> m_cells; // 商店格子，拥有格子中单位的所有权，nullptr表示该格子没有单位
    QHash<Unit*, int> m_unitToIndex; // 单位 -> 商店格子索引的映射，方便快速查找单位在商店中的位置
    int m_level;
    QVector<StoreSlotItem*> m_slotItems; // 商店格子的图形项指针列表，用于在界面上更新格子的显示状态
    StoreRefreshButton* m_refreshButton = nullptr; // 刷新按钮，Store 拥有管理权
};

#endif // STORE_H