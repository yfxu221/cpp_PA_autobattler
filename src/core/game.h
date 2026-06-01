#ifndef CORE_GAME_H
#define CORE_GAME_H

#include <QObject>
#include <QList>
#include <QPoint>
#include <QPointF>
#include <QPolygonF>
#include <unordered_map>
#include <vector>
#include "board.h"

class Unit;
class QGraphicsScene;
class GridItem;
class UnitItem;

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

    Board m_board; // 棋盘对象，管理单位的状态和位置
    QList<Unit*> m_units; // 存储所有单位的指针，方便访问和管理

    QGraphicsScene* m_scene;
    std::vector<GridItem*> m_gridItems; // 存储格子项的指针，方便访问和管理
    std::vector<UnitItem*> m_unitItems; // 存储单位项的指针，方便访问和管理

    bool m_dragActive; // 是否正在拖动
    int m_activeUnitId; // 当前正在拖动的单位ID
    QPoint m_sourceGrid; // 拖动开始时的格子位置
    std::unordered_map<int, UnitItem*> m_unitItemById; // 单位ID -> UnitItem的映射，方便快速查找单位对应的图形项

    int m_rows;
    int m_cols;
    qreal m_radius; // 浮点数半径，用于计算六边形格子的大小
    qreal m_rowSpacing; // 行间距，通常为半径的1.5倍，但可以根据需要调整以获得更紧凑或更宽松的布局
};

#endif // CORE_GAME_H
