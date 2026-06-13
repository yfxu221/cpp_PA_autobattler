#ifndef STOREREFRESHBUTTON_H
#define STOREREFRESHBUTTON_H

#include <QGraphicsObject>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <QTimer>

class StoreRefreshButton : public QGraphicsObject {
    Q_OBJECT
public:
    StoreRefreshButton(const QPointF& center,
                       double width, double height,
                       QGraphicsItem* parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    void setAffordable(bool canAfford);

signals:
    void clicked();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

private:
    QRectF m_rect;
    bool m_hovered = false;
    bool m_pressed = false;
    bool m_affordable = true;
    QTimer* m_pressTimer = nullptr;

    // 颜色常量
    static constexpr QColor kDefaultBg = QColor(50, 50, 60);
    static constexpr QColor kHoverBg = QColor(80, 80, 95);
    static constexpr QColor kPressedBg = QColor(110, 110, 130);
    static constexpr QColor kBorder = QColor(140, 140, 160);
    static constexpr QColor kTextColor = QColor(220, 220, 220);
    static constexpr QColor kTextDimmed = QColor(120, 120, 120);
};

#endif // STOREREFRESHBUTTON_H
