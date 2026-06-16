#ifndef ENEMYSPAWNER_H
#define ENEMYSPAWNER_H

#include <QVector>
#include <QPoint>
#include <QString>
#include "board.h"
#include <QJsonObject>

enum class UnitRole {
    Frontline, // 前排：坦克、战士
    Midline, // 中排：刺客
    Backline // 后排：射手、辅助
};


struct EnemySpawnPlan {
    QString unitKey; // 要生成的敌人单位的模板key
    int starLevel; // 要生成的敌人单位的星级
    QPoint position; // 生成位置，使用网格坐标表示（列, 行）
    QStringList equipmentKeys; // 该敌人预设装备的 key 列表
};

class EnemySpawner {
public:
    static EnemySpawner* instance();

    // 从 JSON 文件加载所有技能定义
    bool load(const QString& jsonPath);

    // 根据当前战斗索引和棋盘状态生成敌人单位的生成计划列表
    QVector<EnemySpawnPlan>  generateEnemy(int battleIndex, const BoardANDBench& board);

private:
    struct StageConfig {
        int battleIndex;
        int minUnits;
        int maxUnits;
        double star1Percent;
        double star2Percent;
        double star3Percent;
        double equipChance;
    };

    EnemySpawner() = default;
    ~EnemySpawner() = default;

    QVector<EnemySpawnPlan> decideEnemy(int battleIndex); // 根据战斗索引和配置决定生成哪些敌人（不考虑棋盘状态）
    QVector<EnemySpawnPlan> planFormation(QVector<EnemySpawnPlan>& initialPlans, const BoardANDBench& board); // 生成阵型

    QMap<int, StageConfig> m_stageConfigs; // 每个战斗阶段的配置列表

};


#endif // ENEMYSPAWNER_H