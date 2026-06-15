#ifndef ENTITY_EQUIPMENTDATA_H
#define ENTITY_EQUIPMENTDATA_H

#include <QHash>
#include <QList>
#include <QString>
#include <memory>
#include "equipment.h"

class EquipmentRegistry {
public:
    static EquipmentRegistry* instance();

    // 从 JSON 文件加载所有装备定义
    bool load(const QString& jsonPath);

    // 按 key 查找装备模板（返回 nullptr 表示未找到）
    const Equipment* get(const QString& key) const;

    // 根据模板 key 创建一个堆上的装备实例
    std::shared_ptr<Equipment> createEquipment(const QString& key) const;

    // 返回所有装备 key
    QList<QString> allKeys() const;

private:
    EquipmentRegistry() = default;
    QHash<QString, Equipment> m_templates;
};

#endif // ENTITY_EQUIPMENTDATA_H
