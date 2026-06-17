#include "unit.h"
#include "skill.h"
#include <algorithm>
#include "core/pathfinder.h"


// 距离比较：距离近 → 血量低 → Y 小 → X 小
Unit* Skill::distanceLess(const Unit& caster, Unit* a, Unit* b) const {
    if (a == b) return a;
    int distA = Pathfinder::hexDistance(caster.position(), a->position());
    int distB = Pathfinder::hexDistance(caster.position(), b->position());
    if(distA != distB) return distA < distB ? a : b;
    if(a->hp() != b->hp()) return a->hp() < b->hp() ? a : b;
    if(a->position().y() != b->position().y())
        return a->position().y() > b->position().y() ? a : b;
    return a->position().x() > b->position().x() ? a : b;
}

// 攻击力比较：攻击力高 → 距离远 → 血量低 → Y 小 → X 小
Unit* Skill::attackGreater(const Unit& caster, Unit* a, Unit* b) const {
    if (a == b) return a;
    if(a->atk() != b->atk()) return a->atk() > b->atk() ? a : b;
    int distA = Pathfinder::hexDistance(caster.position(), a->position());
    int distB = Pathfinder::hexDistance(caster.position(), b->position());
    if(distA != distB) return distA > distB ? a : b;
    if(a->hp() != b->hp()) return a->hp() < b->hp() ? a : b;
    if(a->position().y() != b->position().y())
        return a->position().y() > b->position().y() ? a : b;
    return a->position().x() > b->position().x() ? a : b;
}

// 血量比较：血量低 → 距离近 → Y 小 → X 小
Unit* Skill::hpLess(const Unit& target, Unit* a, Unit* b) const {
    if (a == b) return a;
    if(a->hp() != b->hp()) return a->hp() < b->hp() ? a : b;
    int distA = Pathfinder::hexDistance(target.position(), a->position());
    int distB = Pathfinder::hexDistance(target.position(), b->position());
    if(distA != distB) return distA < distB ? a : b;
    if(a->position().y() != b->position().y())
        return a->position().y() > b->position().y() ? a : b;
    return a->position().x() > b->position().x() ? a : b;
}


SkillResult Skill::execute(Unit& caster, const QVector<Unit*>& allUnits) {
    SkillResult result;
    auto targets = selectTargets(caster, allUnits);
    if (targets.isEmpty()) {
        result.success = false;
        return result; // 没有目标，技能释放失败
    }

    for (const TargetInfo& targetInfo : targets) {
        Unit* target = targetInfo.target;
        int value = calculateValue(caster, *target, targetInfo.isPrimary, targets.size());
        result.hits.append({target, value});
    }
    result.success = true;
    return result;
}

QVector<Unit*> Skill::enemiesInRange(QPoint origin, int range, const Unit& caster, const QVector<Unit*>& allUnits) const {
    QVector<Unit*> result;
    for (Unit* unit : allUnits) {
        if (unit->owner() != caster.owner() && Pathfinder::hexDistance(origin, unit->position()) <= range) {
            result.append(unit);
        }
    }
    return result;
}

QVector<Unit*> Skill::alliesInRange(QPoint origin, int range, const Unit& caster, const QVector<Unit*>& allUnits) const {
    QVector<Unit*> result;
    for (Unit* unit : allUnits) {
        if (unit->owner() == caster.owner() && Pathfinder::hexDistance(origin, unit->position()) <= range) {
            result.append(unit);
        }
    }
    return result;
}

Unit* Skill::lowestHpAlly(const Unit& caster, const QVector<Unit*>& allUnits) const {
    Unit* lowest = nullptr;
    for (Unit* unit : allUnits) {
        if (unit->owner() == caster.owner()) {
            if (!lowest || unit->hp() < lowest->hp()) {
                lowest = unit;
            }
        }
    }
    return lowest;
}

Unit* Skill::nearestEnemy(QPoint origin, const Unit& caster, const QVector<Unit*>& allUnits) const {
    Unit* nearest = nullptr;
    for (Unit* unit : allUnits) {
        if (unit->owner() != caster.owner()) {
            if (!nearest || distanceLess(caster, unit, nearest) == unit) {
                nearest = unit;
            }
        }
    }
    return nearest;
}

Unit* Skill::highestAtkEnemy(const Unit& caster, const QVector<Unit*>& allUnits) const {
    QVector<Unit*> enemies;
    std::copy_if(allUnits.begin(), allUnits.end(), std::back_inserter(enemies), [&](Unit* unit){
        return unit->owner() != caster.owner();
    });
    if (enemies.isEmpty()) {
        return nullptr;
    }
    return *std::max_element(enemies.begin(), enemies.end(),
    [this, &caster](Unit* a, Unit* b) { return Skill::attackGreater(caster, a, b) == b; });
}

QVector<Unit*> Skill::UnitInRange(const Unit* center, const QVector<Unit*>& others, int range) const {
    QVector<Unit*> result;
    for (Unit* unit : others) {
        if (Pathfinder::hexDistance(center->position(), unit->position()) <= range) {
            result.push_back(unit);
        }
    }
    return result;
}
