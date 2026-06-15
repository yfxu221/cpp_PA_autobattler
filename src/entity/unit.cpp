#include "unit.h"
#include <cmath>

QColor traitColor(const QString& trait) {
    static QHash<QString, QColor> colorMap = {
        {"人", QColor(80, 200, 80)},
        {"神", QColor(255, 180, 40)},
        {"区", QColor(180, 100, 255)}
    };
    return colorMap.value(trait, QColor(128, 128, 128)); // 默认灰色
}

int Unit::s_nextId = 0; // 静态成员变量初始化，确保每个单位都有一个唯一的ID，从0开始递增

Unit::Unit(const QString& name, int maxHp, int atk, int range, int maxMana, int starLevel, int speed, const QSet<QString>& traits, Owner owner, const QString& spritePath, int attackCooldown, int price)
    : m_id(s_nextId++)
    , m_name(name)
    , m_position(0, 0)
    , m_maxHp(maxHp)
    , m_hp(maxHp)
    , m_atk(atk)
    , m_range(range)
    , m_maxMana(maxMana)
    , m_mana(0)
    , m_owner(owner)
    , m_speed(speed)
    , m_state(UnitState::Idle)
    , m_starLevel(starLevel)
    , m_traits(traits)
    , m_spritePath(spritePath)
    , m_attackCooldown(attackCooldown)
    , m_price(price)
{}

int Unit::atk() const
{
    return m_atk + m_bonusAtk;
}

void Unit::processCooldown()
{
    if (m_currentCooldown > 0) {
        --m_currentCooldown;
    }
}

void Unit::resetCooldown()
{
    m_currentCooldown = m_attackCooldown;
}

int Unit::price() const
{
    return m_price * std::pow(3, m_starLevel - 1) - m_starLevel + 1; // 价格随星级翻倍
}

void Unit::addSkill(std::shared_ptr<Skill> skill)
{
    m_skill = skill;
}

void Unit::applySynergyBonuses(const SynergyBonus& bonus) {
    // HP: 按差值调整，保证不会因为羁绊变化导致血量百分比突变
    int hpDelta = bonus.bonusMaxHp - m_bonusMaxHp;
    
    m_bonusAtk = bonus.bonusAtk;
    m_bonusMaxHp = bonus.bonusMaxHp;
    m_bonusMaxMana = bonus.bonusMaxMana;
    m_bonusSpeed = bonus.bonusSpeed;
    
    if (hpDelta != 0) {
        m_hp = std::clamp(m_hp + hpDelta, 0, maxHp());
    }
    m_mana = std::clamp(m_mana, 0, maxMana());
}

void Unit::clearSynergyBonuses() {
    m_bonusAtk = 0;
    m_bonusMaxHp = 0;
    m_bonusMaxMana = 0;
    m_bonusSpeed = 0;
    m_hp = std::clamp(m_hp, 0, maxHp());
    m_mana = std::clamp(m_mana, 0, maxMana());
}