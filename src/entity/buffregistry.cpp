#include "buffregistry.h"

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

namespace {

BuffCategory stringToBuffCategory(const QString& s) {
    if (s == "Control") return BuffCategory::Control;
    if (s == "Dot") return BuffCategory::Dot;
    if (s == "StatMod") return BuffCategory::StatMod;
    qWarning() << "BuffRegistry: unknown category" << s << ", fallback to Control";
    return BuffCategory::Control;
}

BuffStackRule stringToBuffStackRule(const QString& s) {
    if (s == "refresh") return BuffStackRule::Refresh;
    if (s == "UniquePerSource") return BuffStackRule::UniquePerSource;
    if (s == "Independent") return BuffStackRule::Independent;
    qWarning() << "BuffRegistry: unknown stackRule" << s << ", fallback to Refresh";
    return BuffStackRule::Refresh;
}

BuffStat stringToBuffStat(const QString& s) {
    if (s.isEmpty()) return BuffStat::None;
    if (s == "ATK") return BuffStat::ATK;
    if (s == "MaxMana") return BuffStat::MaxMana;
    qWarning() << "BuffRegistry: unknown stat" << s << ", fallback to None";
    return BuffStat::None;
}

} // namespace


BuffRegistry* BuffRegistry::instance() {
    static BuffRegistry ins;
    return &ins;
}

bool BuffRegistry::load(const QString& jsonPath) {
    QString resolvedPath = jsonPath;
    if (!QFileInfo::exists(resolvedPath)) {
        const QString appDir = QCoreApplication::applicationDirPath();
        const QString candidates[] = {
            appDir + "/data/bufftypes.json",
            appDir + "/../data/bufftypes.json",
            appDir + "/../../data/bufftypes.json"
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
        qWarning() << "BuffRegistry: cannot open" << resolvedPath;
        return false;
    }

    QByteArray data = file.readAll();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "BuffRegistry: JSON parse error" << parseError.errorString();
        return false;
    }
    if (!doc.isArray()) {
        qWarning() << "BuffRegistry: root must be an array";
        return false;
    }

    m_defs.clear();
    const QJsonArray array = doc.array();
    for (const QJsonValue& value : array) {
        if (!value.isObject()) {
            qWarning() << "BuffRegistry: array element is not an object, skipping";
            continue;
        }
        const QJsonObject obj = value.toObject();

        BuffDef def;
        def.key = obj["key"].toString();
        def.category = stringToBuffCategory(obj["category"].toString());
        def.stackRule = stringToBuffStackRule(obj["stackRule"].toString());
        def.stat  = stringToBuffStat(obj["stat"].toString());

        if (def.key.isEmpty()) {
            qWarning() << "BuffRegistry: entry missing 'key', skipping";
            continue;
        }

        m_defs.insert(def.key, def);
    }

    qDebug() << "BuffRegistry: loaded" << m_defs.size() << "buff type(s)";
    return true;
}

const BuffDef* BuffRegistry::get(const QString& key) const {
    auto it = m_defs.constFind(key);
    return (it != m_defs.constEnd()) ? &it.value() : nullptr;
}

QList<QString> BuffRegistry::allKeys() const {
    return m_defs.keys();
}
