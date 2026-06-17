#include "SingleTargetedSkill.h"
#include "../unit.h"
#include <algorithm>
#include <stdexcept>
#include "core/pathfinder.h"


namespace {

TargetType stringToTargetType(const QString& s) {
    if (s == "Enemy") return TargetType::Enemy;
    if (s == "Ally") return TargetType::Ally;
    if (s == "Self") return TargetType::Self;
    throw std::invalid_argument("Unknown TargetType: " + s.toStdString());
}

SelectType stringToSelectType(const QString& s) {
    if (s == "Nearest") return SelectType::Nearest;
    if (s == "LowestHp") return SelectType::LowestHp;
    if (s == "HighestAtk") return SelectType::HighestAtk;
    if (s.isEmpty()) return SelectType::Nearest;
    throw std::invalid_argument("Unknown SelectType: " + s.toStdString());
}

ValueType stringToValueType(const QString& s) {
    if (s == "AtkRatio") return ValueType::AtkRatio;
    if (s == "HpRatio") return ValueType::HpRatio;
    if (s == "fixed") return ValueType::fixed;
    if (s == "split") return ValueType::split;
    if (s == "splash") return ValueType::splash;
    throw std::invalid_argument("Unknown ValueType: " + s.toStdString());
}

}

SingleTargetedParams SingleTargetedParams::fromJson(const QJsonObject& json) {
    SingleTargetedParams p;
    p.name = json["name"].toString();
    p.targetType = stringToTargetType(json["targetType"].toString());
    p.selectType = stringToSelectType(json["selectType"].toString());
    p.valueType = stringToValueType(json["valueType"].toString());
    p.castRange = json["castRange"].toInt(1);
    p.value = json["value"].toInt(0);
    p.ratio = json["ratio"].toDouble(1.0);
    return p;
}

SingleTargetedSkill::SingleTargetedSkill(const SingleTargetedParams& p)
    : m_p(p) {}

SingleTargetedSkill::~SingleTargetedSkill() = default;


QVector<TargetInfo> SingleTargetedSkill::selectTargets(const Unit& caster, const QVector<Unit*>& allUnits) const {
    QVector<Unit*> UnitInRange, candidates;
    // 首先筛选出施法范围内的单位
    std::copy_if(allUnits.begin(), allUnits.end(), std::back_inserter(UnitInRange),
                 [&](Unit* u) {
                     return Pathfinder::hexDistance(caster.position(), u->position()) <= m_p.castRange;
                 });
    
    // 根据目标类型筛选候选目标
    switch (m_p.targetType) {
    case TargetType::Enemy:
        std::copy_if(UnitInRange.begin(), UnitInRange.end(), std::back_inserter(candidates),
                     [&](Unit* u) { return u->owner() != caster.owner(); });
        break;
    case TargetType::Ally:
        std::copy_if(UnitInRange.begin(), UnitInRange.end(), std::back_inserter(candidates),
                     [&](Unit* u) { return u->owner() == caster.owner() && u != &caster; });
        break;
    case TargetType::Self:
        candidates.push_back(const_cast<Unit*>(&caster));
        break;
    }

    if (candidates.empty())
        return {};


    // 根据筛选方式选择目标
    switch (m_p.selectType) {
    case SelectType::LowestHp:
        return {{ *std::min_element(candidates.begin(), candidates.end(),
            [this, &caster](Unit* a, Unit* b) {
                return hpLess(caster, a, b) == a;
            }), true }};
    case SelectType::Nearest:
        return {{ *std::min_element(candidates.begin(), candidates.end(),
            [this, &caster](Unit* a, Unit* b) {
                return distanceLess(caster, a, b) == a;
            }), true }};
    case SelectType::HighestAtk:
        return {{ *std::max_element(candidates.begin(), candidates.end(),
            [this, &caster](Unit* a, Unit* b) {
                return attackGreater(caster, a, b) == b;
            }), true }};
    }
    return {};
}

int SingleTargetedSkill::calculateValue(
    const Unit& caster, const Unit& target, bool isPrimary, int totalTargets) const
{
    Q_UNUSED(isPrimary); // 单目标技能不需要区分主次目标
    Q_UNUSED(totalTargets); // 单目标技能不需要总目标数

    int baseValue = 0;
    switch (m_p.valueType) {
    case ValueType::AtkRatio:
        baseValue = static_cast<int>(caster.atk() * m_p.ratio);
        break;
    case ValueType::HpRatio:
        baseValue = static_cast<int>(target.maxHp() * m_p.ratio);
        break;
    case ValueType::fixed:
        baseValue = m_p.value;
        break;
    case ValueType::split:
    case ValueType::splash:
        throw std::invalid_argument(
            "SingleTargetedSkill: ValueType::split/splash not supported. "
            "Use an Area skill subclass instead.");
    }

    // 敌方 → 伤害（负数），友方/自身 → 治疗（正数）
    return (m_p.targetType == TargetType::Enemy) ? -baseValue : baseValue;
}
