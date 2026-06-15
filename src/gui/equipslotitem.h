#ifndef GUI_EQUIPSLOTITEM_H
#define GUI_EQUIPSLOTITEM_H

#include <QGraphicsObject>
#include <QPixmap>
#include <memory>
#include "entity/equipment.h"

// 装备栏单个槽位
class EquipSlotItem : public QGraphicsObject
{
    Q_OBJECT

public:
    EquipSlotItem(int index, const QPointF& topLeft, double size, QGraphicsItem* parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    int index() const { return m_index; }

    // 装备管理
    void setEquipment(std::shared_ptr<Equipment> eq);
    std::shared_ptr<Equipment> equipment() const { return m_equipment; }
    bool isEmpty() const { return m_equipment == nullptr; }

    // 拖拽视觉状态
    void setDraggingOut(bool dragging); // 装备正在被拖出时，槽位变半透明

signals:
    void equipDragStarted(int slotIndex, const QPointF& scenePos);
    void equipDragMoved(int slotIndex, const QPointF& scenePos);
    void equipDragDropped(int slotIndex, const QPointF& scenePos);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

private:
    void ensureSpriteLoaded() const;

    int m_index;
    double m_size; // 槽位边长
    double m_radius; // 圆角半径
    std::shared_ptr<Equipment> m_equipment;

    bool m_dragging = false; // 当前槽位正在被拖拽
    bool m_draggingOut = false; // 装备已被拖出（半透明态）
    bool m_hovered = false;

    // 图缓存
    mutable QPixmap m_sprite;
    mutable bool m_spriteTried = false;

    // 颜色常量
    static constexpr QColor kEmptyColor   = QColor(50, 50, 55);
    static constexpr QColor kFilledColor  = QColor(65, 65, 75);
    static constexpr QColor kBorderColor  = QColor(120, 120, 140);
    static constexpr QColor kHoverColor   = QColor(90, 90, 100);
    static constexpr QColor kDragOutColor = QColor(50, 50, 55, 90);
};

#endif // GUI_EQUIPSLOTITEM_H
