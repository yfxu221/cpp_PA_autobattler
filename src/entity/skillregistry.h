#ifndef SKILL_REGISTRY_H
#define SKILL_REGISTRY_H

#include <QString>
#include <QMap>
#include <QList>
#include <QJsonObject>
#include <memory>

#include "skill.h"


struct SkillDef {
    QString key;
    QString name;
    QString strategy; // 策略名
    QJsonObject params; // 策略专属参数，透传给 XxxParams::fromJson
};


class SkillRegistry {
public:
    static SkillRegistry* instance();

    // 从 JSON 文件加载所有技能定义
    bool load(const QString& jsonPath);

    // 按 key 查找技能定义（返回 nullptr 表示未找到）
    const SkillDef* get(const QString& key) const;

    // 返回所有技能 key
    QList<QString> allKeys() const;

    // 根据 SkillDef 创建对应的 Skill 子类实例
    std::unique_ptr<Skill> createSkill(const QString& key) const;

private:
    SkillRegistry() = default;
    QMap<QString, SkillDef> m_defs;
};

#endif // SKILL_REGISTRY_H
