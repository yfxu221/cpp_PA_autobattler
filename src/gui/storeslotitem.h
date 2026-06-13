#ifndef STORESLOTITEM_H
#define STORESLOTITEM_H

#include <QGraphicsObject>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include "entity/unit.h"
#include "gui/unititem.h"
#include <QTimer>

class StoreSlotItem : public QGraphicsObject {
    Q_OBJECT
public:
    StoreSlotItem(int index, const QPointF& center, double radius, QGraphicsItem* parent = nullptr);

    QRectF boundingRect() const override; // 包含整个六边形和边框的矩形
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    void setOccupied(bool occupied);  // 切换空/有unit的外观
    int index() const { return m_index; }
    void setHoverActive(bool active);
    void setUnit(Unit* unit); // 设置当前占用该槽位的单位指针，nullptr表示没有单位
    Unit* unit() const {return m_unitItem ? m_unitItem->unit() : nullptr;} // 获取当前占用该槽位的单位指针，如果没有单位则返回nullptr

signals:
    void clicked(int index);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

private:
    int m_index;           // 槽位编号 0~4
    QPolygonF m_polygon;   // 预计算的六边形顶点
    bool m_occupied;       // 是否被占用
    double m_radius;      // 六边形半径，决定大小
    bool m_hovered = false; // 鼠标悬停状态
    bool m_pressed = false; // 鼠标按下状态
    UnitItem* m_unitItem = nullptr; // 显示单位的图形项，如果有单位则指向它，否则为nullptr
    QTimer* m_pressTimer; // 用于处理点击时的视觉反馈，按下后短暂保持按下状态，增强用户体验

    // 颜色常量
    static constexpr QColor kEmptyColor = QColor(50, 50, 60);       // 空位：深灰蓝
    static constexpr QColor kOccupiedColor = QColor(60, 70, 90);    // 占用：稍亮
    static constexpr QColor kBorderColor = QColor(140, 140, 160);   // 边框
    static constexpr QColor kOccupiedBorder = QColor(200, 200, 80); // 占用时边框偏金
    static constexpr QColor kHoverColor = QColor(220, 220, 220);   // 悬停时的高亮色

    // 价格标签常量
    static constexpr double kPriceTagW = 50.0;
    static constexpr double kPriceTagH = 22.0;
    static constexpr double kPriceTagGap = 6.0;
    static constexpr QColor kPriceTagBg = QColor(220, 215, 180, 220);
    static constexpr QColor kPriceTagBorder = QColor(180, 170, 130);
    static constexpr QColor kPriceTagText = QColor(60, 50, 30);
};


#endif // STORESLOTITEM_H