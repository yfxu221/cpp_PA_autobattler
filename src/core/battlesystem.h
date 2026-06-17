#ifndef BATTLESYSTEM_H
#define BATTLESYSTEM_H

#include <QObject>
#include <QTimer>
#include <QList>
#include <QSet>
#include <QPoint>
#include "entity/unit.h"
#include "pathfinder.h"
#include "board.h"

struct PlannedAction{
    Unit* unit;
    QPoint targetPos; 
    QSet<Unit*> targetUnits; 
};

struct DamageEvent{
    Unit* source;
    QSet<Unit*> targets;
    int damage;
};

enum class BattleResult {
    Ongoing,
    PlayerWin,
    EnemyWin,
    Draw
};


class Player;

class BattleSystem : public QObject {
    Q_OBJECT

public:
    explicit BattleSystem(QObject *parent = nullptr);

    void start(BoardANDBench& board,
               QList<Unit*>& units,
               Player* player,
               Player* enemy); // 从Game中初始化数据
    void stop();
    bool isRunning() const { return m_timer != nullptr && m_timer->isActive(); }

signals:
    void battleFinished(BattleResult result); // 一方全灭
    void stateUpdated(); // 通知 Game 刷新 UI

private slots:
    void onBattleTick(); // 战斗循环的每一步，更新单位状态，处理攻击等

private:
    Unit* selectTarget(Unit* self, const QList<Unit*>& enemies); // 选择攻击目标，简单起见可以选择最近的敌人
    
    PlannedAction decideAction(Unit* unit);
    void resolveActions(QList<PlannedAction>& actions);
    void resolveDamage(); // 统一处理所有伤害
    void resolveDeaths(); // 统一处理所有死亡事件
    BattleResult checkEndCondition();
    void updateUnits(); 
    void moveAction(QList<PlannedAction>& actions);
    void makeMove(Unit* unit, const QPoint& targetPos);
    void attackAction(QList<PlannedAction>& actions);
    void makeAttack(Unit* attacker, const QSet<Unit*>& targets);
    void skillAction(QList<PlannedAction>& actions);
    void makeSkill(Unit* caster, const QVector<Unit*>& allUnits);

    void processBuffsPreActions();  // tick 开头：DoT 累伤 + duration--
    void processBuffsPostActions(); // tick 末尾：清理过期 buff

    QList<Unit*> getUnitsByOwner(Owner owner) const;
    QSet<QPoint> getOccupiedPositions() const;
    QVector<Unit*> getBoardUnits() const;

    BoardANDBench* m_board = nullptr;
    QList<Unit*>* m_units = nullptr;
    Player* m_player = nullptr;
    Player* m_enemy = nullptr;

    QTimer* m_timer = nullptr;
    int m_tickCount = 0;
    QList<DamageEvent> m_pendingDamageEvents; // 记录当前tick内发生的所有伤害事件，统一处理

    static constexpr int TICK_INTERVAL_MS = 300;
    static constexpr int ATTACK_COOLDOWN_TICKS = 2;
};

#endif // BATTLESYSTEM_H