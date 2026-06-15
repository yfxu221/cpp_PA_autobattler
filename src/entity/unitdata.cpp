#include "unitdata.h"
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonParseError>
#include <QFileInfo>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include "skillregistry.h"


UnitData* UnitData::instance()
{
    static UnitData ins; // 全局唯一、只初始化一次
    return &ins;
}

bool UnitData::load(const QString& jsonPath)
{   
    QString resolvedPath = jsonPath;
    if (!QFileInfo::exists(resolvedPath)){
        const QString appDir = QCoreApplication::applicationDirPath();
        const QString candidates[] = {
            appDir + "/data/units.json",
            appDir + "/../data/units.json",
            appDir + "/../../data/units.json"
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
        qWarning() << "无法打开单位数据文件:" << resolvedPath;
        return false;
    }

    QByteArray data = file.readAll();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "解析单位数据文件失败:" << resolvedPath << parseError.errorString();
        return false;
    }

    if (!doc.isArray()) {
        qWarning() << "单位数据文件格式错误，根节点必须是数组:" << resolvedPath;
        return false;
    }

    m_templates.clear();

    const QJsonArray array = doc.array();
    for (const QJsonValue& value : array) {
        if (!value.isObject()) {
            qWarning() << "单位数据文件中存在非对象元素:" << resolvedPath;
            return false;
        }

        const QJsonObject object = value.toObject();

        UnitTemplate temp;
        temp.key = object.value("key").toString();
        temp.name = object.value("name").toString();
        temp.maxHp = object.value("maxHp").toInt();
        temp.atk = object.value("atk").toInt();
        temp.range = object.value("range").toInt();
        temp.maxMana = object.value("maxMana").toInt();
        temp.sprite = object.value("sprite").toString();
        temp.attackCooldown = object.value("attackCooldown").toInt();
        temp.speed = object.value("speed").toInt();
        temp.price = object.value("price").toInt();
        temp.skill = object.value("skill").toString();

        const QJsonValue traitsValue = object.value("traits");
        if (traitsValue.isArray()) {
            const QJsonArray traitsArray = traitsValue.toArray();
            for (const QJsonValue& traitValue : traitsArray) {
                if (traitValue.isString()) {
                    temp.traits.insert(traitValue.toString());
                }
            }
        }

        if (temp.key.isEmpty()) {
            qWarning() << "单位数据中缺少 key:" << object;
            return false;
        }

        m_templates.insert(temp.key, temp);
    }

    return true;
}

std::unique_ptr<Unit> UnitData::createUnit(const QString& key, Owner owner, int starLevel) const
{
    const auto it = m_templates.constFind(key);
    if (it == m_templates.constEnd()) {
        qWarning() << "找不到单位模板:" << key;
        return nullptr;
    }

    const UnitTemplate& temp = it.value();
    std::unique_ptr<Unit> unit = std::make_unique<Unit>(temp.key, temp.name, temp.maxHp, temp.atk, temp.range, temp.maxMana,
                                  starLevel, temp.speed, temp.traits, owner, temp.sprite,
                                  temp.attackCooldown, temp.price);
    if (!temp.skill.isEmpty()) {
    auto skill = SkillRegistry::instance()->createSkill(temp.skill);
    if (skill)
        unit->addSkill(std::move(skill));
    else
        qWarning() << "Unit" << key << ": skill not found:" << temp.skill;
    }
    return unit;
}

QList<QString> UnitData::allKeys() const
{
    return m_templates.keys();
}


