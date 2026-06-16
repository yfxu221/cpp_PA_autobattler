#ifndef UNIT_H
#define UNIT_H

#include <QPoint>
#include <QString>
#include<QSet>
#include <algorithm>
#include <memory>
#include <QColor>
#include "skill.h"
#include "synergyregistry.h"
#include "equipment.h"
#include <vector>

enum Owner{
    PlayerCtrl,
    EnemyCtrl
};

enum UnitState{
    Idle, // 空闲状态
    Moving, // 移动状态
    Attacking, // 攻击状态
    Casting, // 施法状态
    Dead // 死亡状态
};

QColor traitColor(const QString& trait); // 根据羁绊名称返回对应的颜色

class Unit // 代表一个单位的类，包含单位的ID、名称和位置等属性
{
public:
    explicit Unit(const QString& key = QString(),
                const QString& name = QString("Unit"),
                int maxHp = 100,
                int atk = 10,
                int range = 1,
                int maxMana = 100,
                int starLevel = 1,
                int speed = 1,
                const QSet<QString>& traits = {},
                Owner owner = PlayerCtrl,
                const QString& spritePath = QString(),
                const QString& type = QString(),
                int attackCooldown = 2,
                int price = 100
                );
    ~Unit() = default;

    int id() const { return m_id; } // 获取单位的唯一ID，单位ID在创建时自动分配，确保每个单位都有一个独特的标识符
    QString key() const { return m_key; } // 获取单位的模板key，用于识别同类单位（升星合并的依据）
    QString name() const { return m_name; } // 获取单位的名称
    QPoint position() const { return m_position; } // 获取单位在棋盘上的位置，使用网格坐标表示（列, 行）

    void setName(const QString& name) { m_name = name; }
    void setPosition(const QPoint& pos) { m_position = pos; }

    bool isAlive() const { return m_hp > 0; } // 判断单位是否存活
    bool hasTrait(const QString& trait) const { return m_traits.contains(trait); } // 检查单位是否具有特定的羁绊
    void takeDamage(int damage) { m_hp = std::clamp(m_hp - damage, 0, maxHp()); } // 受到伤害，减少生命值，但不低于0
    Owner owner() const { return m_owner; } // 获取单位的所有者，表示是玩家控制还是敌人控制
    int maxHp() const { return static_cast<int>(m_baseMaxHp * starMultiplier(m_starLevel)) + m_bonusMaxHp + m_equipBonusMaxHp; } // 获取单位的最大生命值（随星级缩放）
    int hp() const { return m_hp; } // 获取单位当前的生命值
    int atk() const; // 获取单位的攻击力
    int range() const { return m_range; } // 获取单位的攻击范围
    int maxMana() const { return std::max(5, m_maxMana + m_bonusMaxMana + m_equipBonusMaxMana); } // 获取单位的最大法力值（最低为5）
    int mana() const { return m_mana; } // 获取单位当前的法力值
    int starLevel() const { return m_starLevel; } // 获取单位的星级
    const QSet<QString>& traits() const { return m_traits; } // 获取单位的羁绊集合
    QString spritePath() const { return m_spritePath; } // 获取单位的精灵图片路径
    UnitState state() const { return m_state; } // 获取单位的当前状态
    void setState(UnitState state) { m_state = state; } // 设置单位的当前状态
    QString type() const { return m_type; } // 获取单位的类型，用于生成敌人时的启发式阵型选择

    void setHp(int hp) { m_hp = std::clamp(hp, 0, maxHp()); } // 设置单位的生命值，确保不超过最大生命值且不低于0
    void setMana(int mana) { m_mana = std::clamp(mana, 0, maxMana()); } // 设置单位的法力值，确保不超过最大
    void processCooldown(); // 每tick调用：若冷却中则递减1（纯计数，不碰状态）
    void resetCooldown();   // 攻击后调用：将冷却重置为最大值
    bool isCooldownReady() const { return m_currentCooldown <= 0; } // 检查单位是否准备好进行下一次攻击
    int speed() const { return m_speed + m_bonusSpeed + m_equipBonusSpeed; } // 获取单位的速度
    int price() const; // 获取单位的价格

    std::unique_ptr<Unit> clone() const { return std::make_unique<Unit>(*this); }

    void addSkill(std::shared_ptr<Skill> skill); // 添加技能
    const std::shared_ptr<Skill>& skill() const {return m_skill;} // 获取单位的技能
    bool hasSkill() const { return m_skill != nullptr; } // 检查单位是否拥有技能
    bool canUseSkill() const {return hasSkill() && m_mana >= maxMana();} // 检查单位是否可以使用技能（拥有技能且法力值足够）

    void upgradeStar(); // 升星：starLevel+1，按比例调整血量
    static double starMultiplier(int starLevel); // 星级属性倍率：1星=1.0, 2星=1.8, 3星=3.24

    void applySynergyBonuses(const SynergyBonus& bonus); // 设置羁绊加成
    void clearSynergyBonuses(); // 清除羁绊加成

    // 装备系统
    bool equip(std::shared_ptr<Equipment> eq); // 装备一件装备，失败返回 false
    std::shared_ptr<Equipment> unequip(int index); // 卸下指定槽位的装备
    const std::vector<std::shared_ptr<Equipment>>& equipments() const { return m_equipments; }
    int equipmentCount() const { return static_cast<int>(m_equipments.size()); }
    int maxEquipSlots() const { return m_maxEquipSlots; }
    bool canEquip() const { return equipmentCount() < m_maxEquipSlots; }
    void recalcEquipBonuses(); // 重新计算装备加成总和

private:
    static int s_nextId; // 静态成员变量，用于生成唯一的单位ID，每创建一个单位，s_nextId就会递增，确保每个单位都有一个独特的ID

    int m_id; // 单位的唯一ID，由s_nextId自动分配
    QString m_key; // 单位的模板key，用于识别同类单位（升星合并的依据）
    QString m_name; // 单位的名称，可以在创建单位时指定，也可以后续修改
    QPoint m_position; // 单位在棋盘上的位置，使用网格坐标表示（列, 行）
    int m_baseMaxHp; // 单位1星时的基础最大生命值（实际maxHp = baseMaxHp * starMultiplier(starLevel) + bonusMaxHp）
    int m_hp; // 单位当前的生命值
    int m_baseAtk; // 单位1星时的基础攻击力（实际atk = baseAtk * starMultiplier(starLevel) + bonusAtk）
    int m_range; // 单位的攻击范围
    int m_maxMana; // 单位的最大法力值
    int m_mana; // 单位当前的法力值
    Owner m_owner; // 单位的所有者，表示是玩家控制还是敌人控制
    QSet<QString> m_traits; //单位羁绊
    int m_starLevel; //单位星级
    QString m_spritePath; // 精灵图片路径
    QString m_type; // 单位类型，用于生成敌人时的启发式阵型选择
    UnitState m_state; // 单位的当前状态
    int m_attackCooldown; // 记录攻击冷却时间，单位为tick数
    int m_currentCooldown = 0; // 当前剩余的冷却时间，单位为tick数
    int m_speed; //单位速度，影响移动优先级
    int m_price; //单位价格
    std::shared_ptr<Skill> m_skill; // 单位的技能，暂时假设每个单位只有一个技能
    int m_bonusAtk = 0; // 来自羁绊的攻击力加成
    int m_bonusMaxHp = 0; // 来自羁绊的最大生命值加成
    int m_bonusMaxMana = 0; // 来自羁绊的最大法力值加成
    int m_bonusSpeed = 0; // 来自羁绊的速度加成

    // 装备系统
    int m_maxEquipSlots = 1;           // 每个单位最多装备数量（1星=1，≥2星=2）
    std::vector<std::shared_ptr<Equipment>> m_equipments; // 当前装备列表
    int m_equipBonusAtk = 0; // 来自装备的攻击力加成
    int m_equipBonusMaxHp = 0; // 来自装备的最大生命值加成
    int m_equipBonusMaxMana = 0;// 来自装备的最大法力值加成
    int m_equipBonusSpeed = 0; // 来自装备的速度加成

};

#endif // UNIT_H
