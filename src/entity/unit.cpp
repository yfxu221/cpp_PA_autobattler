#include "unit.h"
#include <cmath>

QColor traitColor(const QString& trait) {
    static QHash<QString, QColor> colorMap = {
        {"人", QColor(80, 200, 80)},
        {"神", QColor(255, 180, 40)},
        {"区", QColor(180, 100, 255)},
        {"dot女子", QColor(255, 100, 100)}
    };
    return colorMap.value(trait, QColor(128, 128, 128)); // 默认灰色
}

int Unit::s_nextId = 0; // 静态成员变量初始化，确保每个单位都有一个唯一的ID，从0开始递增
int Unit::s_nextBuffInstanceId = 0; // Buff 实例 ID 分配器

Unit::Unit(const QString& key, const QString& name, int maxHp, int atk, int range, int maxMana, int starLevel, int speed, const QSet<QString>& traits, Owner owner, const QString& spritePath, const QString& type, int attackCooldown, int moveCooldown, int price)
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
    , m_moveCooldown(moveCooldown)
    , m_price(price)
    , m_maxEquipSlots(starLevel == 1 ? 1 : 2)
{
}

int Unit::atk() const
{
    const int base = static_cast<int>(m_baseAtk * starMultiplier(m_starLevel));
    const int bonus = m_bonusAtk + m_equipBonusAtk;
    const float buffMod = statModSum(BuffStat::ATK);
    return static_cast<int>(base + bonus + buffMod);
}

int Unit::maxHp() const
{
    return static_cast<int>(m_baseMaxHp * starMultiplier(m_starLevel))
           + m_bonusMaxHp + m_equipBonusMaxHp;
}

int Unit::maxMana() const
{
    const int base = m_maxMana + m_bonusMaxMana + m_equipBonusMaxMana;
    const float buffMod = statModSum(BuffStat::MaxMana);
    return std::max(5, static_cast<int>(base + buffMod));
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
    m_currentCooldown = effectiveAttackCooldown();
}

int Unit::effectiveAttackCooldown() const
{
    // 基础冷却 / (1 + 装备攻速加成)，至少为 1
    int raw = static_cast<int>(m_attackCooldown / (1.0f + m_equipAttackSpeedMod));
    return std::max(1, raw);
}

// 移动冷却
void Unit::processMoveCooldown()
{
    if (m_currentMoveCooldown > 0) {
        --m_currentMoveCooldown;
    }
}

void Unit::resetMoveCooldown()
{
    m_currentMoveCooldown = effectiveMoveCooldown();
}

int Unit::effectiveMoveCooldown() const
{
    // 基础冷却 / (1 + 装备移速加成)，至少为 1
    int raw = static_cast<int>(m_moveCooldown / (1.0f + m_equipMoveSpeedMod));
    return std::max(1, raw);
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

    // 装备百分比加成求和
    m_equipAttackSpeedMod = 0.0f;
    m_equipMoveSpeedMod = 0.0f;
    for (const auto& eq : m_equipments) {
        if (!eq) continue;
        m_equipAttackSpeedMod += eq->bonusAttackSpeed;
        m_equipMoveSpeedMod += eq->bonusMoveSpeed;
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


// Buff / 状态效果系统
float Unit::statModSum(BuffStat stat) const
{
    float sum = 0.0f;
    for (const auto& buff : m_buffs) {
        const BuffDef* def = BuffRegistry::instance()->get(buff.buffKey);
        if (def && def->category == BuffCategory::StatMod && def->stat == stat) {
            sum += buff.magnitude;
        }
    }
    return sum;
}

void Unit::addBuff(const BuffInstance& buff)
{
    const BuffDef* def = BuffRegistry::instance()->get(buff.buffKey);
    if (!def) return;

    switch (def->stackRule) {

    case BuffStackRule::Refresh:
        // 同 key → 刷新时间 + 取绝对值最大的效果
        for (auto& existing : m_buffs) {
            if (existing.buffKey == buff.buffKey) {
                const int newRemaining = std::max(existing.remainingTicks, buff.totalTicks);
                if (newRemaining > existing.remainingTicks)
                    existing.totalTicks = buff.totalTicks;
                existing.remainingTicks = newRemaining;
                if (std::abs(buff.magnitude) > std::abs(existing.magnitude))
                    existing.magnitude = buff.magnitude;
                existing.damageInterval = buff.damageInterval;
                return;
            }
        }
        break;

    case BuffStackRule::UniquePerSource:
        // 同 source + 同 key → Refresh；异 source → 独立实例
        for (auto& existing : m_buffs) {
            if (existing.buffKey == buff.buffKey && existing.sourceUnitId == buff.sourceUnitId) {
                const int newRemaining = std::max(existing.remainingTicks, buff.totalTicks);
                if (newRemaining > existing.remainingTicks)
                    existing.totalTicks = buff.totalTicks;
                existing.remainingTicks = newRemaining;
                if (std::abs(buff.magnitude) > std::abs(existing.magnitude))
                    existing.magnitude = buff.magnitude;
                existing.damageInterval = buff.damageInterval;
                return;
            }
        }
        break;

    case BuffStackRule::Independent:
        // 总是新增实例
        break;
    }

    // 未命中任何合并规则 → 添加新实例
    BuffInstance newBuff = buff;
    newBuff.instanceId = s_nextBuffInstanceId++;
    m_buffs.push_back(newBuff);
}

int Unit::processBuffsPreAction()
{
    int totalDotDamage = 0;

    for (auto& buff : m_buffs) {
        const BuffDef* def = BuffRegistry::instance()->get(buff.buffKey);

        // DoT 类：按间隔造成伤害
        if (def && def->category == BuffCategory::Dot) {
            if (buff.damageIntervalCounter <= 0) {
                totalDotDamage += static_cast<int>(buff.magnitude);
                buff.damageIntervalCounter = buff.damageInterval;
            }
            buff.damageIntervalCounter--;
        }

        // 所有 buff：持续时间递减
        buff.remainingTicks--;
    }

    return totalDotDamage;
}

void Unit::removeExpiredBuffs()
{
    m_buffs.erase(
        std::remove_if(m_buffs.begin(), m_buffs.end(),
                       [](const BuffInstance& b) { return b.remainingTicks <= 0; }),
        m_buffs.end());

    // buff 移除可能导致 maxMana 下降，重新 clamp 法力值
    m_mana = std::clamp(m_mana, 0, maxMana());
}

void Unit::clearBuffs()
{
    m_buffs.clear();
}

bool Unit::isDisabled() const
{
    for (const auto& buff : m_buffs) {
        const BuffDef* def = BuffRegistry::instance()->get(buff.buffKey);
        if (def && def->category == BuffCategory::Control) {
            return true;
        }
    }
    return false;
}