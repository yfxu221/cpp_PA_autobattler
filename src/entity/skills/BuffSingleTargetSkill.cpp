#include "BuffSingleTargetSkill.h"
#include "../unit.h"
#include "core/pathfinder.h"

#include <algorithm>

#include <QJsonArray>
#include <QJsonObject>


namespace {

TargetType stringToTargetType(const QString& s) {
    if (s == "Enemy") return TargetType::Enemy;
    if (s == "Ally") return TargetType::Ally;
    if (s == "Self") return TargetType::Self;
    return TargetType::Enemy;
}

SelectType stringToSelectType(const QString& s) {
    if (s == "Nearest") return SelectType::Nearest;
    if (s == "LowestHp") return SelectType::LowestHp;
    if (s == "HighestAtk") return SelectType::HighestAtk;
    return SelectType::Nearest;
}

ValueType stringToValueType(const QString& s) {
    if (s == "AtkRatio") return ValueType::AtkRatio;
    if (s == "HpRatio") return ValueType::HpRatio;
    if (s == "fixed") return ValueType::fixed;
    return ValueType::fixed;
}

// 根据 ValueType 计算 buff 的数值（仅在施放时计算一次，之后不变）
float calculateBuffMagnitude(const Unit& caster, const Unit& target,
                              ValueType vt, double ratio, int fixedVal) {
    switch (vt) {
    case ValueType::AtkRatio:
        return static_cast<float>(caster.atk() * ratio);
    case ValueType::HpRatio:
        return static_cast<float>(target.maxHp() * ratio);
    case ValueType::fixed:
        return static_cast<float>(fixedVal);
    default:
        return 0.0f;
    }
}

} // namespace


BuffSkillParam BuffSkillParam::fromJson(const QJsonObject& json) {
    BuffSkillParam p;
    p.buffKey = json["buffKey"].toString();
    p.duration = json["duration"].toInt(0);
    p.damageInterval = json["damageInterval"].toInt(6);
    p.valueType = stringToValueType(json["valueType"].toString());
    p.ratio = json["ratio"].toDouble(0.0);
    p.fixedValue = json["value"].toInt(0);
    return p;
}

BuffSingleTargetParams BuffSingleTargetParams::fromJson(
    const QJsonObject& params, const QJsonArray& buffsArray) {
    BuffSingleTargetParams p;
    p.name = params["name"].toString();
    p.targetType = stringToTargetType(params["targetType"].toString());
    p.selectType = stringToSelectType(params["selectType"].toString());
    p.castRange = params["castRange"].toInt(1);

    for (const QJsonValue& v : buffsArray) {
        if (v.isObject())
            p.buffs.append(BuffSkillParam::fromJson(v.toObject()));
    }
    return p;
}


BuffSingleTargetSkill::BuffSingleTargetSkill(const BuffSingleTargetParams& p)
    : m_p(p) {}

BuffSingleTargetSkill::~BuffSingleTargetSkill() = default;


QVector<TargetInfo> BuffSingleTargetSkill::selectTargets(const Unit& caster, const QVector<Unit*>& allUnits) const
{
    QVector<Unit*> inRange, candidates;

    // 筛选施法范围内的单位
    std::copy_if(allUnits.begin(), allUnits.end(), std::back_inserter(inRange),
                 [&](Unit* u) {
                     return Pathfinder::hexDistance(caster.position(), u->position())
                            <= m_p.castRange;
                 });

    // 根据目标类型过滤
    switch (m_p.targetType) {
    case TargetType::Enemy:
        std::copy_if(inRange.begin(), inRange.end(), std::back_inserter(candidates),
                     [&](Unit* u) { return u->owner() != caster.owner(); });
        break;
    case TargetType::Ally:
        std::copy_if(inRange.begin(), inRange.end(), std::back_inserter(candidates),
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

int BuffSingleTargetSkill::calculateValue(const Unit&, const Unit&, bool, int) const {
    return 0; // Buff 技能不产生即时伤害
}

SkillResult BuffSingleTargetSkill::execute(Unit& caster, const QVector<Unit*>& allUnits) {
    SkillResult result;
    const auto targets = selectTargets(caster, allUnits);
    if (targets.isEmpty()) {
        result.success = false;
        return result;
    }

    for (const TargetInfo& ti : targets) {
        Unit* target = ti.target;
        for (const auto& bp : m_p.buffs) {
            const float magnitude = calculateBuffMagnitude(
                caster, *target, bp.valueType, bp.ratio, bp.fixedValue);
            result.appliedBuffs.append({target, bp.buffKey, bp.duration, bp.damageInterval, magnitude});
        }
    }

    result.success = true;
    return result;
}
