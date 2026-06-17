#include "equipmentdata.h"

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

EquipmentRegistry* EquipmentRegistry::instance()
{
    static EquipmentRegistry ins;
    return &ins;
}

bool EquipmentRegistry::load(const QString& jsonPath)
{
    QString resolvedPath = jsonPath;
    if (!QFileInfo::exists(resolvedPath)) {
        const QString appDir = QCoreApplication::applicationDirPath();
        const QString candidates[] = {
            appDir + "/data/equipments.json",
            appDir + "/../data/equipments.json",
            appDir + "/../../data/equipments.json"
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
        qWarning() << "EquipmentRegistry: cannot open:" << resolvedPath;
        return false;
    }

    QByteArray data = file.readAll();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "EquipmentRegistry: JSON parse error" << parseError.errorString();
        return false;
    }
    if (!doc.isArray()) {
        qWarning() << "EquipmentRegistry: root must be an array";
        return false;
    }

    m_templates.clear();
    const QJsonArray array = doc.array();
    for (const QJsonValue& value : array) {
        if (!value.isObject()) {
            qWarning() << "EquipmentRegistry: root must be an array";
            continue;
        }

        const QJsonObject obj = value.toObject();
        Equipment eq;
        eq.key = obj["key"].toString();
        eq.name = obj["name"].toString();
        eq.bonusAtk = obj["bonusAtk"].toInt(0);
        eq.bonusMaxHp = obj["bonusMaxHp"].toInt(0);
        eq.bonusMaxMana = obj["bonusMaxMana"].toInt(0);
        eq.bonusSpeed = obj["bonusSpeed"].toInt(0);
        eq.bonusAttackSpeed = static_cast<float>(obj["bonusAttackSpeed"].toDouble(0.0));
        eq.bonusMoveSpeed = static_cast<float>(obj["bonusMoveSpeed"].toDouble(0.0));
        eq.spritePath = obj["sprite"].toString();

        if (eq.key.isEmpty()) {
            qWarning() << "EquipmentRegistry: equipment with empty key, skipping";
            continue;
        }

        m_templates.insert(eq.key, eq);
    }

    qDebug() << "EquipmentRegistry: loaded" << m_templates.size() << "equipment";
    return true;
}

const Equipment* EquipmentRegistry::get(const QString& key) const
{
    auto it = m_templates.constFind(key);
    return (it != m_templates.constEnd()) ? &it.value() : nullptr;
}

std::shared_ptr<Equipment> EquipmentRegistry::createEquipment(const QString& key) const
{
    const Equipment* tmpl = get(key);
    if (!tmpl) {
        qWarning() << "EquipmentRegistry: unknown equipment key:" << key;
        return nullptr;
    }

    // Equipment 是纯值类型，直接拷贝一份作为实例
    return std::make_shared<Equipment>(*tmpl);
}

QList<QString> EquipmentRegistry::allKeys() const
{
    return m_templates.keys();
}
