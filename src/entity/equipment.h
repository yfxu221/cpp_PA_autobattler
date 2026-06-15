#ifndef ENTITY_EQUIPMENT_H
#define ENTITY_EQUIPMENT_H

#include <QString>

// 装备数据类，纯值对象
struct Equipment {
    QString key; // 装备模板标识符
    QString name; // 显示名称
    int bonusAtk = 0; // 攻击力加成
    int bonusMaxHp = 0; // 最大生命值加成
    int bonusMaxMana = 0; // 最大法力值加成
    int bonusSpeed = 0; // 速度加成
    QString spritePath; // 装备图标路径（为空时绘制文字回退）
};

#endif // ENTITY_EQUIPMENT_H
