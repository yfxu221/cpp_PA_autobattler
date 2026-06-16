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

Unit::Unit(const QString& key, const QString& name, int maxHp, int atk, int range, int maxMana, int starLevel, int speed, const QSet<QString>& traits, Owner owner, const QString& spritePath, const QString& type, int attackCooldown, int price)
    : m_id(s_nextId++)
    , m_key(key)
    , m_name(name)
    , m_position(0, 0)
    , m_baseMaxHp(maxHp)
    , m_hp(static_cast<int>(maxHp * starMultiplier(starLevel)))
    , m_baseAtk(atk)
    , m_range(range)
    , m_maxMana(maxMana)
    , m_mana(0)
    , m_type(type)
    , m_owner(owner)
    , m_speed(speed)
    , m_state(UnitState::Idle)
    , m_starLevel(starLevel)
    , m_traits(traits)
    , m_spritePath(spritePath)
    , m_attackCooldown(attackCooldown)
    , m_price(price)
    , m_maxEquipSlots(starLevel == 1 ? 1 : 2)
{
}

int Unit::atk() const
{
    return static_cast<int>(m_baseAtk * starMultiplier(m_starLevel)) + m_bonusAtk + m_equipBonusAtk;
}


double Unit::starMultiplier(int starLevel)
{
    // 1星=1.0, 2星=1.8, 3星=1.8^2=3.24
    return std::pow(1.8, starLevel - 1);
}

void Unit::upgradeStar()
{
    if (m_starLevel >= 3) return;

    double hpRatio = static_cast<double>(m_hp) / maxHp();
    m_starLevel++;
    m_hp = std::clamp(static_cast<int>(maxHp() * hpRatio), 1, maxHp());
    m_maxEquipSlots = 2; // 升星后至少2星，2个装备槽
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
    // HP: 按差值调整
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

void Unit::recalcEquipBonuses()
{
    // 记录旧值，用于计算增量
    const int oldEquipBonusHp = m_equipBonusMaxHp;

    m_equipBonusAtk = 0;
    m_equipBonusMaxHp = 0;
    m_equipBonusMaxMana = 0;
    m_equipBonusSpeed = 0;

    for (const auto& eq : m_equipments) {
        if (!eq) continue;
        m_equipBonusAtk += eq->bonusAtk;
        m_equipBonusMaxHp += eq->bonusMaxHp;
        m_equipBonusMaxMana += eq->bonusMaxMana;
        m_equipBonusSpeed += eq->bonusSpeed;
    }

    // 按差值调整 HP
    const int hpDelta = m_equipBonusMaxHp - oldEquipBonusHp;
    if (hpDelta != 0) {
        m_hp = std::clamp(m_hp + hpDelta, 0, maxHp());
    }
    m_mana = std::clamp(m_mana, 0, maxMana());
}

bool Unit::equip(std::shared_ptr<Equipment> eq)
{
    if (!eq) return false;
    if (!canEquip()) return false;

    m_equipments.push_back(std::move(eq));
    recalcEquipBonuses();
    return true;
}

std::shared_ptr<Equipment> Unit::unequip(int index)
{
    if (index < 0 || index >= static_cast<int>(m_equipments.size()))
        return nullptr;

    auto removed = m_equipments[index];
    m_equipments.erase(m_equipments.begin() + index);
    recalcEquipBonuses();
    return removed;
}