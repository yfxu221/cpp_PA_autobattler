#ifndef BUFF_SINGLE_TARGET_SKILL_H
#define BUFF_SINGLE_TARGET_SKILL_H

#include "../skill.h"

// 单个 buff 附带的数值参数
struct BuffSkillParam {
    QString buffKey;
    int duration; // 持续回合数
    int damageInterval = 6; // DoT 伤害间隔（tick数），默认6=300ms
    ValueType valueType; // 计算数值的方式
    double ratio;
    int fixedValue;

    static BuffSkillParam fromJson(const QJsonObject& json);
};

// BuffSingleTargeted 策略参数
struct BuffSingleTargetParams {
    QString name;
    TargetType targetType = TargetType::Enemy;
    SelectType selectType = SelectType::Nearest;
    int castRange = 1;
    QVector<BuffSkillParam> buffs;

    static BuffSingleTargetParams fromJson(const QJsonObject& params,
                                           const QJsonArray& buffsArray);
};

class BuffSingleTargetSkill : public Skill {
public:
    explicit BuffSingleTargetSkill(const BuffSingleTargetParams& p);
    ~BuffSingleTargetSkill() override;

    QString name() const override { return m_p.name; }
    TargetType targetType() const override { return m_p.targetType; }
    SelectType selectType() const override { return m_p.selectType; }
    ValueType valueType() const override { return ValueType::fixed; } // buff 技能无统一伤害值
    int castRange() const override { return m_p.castRange; }

    QVector<TargetInfo> selectTargets(
        const Unit& caster,
        const QVector<Unit*>& allUnits) const override;

    int calculateValue(
        const Unit& caster,
        const Unit& target,
        bool isPrimary = true,
        int totalTargets = 1) const override;

    SkillResult execute(Unit& caster, const QVector<Unit*>& allUnits) override;

private:
    BuffSingleTargetParams m_p;
};

#endif // BUFF_SINGLE_TARGET_SKILL_H
