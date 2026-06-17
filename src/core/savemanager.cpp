#include "savemanager.h"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QCoreApplication>
#include <QDebug>


static constexpr int kSaveVersion = 1;

// JSON key 常量（防止字符串拼写错误）
namespace JsonKey {
    constexpr auto kVersion      = "version";
    constexpr auto kTimestamp    = "timestamp";
    constexpr auto kLabel        = "label";
    constexpr auto kBattleIndex  = "battleIndex";
    constexpr auto kPlayer       = "player";
    constexpr auto kEnemy        = "enemy";
    constexpr auto kUnits        = "units";
    constexpr auto kStoreSlots   = "storeSlots";
    constexpr auto kEquipBarKeys = "equipBarKeys";

    // Player / Enemy 子字段
    constexpr auto kGold    = "gold";
    constexpr auto kHp      = "hp";
    constexpr auto kMaxHp   = "maxHp";
    constexpr auto kLevel   = "level";
    constexpr auto kXp      = "xp";
    constexpr auto kXpToNext = "xpToNext";

    // Unit 子字段
    constexpr auto kKey            = "key";
    constexpr auto kName           = "name";
    constexpr auto kStarLevel      = "starLevel";
    constexpr auto kCol            = "col";
    constexpr auto kRow            = "row";
    constexpr auto kHpUnit         = "hp";
    constexpr auto kMana           = "mana";
    constexpr auto kOwner          = "owner";
    constexpr auto kEquipmentKeys  = "equipmentKeys";
    constexpr auto kSkillKey       = "skillKey";

    // Store 子字段
    constexpr auto kFilled = "filled";
}


//  内部辅助：序列化 / 反序列化
static QJsonObject playerToJson(int gold, int hp, int maxHp, int level, int xp, int xpToNext)
{
    QJsonObject obj;
    obj[JsonKey::kGold] = gold;
    obj[JsonKey::kHp] = hp;
    obj[JsonKey::kMaxHp] = maxHp;
    obj[JsonKey::kLevel] = level;
    obj[JsonKey::kXp] = xp;
    obj[JsonKey::kXpToNext] = xpToNext;
    return obj;
}

static bool playerFromJson(const QJsonObject& obj,
                           int& gold, int& hp, int& maxHp, int& level, int& xp, int& xpToNext)
{
    gold = obj[JsonKey::kGold].toInt(20);
    hp = obj[JsonKey::kHp].toInt(100);
    maxHp = obj[JsonKey::kMaxHp].toInt(100);
    level = obj[JsonKey::kLevel].toInt(1);
    xp = obj[JsonKey::kXp].toInt(0);
    xpToNext = obj[JsonKey::kXpToNext].toInt(4);
    return true;
}

static QJsonObject unitEntryToJson(const SaveData::UnitEntry& u)
{
    QJsonObject obj;
    obj[JsonKey::kKey] = u.key;
    obj[JsonKey::kName] = u.name;
    obj[JsonKey::kStarLevel] = u.starLevel;
    obj[JsonKey::kCol] = u.col;
    obj[JsonKey::kRow] = u.row;
    obj[JsonKey::kHpUnit] = u.hp;
    obj[JsonKey::kMana] = u.mana;
    obj[JsonKey::kOwner] = u.owner;

    QJsonArray eqArr;
    for (const QString& ek : u.equipmentKeys) {
        eqArr.append(ek);
    }
    obj[JsonKey::kEquipmentKeys] = eqArr;
    obj[JsonKey::kSkillKey] = u.skillKey;
    return obj;
}

static SaveData::UnitEntry unitEntryFromJson(const QJsonObject& obj)
{
    SaveData::UnitEntry u;
    u.key = obj[JsonKey::kKey].toString();
    u.name = obj[JsonKey::kName].toString();
    u.starLevel = obj[JsonKey::kStarLevel].toInt(1);
    u.col = obj[JsonKey::kCol].toInt(0);
    u.row = obj[JsonKey::kRow].toInt(0);
    u.hp = obj[JsonKey::kHpUnit].toInt(100);
    u.mana = obj[JsonKey::kMana].toInt(0);
    u.owner = obj[JsonKey::kOwner].toString("player");

    const QJsonArray eqArr = obj[JsonKey::kEquipmentKeys].toArray();
    for (const auto& v : eqArr) {
        u.equipmentKeys.append(v.toString());
    }
    u.skillKey = obj[JsonKey::kSkillKey].toString();
    return u;
}

static QJsonObject storeEntryToJson(const SaveData::StoreEntry& s)
{
    QJsonObject obj;
    obj[JsonKey::kKey] = s.key;
    obj[JsonKey::kStarLevel] = s.starLevel;
    obj[JsonKey::kFilled] = s.filled;
    return obj;
}

static SaveData::StoreEntry storeEntryFromJson(const QJsonObject& obj)
{
    SaveData::StoreEntry s;
    s.key = obj[JsonKey::kKey].toString();
    s.starLevel = obj[JsonKey::kStarLevel].toInt(1);
    s.filled = obj[JsonKey::kFilled].toBool(false);
    return s;
}
// 从 SaveData 结构序列化到 JSON对象
static QJsonObject saveDataToJson(const SaveData& data)
{
    QJsonObject root;
    root[JsonKey::kVersion] = kSaveVersion;
    root[JsonKey::kTimestamp] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    root[JsonKey::kBattleIndex] = data.battleIndex;

    root[JsonKey::kPlayer] = playerToJson(
        data.playerGold, data.playerHp, data.playerMaxHp,
        data.playerLevel, data.playerXp, data.playerXpToNext);

    root[JsonKey::kEnemy] = playerToJson(
        data.enemyGold, data.enemyHp, data.enemyMaxHp,
        data.enemyLevel, data.enemyXp, data.enemyXpToNext);

    QJsonArray unitsArr;
    for (const auto& u : data.units) {
        unitsArr.append(unitEntryToJson(u));
    }
    root[JsonKey::kUnits] = unitsArr;

    QJsonArray storeArr;
    for (int i = 0; i < 5; ++i) {
        storeArr.append(storeEntryToJson(data.storeSlots[i]));
    }
    root[JsonKey::kStoreSlots] = storeArr;

    QJsonArray eqArr;
    for (int i = 0; i < 8; ++i) {
        eqArr.append(data.equipBarKeys[i]);
    }
    root[JsonKey::kEquipBarKeys] = eqArr;

    return root;
}

// 从 JSON 反序列化到 SaveData 结构
static std::optional<SaveData> saveDataFromJson(const QJsonObject& root)
{
    // version 字段可选
    int version = root[JsonKey::kVersion].toInt(1);

    SaveData data;

    // Player
    {
        QJsonObject obj = root[JsonKey::kPlayer].toObject();
        playerFromJson(obj,
            data.playerGold, data.playerHp, data.playerMaxHp,
            data.playerLevel, data.playerXp, data.playerXpToNext);
    }

    // Enemy
    {
        QJsonObject obj = root[JsonKey::kEnemy].toObject();
        playerFromJson(obj,
            data.enemyGold, data.enemyHp, data.enemyMaxHp,
            data.enemyLevel, data.enemyXp, data.enemyXpToNext);
    }

    data.battleIndex = root[JsonKey::kBattleIndex].toInt(1);

    // Units
    {
        const QJsonArray arr = root[JsonKey::kUnits].toArray();
        for (const auto& v : arr) {
            data.units.push_back(unitEntryFromJson(v.toObject()));
        }
    }

    // Store slots
    {
        const QJsonArray arr = root[JsonKey::kStoreSlots].toArray();
        for (int i = 0; i < 5 && i < arr.size(); ++i) {
            data.storeSlots[i] = storeEntryFromJson(arr[i].toObject());
        }
    }

    // EquipBar keys
    {
        const QJsonArray arr = root[JsonKey::kEquipBarKeys].toArray();
        for (int i = 0; i < 8 && i < arr.size(); ++i) {
            data.equipBarKeys[i] = arr[i].toString();
        }
    }

    (void)version;
    return data;
}

//  文件路径
QString SaveManager::savesDirectory()
{
    return QCoreApplication::applicationDirPath() + "/saves";
}

QString SaveManager::saveFilePath(int slot)
{
    return savesDirectory() + QString("/slot_%1.json").arg(slot);
}


// 主要接口实现
bool SaveManager::saveToFile(int slot, const SaveData& data, const QString& label)
{
    if (slot < 0 || slot >= MAX_SLOTS) return false;

    // 确保目录存在
    const QString dirPath = savesDirectory();
    QDir dir;
    if (!dir.mkpath(dirPath)) {
        qWarning() << "SaveManager: 无法创建存档目录" << dirPath;
        return false;
    }

    // 构建 JSON
    QJsonObject root = saveDataToJson(data);
    // 覆盖 timestamp
    root[JsonKey::kTimestamp] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    root[JsonKey::kLabel] = label;

    // 写入文件
    QFile file(saveFilePath(slot));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "SaveManager: 无法写入存档文件" << file.fileName();
        return false;
    }

    QJsonDocument doc(root);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    qDebug() << "SaveManager: 存档已保存到槽位" << slot << file.fileName();
    return true;
}

std::optional<SaveData> SaveManager::loadFromFile(int slot)
{
    if (slot < 0 || slot >= MAX_SLOTS) return std::nullopt;

    const QString path = saveFilePath(slot);
    QFile file(path);
    if (!file.exists()) {
        return std::nullopt;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "SaveManager: 无法读取存档文件" << path;
        return std::nullopt;
    }

    QByteArray raw = file.readAll();
    file.close();

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(raw, &err);
    if (err.error != QJsonParseError::NoError) {
        qWarning() << "SaveManager: JSON 解析失败" << path << err.errorString();
        return std::nullopt;
    }

    if (!doc.isObject()) {
        qWarning() << "SaveManager: 存档文件格式错误（非 JSON 对象）" << path;
        return std::nullopt;
    }

    return saveDataFromJson(doc.object());
}

SaveMeta SaveManager::getMeta(int slot)
{
    SaveMeta meta;
    meta.slot = slot;
    meta.isEmpty = true;

    if (slot < 0 || slot >= MAX_SLOTS) return meta;

    const QString path = saveFilePath(slot);
    QFile file(path);
    if (!file.exists()) return meta;

    if (!file.open(QIODevice::ReadOnly)) return meta;

    QByteArray raw = file.readAll();
    file.close();

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(raw, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) return meta;

    QJsonObject root = doc.object();

    // 提取元数据字段
    meta.isEmpty = false;
    meta.timestamp = root[JsonKey::kTimestamp].toString();
    meta.label = root[JsonKey::kLabel].toString();
    meta.battleIndex = root[JsonKey::kBattleIndex].toInt(1);

    QJsonObject playerObj = root[JsonKey::kPlayer].toObject();
    meta.playerLevel = playerObj[JsonKey::kLevel].toInt(1);
    meta.playerHp = playerObj[JsonKey::kHp].toInt(100);
    meta.playerGold = playerObj[JsonKey::kGold].toInt(20);

    return meta;
}

std::vector<SaveMeta> SaveManager::getAllMetas()
{
    std::vector<SaveMeta> result;
    for (int i = 0; i < MAX_SLOTS; ++i) {
        result.push_back(getMeta(i));
    }
    return result;
}

bool SaveManager::deleteSave(int slot)
{
    if (slot < 0 || slot >= MAX_SLOTS) return false;

    const QString path = saveFilePath(slot);
    QFile file(path);
    if (!file.exists()) return true; // 文件不存在，视为删除成功

    if (!file.remove()) {
        qWarning() << "SaveManager: 无法删除存档文件" << path;
        return false;
    }

    qDebug() << "SaveManager: 已删除槽位" << slot << "的存档";
    return true;
}
