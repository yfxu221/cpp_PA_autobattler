#ifndef CORE_GAME_H
#define CORE_GAME_H

#include <QObject>
#include <QList>
#include <QPoint>
#include <QPointF>
#include <QPolygonF>
#include <QHash>
#include <QString>
#include <memory>
#include <unordered_map>
#include <vector>
#include "board.h"
#include "entity/player.h"
#include "battlesystem.h"
#include "store.h"
#include "gui/storeslotitem.h"
#include "entity/skillregistry.h"
#include "entity/synergyregistry.h"
#include "gui/sellzoneitem.h"
#include "gui/equipbar.h"

class Unit;
class QGraphicsScene;
class GridItem;
class UnitItem;
class Player;

enum class GamePhase {
    Preparation,  // 准备阶段：可拖动、可购买
    Battle,       // 战斗阶段：不可操作，看棋子自动战斗
    Settlement    // 结算阶段：短暂提示输赢、金币/血量变化
};

enum class DragStartPlace {
    None,
    Board,
    Bench
};

struct SettlementInfo {
    BattleResult result;
    int playerHpBefore;
    int playerHpAfter;
    int playerGoldBefore;
    int playerGoldAfter;
    int enemyHpBefore;
    int enemyHpAfter;
    int enemyGoldBefore;
    int enemyGoldAfter;
    bool isGameOver; // 是否有人 HP ≤ 0
};



class Game : public QObject
{
    Q_OBJECT

public:
    explicit Game(QObject* parent = nullptr);
    ~Game();

    void initialize();
    void reset();

    QGraphicsScene* scene() const { return m_scene; }

    void handleDragStarted(int unitId, const QPoint& sourceGrid, const QPointF& scenePos);
    void handleDragMoved(int unitId, const QPoint& sourceGrid, const QPointF& scenePos);
    void handleDropCommand(int unitId, const QPoint& sourceGrid, const QPointF& scenePos);

    // 装备拖拽
    void handleEquipDragStarted(int slotIndex, const QPointF& scenePos);
    void handleEquipDragMoved(int slotIndex, const QPointF& scenePos);
    void handleEquipDropCommand(int slotIndex, const QPointF& scenePos);
    void handleEquipUnequip(int unitId, int equipIndex);

    // 阶段相关
    GamePhase phase() const { return m_phase; }
    void startBattle();      // 准备 → 战斗
    void startSettlement();  // 战斗 → 结算
    void startPreparation(); // 结算 → 准备（下一回合）

    Player* player() { return &m_player; }
    Player* enemy() { return &m_enemy; }
    int battleIndex() const { return m_battleIndex; } // 获取当前战斗轮次

    int countFieldUnits(Owner owner) const; // 计算当前棋盘上单位的数量
    QHash<QString, int> getTraitCounts(Owner owner) const; // 获取指定所有者的羁绊计数
    void recalculateSynergies(); // 重新计算所有场上单位的羁绊加成
    int playerMaxFieldUnits() const { return m_player.maxFieldUnits(); } // 获取玩家在棋盘上可以拥有的最大单位数量
    void buyXp(int amount); // 玩家购买经验，增加经验值
    void refreshStore(); // 刷新商店，生成新的单位列表

signals:
    void phaseChanged(GamePhase newPhase);  // 通知 UI 更新
    void stateUpdated(); // 通知 UI 更新状态变化
    void SettlementGUI(); // 通知 UI 显示结算界面
    void settlementReady(SettlementInfo); // 结算信息准备好，可以显示结算界面

private:
    void createStarterUnitsIfNeeded();
    Unit* findUnitById(int unitId) const;
    GridItem* findGridItem(const QPoint& gridPos) const;
    UnitItem* findUnitItem(int unitId) const;
    void clearGridHighlights();
    bool canApplyDrop(int unitId, const QPoint& source, const QPoint& target) const;
    void applyDrop(int unitId, const QPoint& target);
    void buildScene();
    void syncFromBoard();
    void buildStoreScene();
    void populateStore(); // 随机填充商店 5 个单位（共享的随机生成逻辑）

    QPointF gridToWorld(int row, int col) const;
    QPoint worldToGrid(const QPointF& world) const;
    QPolygonF cellHexPolygon(int row, int col) const;

    BoardANDBench m_board; // 棋盘和备战区对象，管理单位的状态和位置
    Store m_store; // 商店对象
    std::vector<std::unique_ptr<Unit>> m_units; // 拥有所有场上单位（棋盘+备战席）的所有权

    QGraphicsScene* m_scene;
    std::vector<GridItem*> m_gridItems; // 存储格子项的指针，方便访问和管理
    std::vector<UnitItem*> m_unitItems; // 存储单位项的指针，方便访问和管理

    bool m_dragActive; // 是否正在拖动
    DragStartPlace m_dragStartPlace; // 拖动开始的位置
    int m_activeUnitId; // 当前正在拖动的单位ID
    QPoint m_sourceGrid; // 拖动开始时的格子位置
    std::unordered_map<int, UnitItem*> m_unitItemById; // 单位ID -> UnitItem的映射，方便快速查找单位对应的图形项

    int m_board_rows;
    int m_board_cols;
    int m_bench_rows;
    int m_bench_cols;
    qreal m_radius; // 浮点数半径，用于计算六边形格子的大小
    qreal m_rowSpacing; // 行间距，通常为半径的1.5倍，但可以根据需要调整以获得更紧凑或更宽松的布局
    qreal m_BBSpacing; // 棋盘和备战区之间的间距，单位为像素，根据需要调整以获得合适的视觉效果
    GamePhase m_phase = GamePhase::Preparation; // 当前游戏阶段，初始值为Preparation
    Player m_player;   // 玩家
    Player m_enemy;    // 敌方
    BattleSystem* m_battleSystem = nullptr; // 战斗系统，负责处理战斗逻辑
    QList<Unit*> m_battleUnits; // 当前参战单位（仅棋盘上的单位），传给BattleSystem

    int m_battleIndex = 0; // 战斗回合计数器，记录当前是第几轮战斗

    SettlementInfo m_settlementInfo; // 结算信息，记录战斗结果和状态变化，供结算界面显示

    std::vector<std::unique_ptr<Unit>> m_unitsSnapshot; // 战斗开始前的单位快照，用于战斗结束后恢复

    // 各阶段的入口逻辑
    void onBattleStart();
    void onSettlementStart();
    void onPreparationStart();
    void onBattleFinished(BattleResult result);

    void onStoreSlotClicked(int index); // 商店格子被点击，index表示格子编号

    QPoint findEmptyBenchSlot() const; // 查找备战区的空位，返回空位的网格坐标，如果没有空位则返回(-1, -1)

    SellZoneItem* m_sellZone = nullptr; // 出售区图形项

    // 装备系统
    EquipBar* m_equipBar = nullptr;            // 装备栏管理器
    bool m_equipDragActive = false;            // 装备拖拽进行中
    int m_activeEquipSlotIndex = -1;           // 当前被拖拽的装备槽位
    QGraphicsPixmapItem* m_equipDragGhost = nullptr; // 拖拽幽灵图标

    bool isOverSellZone(const QPointF& scenePos) const;
    void sellUnit(int unitId);
    void tryMergeStar(Unit* newUnit); // 检查并执行升星合并（三合一）

    void buildEquipBar();              // 构建装备栏 GUI
    UnitItem* findUnitItemAtScenePos(const QPointF& scenePos) const; // 查找场景坐标处的单位图形项


};

#endif // CORE_GAME_H
