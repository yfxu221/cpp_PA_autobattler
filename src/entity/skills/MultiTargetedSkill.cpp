#include "MultiTargetedSkill.h"
#include "../unit.h"
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
    if (s.isEmpty()) return SelectType::Nearest; // Caster 中心等场景不依赖 selectType，缺失时给默认值
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

AreaCenter stringToAreaCenter(const QString& s) {
    if (s == "Caster") return AreaCenter::Caster;
    if (s == "Target") return AreaCenter::Target;
    throw std::invalid_argument("Unknown AreaCenter: " + s.toStdString());
}

}

MultiTargetedParams MultiTargetedParams::fromJson(const QJsonObject& json) {
    MultiTargetedParams p;
    p.name = json["name"].toString();
    p.targetType = stringToTargetType(json["targetType"].toString());
    p.selectType = stringToSelectType(json["selectType"].toString());
    p.valueType = stringToValueType(json["valueType"].toString());
    p.areaCenter = stringToAreaCenter(json["areaCenter"].toString());
    p.castRange = json["castRange"].toInt(1);
    p.value = json["value"].toInt(0);
    p.ratio = json["ratio"].toDouble(1.0);
    p.areaRange = json["areaRange"].toInt(0);
    p.splashRatio = json["splashRatio"].toDouble(0.5);
    return p;
}

MultiTargetedSkill::MultiTargetedSkill(const MultiTargetedParams& p) : m_p(p) {}

MultiTargetedSkill::~MultiTargetedSkill() = default;

QVector<TargetInfo> MultiTargetedSkill::selectTargets(
    const Unit& caster, const QVector<Unit*>& allUnits) const
{
    //  Caster 中心：以施法者位置为圆心，向四周扩散
    if (m_p.areaCenter == AreaCenter::Caster) {
        QVector<TargetInfo> result;
        for (Unit* u : allUnits) {
            bool valid = false;
            switch (m_p.targetType) {
            case TargetType::Enemy: valid = (u->owner() != caster.owner()); break;
            case TargetType::Ally:  valid = (u->owner() == caster.owner() && u != &caster); break;
            case TargetType::Self:  valid = (u == &caster); break;
            }
            if (!valid) continue;
            if (Pathfinder::hexDistance(caster.position(), u->position()) <= m_p.areaRange)
                result.append({u, /*isPrimary=*/true});
        }
        return result;
    }

    //  Target 中心：先在 castRange 内选主目标，再从主目标扩散
    QVector<Unit*> inCastRange, candidates;
    std::copy_if(allUnits.begin(), allUnits.end(), std::back_inserter(inCastRange),
                 [&](Unit* u) {
                     return Pathfinder::hexDistance(caster.position(), u->position()) <= m_p.castRange;
                 });
    switch (m_p.targetType) {
    case TargetType::Enemy:
        std::copy_if(inCastRange.begin(), inCastRange.end(), std::back_inserter(candidates),
                     [&](Unit* u) { return u->owner() != caster.owner(); });
        break;
    case TargetType::Ally:
    case TargetType::Self:
        std::copy_if(inCastRange.begin(), inCastRange.end(), std::back_inserter(candidates),
                     [&](Unit* u) { return u->owner() == caster.owner() && u != &caster; });
        candidates.push_back(const_cast<Unit*>(&caster));
        break;
    }

    if (candidates.empty())
        return {};

    // 按 selectType 挑主目标
    Unit* center = nullptr;
    switch (m_p.selectType) {
    case SelectType::LowestHp:
        center = *std::min_element(candidates.begin(), candidates.end(),
            [this, &caster](Unit* a, Unit* b) { return hpLess(caster, a, b) == a; });
        break;
    case SelectType::Nearest:
        center = *std::min_element(candidates.begin(), candidates.end(),
            [this, &caster](Unit* a, Unit* b) { return distanceLess(caster, a, b) == a; });
        break;
    case SelectType::HighestAtk:
        center = *std::max_element(candidates.begin(), candidates.end(),
            [this, &caster](Unit* a, Unit* b) { return attackGreater(caster, a, b) == a; });
        break;
    }

    // 以主目标为中心扩散
    QVector<TargetInfo> result;
    for (Unit* u : allUnits) {
        bool valid = false;
        switch (m_p.targetType) {
        case TargetType::Enemy: valid = (u->owner() != caster.owner()); break;
        case TargetType::Ally:  valid = (u->owner() == caster.owner() && u != &caster); break;
        case TargetType::Self:  valid = (u == &caster); break;
        }
        if (!valid) continue;
        if (Pathfinder::hexDistance(center->position(), u->position()) <= m_p.areaRange)
            result.append({u, /*isPrimary=*/u == center});
    }
    return result;
}


int MultiTargetedSkill::calculateValue(
    const Unit& caster, const Unit& target, bool isPrimary, int totalTargets) const
{
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
        // 伤害分摊到 n 个目标上
        baseValue = static_cast<int>(m_p.value / totalTargets);
        break;
    case ValueType::splash:
        baseValue = static_cast<int>(m_p.value * (isPrimary ? 1.0 : m_p.splashRatio));
        break;
    }

    // 敌方 → 伤害（负数），友方/自身 → 治疗（正数）
    return (m_p.targetType == TargetType::Enemy) ? -baseValue : baseValue;
}