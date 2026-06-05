#ifndef GUI_ITEMS_UNITITEM_H
#define GUI_ITEMS_UNITITEM_H

#include <QGraphicsObject>
#include <QPoint>
#include <QPixmap>

class Unit;

class UnitItem : public QGraphicsObject // 代表一个单位的图形项
{
    Q_OBJECT

public:
    explicit UnitItem(Unit* unit, QGraphicsItem* parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void drawStatus(QPainter* painter); // 绘制unit生命和mana信息
    void drawStarLevel(QPainter* painter); // 绘制unit星级信息
    void drawAttackValue(QPainter* painter); // 绘制unit攻击力信息
    void drawTeamIndicator(QPainter* painter); // 绘制单位所属队伍的指示器

    Unit* unit() const { return m_unit; } // 获取对应的单位对象
    int unitId() const;

    void setGridPos(const QPoint& gridPos);
    QPoint gridPos() const { return m_gridPos; }

signals:
    void dragStarted(int unitId, const QPoint& sourceGrid, const QPointF& scenePos);
    void dragMoved(int unitId, const QPoint& sourceGrid, const QPointF& scenePos);
    void dragDropped(int unitId, const QPoint& sourceGrid, const QPointF& scenePos);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    void ensureSpriteLoaded() const;
    QString spriteRelativePathForUnit() const;

    Unit* m_unit; // 指向对应的单位对象
    QPoint m_gridPos; // 单位在棋盘上的网格坐标，(row, col)，由Game类在syncFromBoard()中设置
    bool m_dragging; // 是否正在拖动单位图形项，初始值为false，在mousePressEvent中设置为true，在mouseReleaseEvent中设置为false
    mutable QPixmap m_sprite;
    mutable bool m_spriteTried;
};

#endif // GUI_ITEMS_UNITITEM_H
