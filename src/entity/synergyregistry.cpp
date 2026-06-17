#include "synergyregistry.h"

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>

SynergyRegistry* SynergyRegistry::instance() {
    static SynergyRegistry ins;
    return &ins;
}

QVector<SynergyThreshold> SynergyRegistry::thresholds(const QString& trait) const {
    auto it = m_rules.find(trait);
    if (it == m_rules.end()) return {};
    return it.value();
}

SynergyBonus SynergyRegistry::getBonus(const QString& trait, int count) const {
    auto it = m_rules.find(trait);
    if (it == m_rules.end()) return {};
    
    SynergyBonus result;
    for (const auto& thresh : it.value()) {
        if (count >= thresh.count) {
            result = thresh.bonus;  // 取最高满足的阈值
        }
    }
    return result;
}

bool SynergyRegistry::load(const QString& jsonPath) {
    QString resolvedPath = jsonPath;
    if (!QFileInfo::exists(resolvedPath)) {
        const QString appDir = QCoreApplication::applicationDirPath();
        const QString candidates[] = {
            appDir + "/data/synergies.json",
            appDir + "/../data/synergies.json",
            appDir + "/../../data/synergies.json"
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
        qWarning() << "SynergyRegistry: cannot open" << resolvedPath;
        return false;
    }

    QByteArray data = file.readAll();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "SynergyRegistry: JSON parse error" << parseError.errorString();
        return false;
    }
    if (!doc.isArray()) {
        qWarning() << "SynergyRegistry: root must be an array";
        return false;
    }

    m_rules.clear();
    const QJsonArray array = doc.array();
    for (const QJsonValue& value : array) {
        if (!value.isObject()) {
            qWarning() << "SynergyRegistry: array element is not an object, skipping";
            continue;
        }
        const QJsonObject obj = value.toObject();

        QString trait = obj["trait"].toString();
        if (trait.isEmpty()) {
            qWarning() << "SynergyRegistry: entry missing 'trait', skipping";
            continue;
        }

        QJsonArray thresholds = obj["thresholds"].toArray();
        QList<SynergyThreshold> ruleList;
        for (const QJsonValue& threshValue : thresholds) {
            if (!threshValue.isObject()) {
                qWarning() << "SynergyRegistry: threshold is not an object, skipping";
                continue;
            }
            const QJsonObject threshObj = threshValue.toObject();

            SynergyThreshold thresh;
            thresh.count = threshObj["count"].toInt();
            thresh.bonus.bonusAtk = threshObj["bonusAtk"].toInt();
            thresh.bonus.bonusMaxHp = threshObj["bonusMaxHp"].toInt();
            thresh.bonus.bonusMaxMana = threshObj["bonusMaxMana"].toInt();
            thresh.bonus.bonusSpeed = threshObj["bonusSpeed"].toInt();
            thresh.bonus.dotIntervalReduction = threshObj["dotIntervalReduction"].toInt();

            ruleList.append(thresh);
        }
        m_rules[trait] = ruleList;
    }

    qDebug() << "SynergyRegistry: loaded" << m_rules.size() << "synergy rule(s)";
    return true;
}

QStringList SynergyRegistry::allTraits() const {
    return m_rules.keys();
}