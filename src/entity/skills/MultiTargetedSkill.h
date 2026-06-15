#ifndef MULTI_TARGETED_SKILL_H
#define MULTI_TARGETED_SKILL_H

#include "../Skill.h"

enum class AreaCenter {
    Caster,
    Target,
};

struct MultiTargetedParams {
    QString name;
    TargetType targetType = TargetType::Enemy;
    SelectType selectType = SelectType::Nearest;
    ValueType valueType  = ValueType::AtkRatio;
    AreaCenter areaCenter = AreaCenter::Target;
    int castRange = 1;
    int value = 0;       // 仅 ValueType::fixed 时使用
    double ratio = 1.0;     // 仅 ValueType::AtkRatio / HpRatio时使用
    int areaRange = 0;
    double splashRatio = 0.5; // 仅 ValueType::splash 时使用

    // JSON → 类型化参数
    static MultiTargetedParams fromJson(const QJsonObject& json);
};

class MultiTargetedSkill : public Skill {
public:
    MultiTargetedSkill(const MultiTargetedParams& p);
    ~MultiTargetedSkill() override;

    QString name() const override { return m_p.name; };
    TargetType targetType() const override { return m_p.targetType; };
    SelectType selectType() const override { return m_p.selectType; };
    ValueType valueType()  const override { return m_p.valueType; };
    int castRange()  const override { return m_p.castRange; };
    int areaRange()  const { return m_p.areaRange; };
    AreaCenter areaCenter() const { return m_p.areaCenter; };

    QVector<TargetInfo> selectTargets(
        const Unit& caster,
        const QVector<Unit*>& allUnits) const override;

    int calculateValue(
        const Unit& caster,
        const Unit& target,
        bool isPrimary = true,
        int totalTargets = 1) const override;

private:
    MultiTargetedParams m_p;

};

#endif // MULTI_TARGETED_SKILL_H