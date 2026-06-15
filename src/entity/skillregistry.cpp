#include "skillregistry.h"
#include "skills/SingleTargetedSkill.h"
#include "skills/MultiTargetedSkill.h"

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>


SkillRegistry* SkillRegistry::instance() {
    static SkillRegistry ins;
    return &ins;
}


bool SkillRegistry::load(const QString& jsonPath) { //  加载 skills.json
    QString resolvedPath = jsonPath;
    if (!QFileInfo::exists(resolvedPath)) {
        const QString appDir = QCoreApplication::applicationDirPath();
        const QString candidates[] = {
            appDir + "/data/skills.json",
            appDir + "/../data/skills.json",
            appDir + "/../../data/skills.json"
        };
        for (const auto& path : candidates) {
            if (QFileInfo::exists(path)) {
                resolvedPath = QFileInfo(path).absoluteFilePath();
                break;
            }
        }
    }

    QFile file(resolvedPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "SkillRegistry: cannot open" << resolvedPath;
        return false;
    }

    QByteArray data = file.readAll();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "SkillRegistry: JSON parse error" << parseError.errorString();
        return false;
    }
    if (!doc.isArray()) {
        qWarning() << "SkillRegistry: root must be an array";
        return false;
    }

    m_defs.clear();
    const QJsonArray array = doc.array();
    for (const QJsonValue& value : array) {
        if (!value.isObject()) {
            qWarning() << "SkillRegistry: array element is not an object, skipping";
            continue;
        }
        const QJsonObject obj = value.toObject();

        SkillDef def;
        def.key = obj["key"].toString();
        def.name = obj["name"].toString();
        def.strategy = obj["strategy"].toString();
        def.params = obj["params"].toObject();

        if (def.key.isEmpty()) {
            qWarning() << "SkillRegistry: entry missing 'key', skipping";
            continue;
        }
        if (def.strategy.isEmpty()) {
            qWarning() << "SkillRegistry: entry" << def.key << "missing 'strategy', skipping";
            continue;
        }

        m_defs.insert(def.key, def);
    }

    qDebug() << "SkillRegistry: loaded" << m_defs.size() << "skill(s)";
    return true;
}

const SkillDef* SkillRegistry::get(const QString& key) const {
    auto it = m_defs.constFind(key);
    return (it != m_defs.constEnd()) ? &it.value() : nullptr;
}

QList<QString> SkillRegistry::allKeys() const {
    return m_defs.keys();
}


std::unique_ptr<Skill> SkillRegistry::createSkill(const QString& key) const {
    const SkillDef* def = get(key);
    if (!def) {
        qWarning() << "SkillRegistry: unknown skill key" << key;
        return nullptr;
    }

    if (def->strategy == "SingleTargeted") {
        auto params = SingleTargetedParams::fromJson(def->params);
        if (params.name.isEmpty()) // 如果 JSON 中没有 name 字段，就用 SkillDef 的 name 字段
            params.name = def->name;
        return std::make_unique<SingleTargetedSkill>(params);
    }

    if (def->strategy == "MultiTargeted") {
        auto params = MultiTargetedParams::fromJson(def->params);
        if (params.name.isEmpty()) // 如果 JSON 中没有 name 字段，就用 SkillDef 的 name 字段
            params.name = def->name;
        return std::make_unique<MultiTargetedSkill>(params);
    }

    qWarning() << "SkillRegistry: unknown strategy" << def->strategy
               << "for skill" << key;
    return nullptr;
}
