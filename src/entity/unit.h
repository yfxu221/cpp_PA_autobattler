#ifndef UNIT_H
#define UNIT_H

#include <QPoint>
#include <QString>
#include<QSet>
#include <algorithm>

enum Owner{
    PlayerCtrl,
    EnemyCtrl
};

class Unit // 代表一个单位的类，包含单位的ID、名称和位置等属性
{
public:
    explicit Unit(const QString& name = QString("Unit"),
                int maxHp = 100,
                int atk = 10,
                int range = 1,
                int maxMana = 100,
                int starLevel = 1,
                const QSet<QString>& traits = {},
                Owner owner = PlayerCtrl,
                const QString& spritePath = QString());
    ~Unit() = default;

    int id() const { return m_id; } // 获取单位的唯一ID，单位ID在创建时自动分配，确保每个单位都有一个独特的标识符
    QString name() const { return m_name; } // 获取单位的名称
    QPoint position() const { return m_position; } // 获取单位在棋盘上的位置，使用网格坐标表示（列, 行）

    void setName(const QString& name) { m_name = name; }
    void setPosition(const QPoint& pos) { m_position = pos; }

    bool isAlive() const { return m_hp > 0; } // 判断单位是否存活
    bool hasTrait(const QString& trait) const { return m_traits.contains(trait); } // 检查单位是否具有特定的羁绊
    void takeDamage(int damage) { m_hp = std::max(0, m_hp - damage); } // 受到伤害，减少生命值，但不低于0
    Owner owner() const { return m_owner; } // 获取单位的所有者，表示是玩家控制还是敌人控制
    int maxHp() const { return m_maxHp; } // 获取单位的最大生命值
    int hp() const { return m_hp; } // 获取单位当前的生命值
    int atk() const; // 获取单位的攻击力
    int range() const { return m_range; } // 获取单位的攻击范围
    int maxMana() const { return m_maxMana; } // 获取单位的最大法力值
    int mana() const { return m_mana; } // 获取单位当前的法力值
    int starLevel() const { return m_starLevel; } // 获取单位的星级
    const QSet<QString>& traits() const { return m_traits; } // 获取单位的羁绊集合
    QString spritePath() const { return m_spritePath; } // 获取单位的精灵图片路径

    void setHp(int hp) { m_hp = std::clamp(hp, 0, m_maxHp); } // 设置单位的生命值，确保不超过最大生命值且不低于0
    void setMana(int mana) { m_mana = std::clamp(mana, 0, m_maxMana); } // 设置单位的法力值，确保不超过最大

private:
    static int s_nextId; // 静态成员变量，用于生成唯一的单位ID，每创建一个单位，s_nextId就会递增，确保每个单位都有一个独特的ID

    int m_id; // 单位的唯一ID，由s_nextId自动分配
    QString m_name; // 单位的名称，可以在创建单位时指定，也可以后续修改
    QPoint m_position; // 单位在棋盘上的位置，使用网格坐标表示（列, 行）
    int m_maxHp; // 单位的最大生命值
    int m_hp; // 单位当前的生命值
    int m_atk; // 单位的攻击力
    int m_range; // 单位的攻击范围
    int m_maxMana; // 单位的最大法力值
    int m_mana; // 单位当前的法力值
    Owner m_owner; // 单位的所有者，表示是玩家控制还是敌人控制
    QSet<QString> m_traits; //单位羁绊
    int m_starLevel; //单位星级
    QString m_spritePath; // 精灵图片路径

};

#endif // UNIT_H
