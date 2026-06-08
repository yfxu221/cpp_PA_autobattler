#include "unit.h"

QColor traitColor(const QString& trait) {
    static QHash<QString, QColor> colorMap = {
        {"人", QColor(80, 200, 80)},
        {"神", QColor(255, 180, 40)},
        {"区", QColor(180, 100, 255)}
    };
    return colorMap.value(trait, QColor(128, 128, 128)); // 默认灰色
}

int Unit::s_nextId = 0; // 静态成员变量初始化，确保每个单位都有一个唯一的ID，从0开始递增

Unit::Unit(const QString& name, int maxHp, int atk, int range, int maxMana, int starLevel, const QSet<QString>& traits, Owner owner, const QString& spritePath)
    : m_id(s_nextId++)
    , m_name(name)
    , m_position(0, 0)
    , m_maxHp(maxHp)
    , m_hp(maxHp)
    , m_atk(atk)
    , m_range(range)
    , m_maxMana(maxMana)
    , m_mana(maxMana)
    , m_owner(owner)
    , m_starLevel(starLevel)
    , m_traits(traits)
    , m_spritePath(spritePath)
{}

int Unit::atk() const
{
    if (m_maxMana != 0 && m_mana > 0) {
        return m_atk*2 + 5;
    }
    return m_atk;
}