#pragma once
#include <QString>
#include <QVector>
#include <QJsonObject>
#include <QPoint>

class Unit;

enum class TargetType { // 目标类型
    Enemy, // 单体敌人
    Ally, // 单体友军
    Self, // 仅自身
};

enum class SelectType { // 筛选方式
    LowestHp, // 血量最低
    Nearest, // 最近的单位
    HighestAtk // 攻击力最高
};

enum class ValueType {
    AtkRatio, // 按照攻击力百分比
    HpRatio, // 按照当前生命值百分比
    fixed, // 固定数值
    split, // 分摊一个固定数值到n个目标（仅限对群）
    splash // 溅射：主目标吃满伤害，次要目标吃部分伤害
};

struct TargetInfo {
    Unit* target = nullptr;
    bool isPrimary = true; // 是否主目标
};

struct SkillResult {
    struct HitInfo {
        Unit* target = nullptr;
        int value = 0; // 正数=治疗量, 负数=伤害量
    };
    QVector<HitInfo> hits;
    bool success = false; // 是否成功释放
};


class Skill {
public:
    virtual ~Skill() = default;

    virtual QString name() const = 0; // 技能名称
    virtual TargetType targetType() const = 0; // 目标类型
    virtual SelectType selectType() const = 0; // 筛选方式
    virtual ValueType valueType() const = 0; // 数值类型
    virtual int castRange() const = 0; // 施法范围（格数）

    // 选择目标：给定施法者和全场单位，返回要作用的单位列表
    virtual QVector<TargetInfo> selectTargets(const Unit& caster, const QVector<Unit*>& allUnits) const = 0;

    /// 计算对单个目标的效果数值
    /// @param caster   施法者
    /// @param target   目标单位
    /// @param isPrimary 是否主目标（溅射技能里主目标吃满伤害）
    /// @param totalTargets 选定的目标总数（仅 ValueType::split 时使用，用于计算分摊伤害）
    /// @return 正数=治疗量, 负数=伤害量
    virtual int calculateValue(const Unit& caster, 
                                const Unit& target, 
                                bool isPrimary = true, 
                                int totalTargets = 1
                            ) const = 0;

    // 执行技能：给定施法者、全场单位和选定的目标，计算并返回技能效果
    /// @return HitInfo.value 正数=治疗量, 负数=伤害量
    SkillResult execute(Unit& caster, const QVector<Unit*>& allUnits);

protected:
    Unit* distanceLess(const Unit& caster, Unit* a, Unit* b) const; 

    Unit* attackGreater(const Unit& caster, Unit* a, Unit* b) const;

    Unit* hpLess(const Unit& caster, Unit* a, Unit* b) const;

    // 找到 origin 周围 range 格内的所有敌人
    QVector<Unit*> enemiesInRange(
        QPoint origin, int range,
        const Unit& caster, const QVector<Unit*>& allUnits) const;

    // 找到 origin 周围 range 格内的所有友军
    QVector<Unit*> alliesInRange(
        QPoint origin, int range,
        const Unit& caster, const QVector<Unit*>& allUnits) const;

    // 找到全场血量最低的友军
    Unit* lowestHpAlly(
        const Unit& caster, const QVector<Unit*>& allUnits) const;

    // 找到离 origin 最近的敌人
    Unit* nearestEnemy(
        QPoint origin,
        const Unit& caster, const QVector<Unit*>& allUnits) const;

    // 找到 Attack 最大的敌人
    Unit* highestAtkEnemy(
        const Unit& caster, const QVector<Unit*>& allUnits) const;

    QVector<Unit*> UnitInRange(
        const Unit* center, const QVector<Unit*>& others, int range) const;
};