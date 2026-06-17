#ifndef CORE_SAVEMANAGER_H
#define CORE_SAVEMANAGER_H

#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <optional>
#include <vector>

// 存档元数据（仅用于 UI 列表展示，不含完整游戏数据）
struct SaveMeta {
    int slot = -1;
    QString timestamp;  // "2026-06-17 14:30:00"
    int battleIndex = 1;
    int playerLevel = 1;
    int playerHp = 100;
    int playerGold = 20;
    QString label;   // 用户备注（可为空）
    bool isEmpty = true;
};

// 完整存档数据
struct SaveData {
    // Player
    int playerGold = 20;
    int playerHp = 100;
    int playerMaxHp = 100;
    int playerLevel = 1;
    int playerXp = 0;
    int playerXpToNext = 4;

    // Enemy
    int enemyGold = 10;
    int enemyHp = 100;
    int enemyMaxHp = 100;
    int enemyLevel = 1;
    int enemyXp = 0;
    int enemyXpToNext = 4;

    // Game
    int battleIndex = 1;

    // Units
    struct UnitEntry {
        QString key;  // 单位模板 key
        QString name;  // 显示名称
        int starLevel = 1;
        int col = 0;  // 棋盘列（0-7）
        int row = 0;  // 棋盘/备战席行（0-8）
        int hp = 100;
        int mana = 0;
        QString owner; 
        QStringList equipmentKeys; // 装备 key 列表
        QString skillKey;   // 技能 key（空字符串 = 无技能）
    };
    std::vector<UnitEntry> units;

    // Store (5 slots)
    struct StoreEntry {
        QString key; // 空字符串 = 该格无单位
        int starLevel = 1;
        bool filled = false;
    };
    StoreEntry storeSlots[5];

    // EquipBar (8 slots)
    // 空字符串 = 该槽位无装备
    QString equipBarKeys[8];
};

// ━━━ 存档管理器 ━━
class SaveManager {
public:
    static constexpr int MAX_SLOTS = 8;    // 4×2 = 8 个存档槽位

    // 读写存档文件
    static bool saveToFile(int slot, const SaveData& data, const QString& label = "");
    static std::optional<SaveData> loadFromFile(int slot);

    // 获取存档元数据（用于 UI 展示）
    static SaveMeta getMeta(int slot);
    static std::vector<SaveMeta> getAllMetas();

    // 删除存档
    static bool deleteSave(int slot);

private:
    static QString saveFilePath(int slot);
    static QString savesDirectory();
};

#endif // CORE_SAVEMANAGER_H
