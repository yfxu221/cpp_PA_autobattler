#include "game.h"
#include "entity/unit.h"
#include "gui/griditem.h"
#include "gui/unititem.h"
#include <QGraphicsScene>
#include <QtMath>

namespace {
constexpr qreal kZGrid = 0.0; // 网格z坐标
constexpr qreal kZUnit = 1.0; // 单位z坐标
constexpr qreal kZDraggingUnit = 2.0; // 拖拽单位z坐标
}

Game::Game(QObject* parent)
    : QObject(parent)
    , m_scene(new QGraphicsScene(this))
    , m_dragActive(false)
    , m_activeUnitId(-1)
    , m_sourceGrid(-1, -1)
    , m_rows(Board::ROWS)
    , m_cols(Board::COLS)
    , m_radius(46.0)
    , m_rowSpacing(69.0)
{}

Game::~Game()
{
    qDeleteAll(m_units);
    m_units.clear();
}

void Game::initialize()
{
    createStarterUnitsIfNeeded();
    buildScene();
    reset();
}

void Game::reset()
{
    m_board.clear();

    const QPoint initialPositions[] = {
        QPoint(0, 7),
        QPoint(1, 7),
        QPoint(2, 7)
    };

    for (int i = 0; i < m_units.size() && i < 3; ++i) {
        m_board.addUnit(m_units.at(i), initialPositions[i]);
    }

    syncFromBoard();
}

void Game::handleDragStarted(int unitId, const QPoint& sourceGrid, const QPointF&)
{
    m_dragActive = true;
    m_activeUnitId = unitId;
    m_sourceGrid = sourceGrid;

    UnitItem* item = findUnitItem(unitId);
    if (item) {
        item->setZValue(kZDraggingUnit); // 提升正在拖动的单位的Z值，使其显示在其他元素之上
    }
}

void Game::handleDragMoved(int unitId, const QPoint&, const QPointF& scenePos)
{
    if (!m_dragActive) {
        return;
    }

    clearGridHighlights();

    const QPoint target = worldToGrid(scenePos);
    GridItem* targetItem = findGridItem(target);
    if (!targetItem) {
        return;
    }

    targetItem->setHoverActive(true);

    if (canApplyDrop(unitId, m_sourceGrid, target)) {
        targetItem->setDropActive(true);
    }
}

void Game::handleDropCommand(int unitId, const QPoint& sourceGrid, const QPointF& scenePos)
{
    if (!m_dragActive) {
        return;
    }

    const QPoint target = worldToGrid(scenePos);

    clearGridHighlights();
    if (canApplyDrop(unitId, sourceGrid, target)) {
        applyDrop(unitId, target);
    }

    UnitItem* item = findUnitItem(m_activeUnitId);
    if (item) {
        item->setZValue(kZUnit);
    }

    m_dragActive = false;
    m_activeUnitId = -1;
    m_sourceGrid = QPoint(-1, -1);

    syncFromBoard();
}

void Game::createStarterUnitsIfNeeded()
{
    if (!m_units.isEmpty()) {
        return;
    }

    m_units.append(new Unit("战术"));
    m_units.append(new Unit("弓手"));
    m_units.append(new Unit("法师"));
}

Unit* Game::findUnitById(int unitId) const
{
    for (Unit* unit : m_units) {
        if (unit && unit->id() == unitId) {
            return unit;
        }
    }
    return nullptr;
}

GridItem* Game::findGridItem(const QPoint& gridPos) const // 根据网格坐标查找对应的GridItem
{
    for (GridItem* item : m_gridItems) {
        if (item && item->gridPos() == gridPos) {
            return item;
        }
    }
    return nullptr;
}

UnitItem* Game::findUnitItem(int unitId) const // 根据单位ID查找对应的UnitItem，使用哈希表进行快速查找
{
    auto it = m_unitItemById.find(unitId); //没找到返回m_unitItemById.end()
    if (it == m_unitItemById.end()) {
        return nullptr;
    }
    return it->second; //取值
}

void Game::clearGridHighlights() // 清除所有格子的高亮状态
{
    for (GridItem* item : m_gridItems) {
        if (!item) {
            continue;
        }
        item->setHoverActive(false);
        item->setDropActive(false);
    }
}

bool Game::canApplyDrop(int unitId, const QPoint& source, const QPoint& target) const
{
    Unit* unit = findUnitById(unitId);
    if (!unit) {
        return false;
    }

    if (!m_board.isValidPosition(source) || !m_board.isValidPosition(target)) {
        return false;
    }

    if (!m_board.isPlayerHalf(source) || !m_board.isPlayerHalf(target)) {
        return false;
    }

    if (source == target || m_board.hasUnitAt(target)) {
        return false;
    }

    return m_board.getUnitAt(source) == unit;
}

void Game::applyDrop(int unitId, const QPoint& target)
{
    Unit* unit = findUnitById(unitId);
    if (!unit) {
        return;
    }

    m_board.removeUnit(unit);
    m_board.addUnit(unit, target);
}

void Game::buildScene() // 根据当前棋盘状态构建图形场景，创建GridItem和UnitItem，并设置它们的位置和层级关系
{   
    // 清除现有场景中的所有项和状态
    m_scene->clear();
    m_gridItems.clear();
    m_unitItems.clear();
    m_unitItemById.clear();

    QRectF totalBounds;
    bool first = true;
    for (int row = 0; row < Board::ROWS; ++row) {
        for (int col = 0; col < Board::COLS; ++col) {
            const QPolygonF poly = cellHexPolygon(row, col);
            GridItem* gridItem = new GridItem(row, col, poly);
            gridItem->setZValue(kZGrid);
            gridItem->setBaseColor(row < Board::ROWS / 2 ? QColor(80, 60, 60) : QColor(60, 60, 80));

            m_scene->addItem(gridItem); // 把格子项添加到场景中
            m_gridItems.push_back(gridItem);

            const QRectF bounds = gridItem->boundingRect();
            totalBounds = first ? bounds : totalBounds.united(bounds);
            first = false;
        }
    }

    for (Unit* unit : m_units) {
        UnitItem* unitItem = new UnitItem(unit);
        unitItem->setZValue(kZUnit);
        m_scene->addItem(unitItem); // 把单位项添加到场景中
        m_unitItems.push_back(unitItem);
        m_unitItemById[unit->id()] = unitItem;

        connect(unitItem, &UnitItem::dragStarted,
                this, &Game::handleDragStarted);
        connect(unitItem, &UnitItem::dragMoved,
                this, &Game::handleDragMoved);
        connect(unitItem, &UnitItem::dragDropped,
                this, &Game::handleDropCommand);
    }

    m_scene->setSceneRect(totalBounds.adjusted(-40, -40, 40, 40));
}

void Game::syncFromBoard() // 根据棋盘状态更新所有单位图形项的位置和可见性
{
    clearGridHighlights();

    for (UnitItem* item : m_unitItems) {
        if (!item || !item->unit()) {
            continue;
        }

        const QPoint pos = item->unit()->position();
        if (!m_board.isValidPosition(pos) || m_board.getUnitAt(pos) != item->unit()) {
            item->setVisible(false);
            continue;
        }

        item->setVisible(true);
        item->setGridPos(pos);
        item->setPos(gridToWorld(pos.y(), pos.x()));
        item->setZValue(kZUnit);
    }
}

QPointF Game::gridToWorld(int row, int col) const // 将网格坐标转换为世界坐标
{
    const qreal colSpacing = m_radius * qSqrt(3.0);
    const qreal xOffset = (row % 2 == 0) ? colSpacing * 0.5 : 0.0;
    const qreal x = xOffset + col * colSpacing;
    const qreal y = row * m_rowSpacing;
    return QPointF(x, y);
}

QPoint Game::worldToGrid(const QPointF& world) const // 将鼠标坐标转换为网格坐标
{
    QPoint best(-1, -1);
    qreal bestDist = 1e18;

    for (int row = 0; row < m_rows; ++row) {
        for (int col = 0; col < m_cols; ++col) {
            const QPointF center = gridToWorld(row, col);
            const qreal dx = world.x() - center.x();
            const qreal dy = world.y() - center.y();
            const qreal d2 = dx * dx + dy * dy;
            if (d2 < bestDist) {
                bestDist = d2;
                best = QPoint(col, row);
            }
        }
    }

    return best;
}

QPolygonF Game::cellHexPolygon(int row, int col) const // 计算指定网格单元的六边形多边形，用于绘制网格单元的形状
{
    const QPointF center = gridToWorld(row, col);
    QPolygonF poly;
    poly.reserve(6);

    for (int i = 0; i < 6; ++i) {
        const qreal angleDeg = 60.0 * i - 90.0;
        const qreal angleRad = qDegreesToRadians(angleDeg);
        poly.append(QPointF(
            center.x() + m_radius * qCos(angleRad),
            center.y() + m_radius * qSin(angleRad)
        ));
    }

    return poly;
}
