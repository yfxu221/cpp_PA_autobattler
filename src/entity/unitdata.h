#ifndef UNITDATA_H
#define UNITDATA_H

#include <QHash>
#include <QSet>
#include <QList>
#include <QString>
#include <memory>
#include "unit.h"

struct UnitTemplate {  // JSON 反序列化后的结构体
    QString key, name;
    int maxHp, atk, range, maxMana, attackCooldown, speed, price;
    QSet<QString> traits;
    QString sprite;
};

class UnitData { // 单位数据管理类，负责加载单位数据、创建单位实例等功能
public:
    static UnitData* instance();
    bool load(const QString& jsonPath);
    std::unique_ptr<Unit> createUnit(const QString& key, Owner owner, int starLevel = 1) const;
    QList<QString> allKeys() const;

private:
    QHash<QString, UnitTemplate> m_templates;
};

#endif // UNITDATA_H