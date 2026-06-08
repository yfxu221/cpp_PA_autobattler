#ifndef CORE_GAME_H
#define CORE_GAME_H

#include <QObject>
#include <QList>
#include <QPoint>
#include <QPointF>
#include <QPolygonF>
#include <QHash>
#include <QString>
#include <unordered_map>
#include <vector>
#include "board.h"
#include "entity/player.h"

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

    // 阶段相关
    GamePhase phase() const { return m_phase; }
    void startBattle();      // 准备 → 战斗
    void startSettlement();  // 战斗 → 结算
    void startPreparation(); // 结算 → 准备（下一回合）

    Player* player() { return &m_player; }

    int countFieldUnits(Owner owner) const; // 计算当前棋盘上单位的数量
    QHash<QString, int> getTraitCounts(Owner owner) const; // 获取指定所有者的羁绊计数
    int playerMaxFieldUnits() const { return m_player.maxFieldUnits(); } // 获取玩家在棋盘上可以拥有的最大单位数量
    void buyXp(int amount); // 玩家购买经验，增加经验值

signals:
    void phaseChanged(GamePhase newPhase);  // 通知 UI 更新
    void stateUpdated(); // 通知 UI 更新状态变化

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

    QPointF gridToWorld(int row, int col) const;
    QPoint worldToGrid(const QPointF& world) const;
    QPolygonF cellHexPolygon(int row, int col) const;

    BoardANDBench m_board; // 棋盘和备战区对象，管理单位的状态和位置
    QList<Unit*> m_units; // 存储所有单位的指针，方便访问和管理

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

    // 各阶段的入口逻辑
    void onBattleStart();
    void onSettlementStart();
    void onPreparationStart();
    void runBattleLoop();  // 战斗模拟

};

#endif // CORE_GAME_H
