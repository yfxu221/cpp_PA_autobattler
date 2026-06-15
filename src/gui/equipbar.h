#ifndef GUI_EQUIPBAR_H
#define GUI_EQUIPBAR_H

#include <QGraphicsScene>
#include <QPointF>
#include <memory>
#include <vector>
#include "entity/equipment.h"

class EquipSlotItem;

// 装备栏
class EquipBar
{
public:
    EquipBar() = default;
    ~EquipBar();

    // topLeft:  第一个槽位的左上角（场景坐标）
    // slotCount: 槽位数量
    // slotSize:  每个槽位的边长（正方形）
    // spacing:   槽位之间的间距
    void buildBar(QGraphicsScene* scene, const QPointF& topLeft,
                  int slotCount, double slotSize, double spacing);

    // 槽位访问
    int slotCount() const { return static_cast<int>(m_slots.size()); }
    EquipSlotItem* slot(int index) const;

    // 装备管理
    bool addEquipment(std::shared_ptr<Equipment> eq); // 放入第一个空槽位
    bool setEquipment(int index, std::shared_ptr<Equipment> eq); // 覆盖指定槽位
    std::shared_ptr<Equipment> removeEquipment(int index); // 取出并清空槽位
    int findEmptySlot() const; // 返回 -1 表示无空位
    bool hasEmptySlot() const { return findEmptySlot() >= 0; }

    void refreshDisplay(); // 全量刷新（装备实例指针变化后调用）

    void clear(); // 清空所有槽位

private:
    QGraphicsScene* m_scene = nullptr;
    std::vector<EquipSlotItem*> m_slots;
};

#endif // GUI_EQUIPBAR_H
