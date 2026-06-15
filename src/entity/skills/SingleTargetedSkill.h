#ifndef SINGLE_TARGETED_SKILL_H
#define SINGLE_TARGETED_SKILL_H

#include "../skill.h"

struct SingleTargetedParams {
    QString name;
    TargetType targetType = TargetType::Enemy;
    SelectType selectType = SelectType::Nearest;
    ValueType valueType  = ValueType::AtkRatio;
    int castRange = 1;
    int value = 0;       // 仅 ValueType::fixed 时使用
    double ratio = 1.0;     // 仅 ValueType::AtkRatio / HpRatio 时使用

    // JSON → 类型化参数
    static SingleTargetedParams fromJson(const QJsonObject& json);
};


class SingleTargetedSkill : public Skill {
public:
    explicit SingleTargetedSkill(const SingleTargetedParams& p);
    ~SingleTargetedSkill() override;

    QString name() const override { return m_p.name; };
    TargetType targetType() const override { return m_p.targetType; };
    SelectType selectType() const override { return m_p.selectType; };
    ValueType valueType()  const override { return m_p.valueType; };
    int castRange()  const override { return m_p.castRange; };

    QVector<TargetInfo> selectTargets(
        const Unit& caster,
        const QVector<Unit*>& allUnits) const override;

    int calculateValue(
        const Unit& caster,
        const Unit& target,
        bool isPrimary = true,
        int totalTargets = 1) const override;

private:
    SingleTargetedParams m_p;
};

#endif // SINGLE_TARGETED_SKILL_H
