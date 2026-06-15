#ifndef SELLZONEITEM_H
#define SELLZONEITEM_H

#include <QGraphicsObject>
#include <QPainter>

class SellZoneItem : public QGraphicsObject {
    Q_OBJECT
public:
    explicit SellZoneItem(const QPointF& center, double width, double height,
                          QGraphicsItem* parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    // bool containsScenePos(const QPointF& scenePos) const;
    void setHighlighted(bool on);
    bool isHighlighted() const;
private:
    QRectF m_rect; // 局部坐标下的矩形
    bool m_highlighted;
    // 颜色常量
    static constexpr QColor kDefaultBg = QColor(80, 40, 40, 200); // 默认的暗红色背景
    static constexpr QColor kHighlightBg = QColor(200, 60, 60, 220); // 鼠标悬停时的亮红色背景
    static constexpr QColor kBorder = QColor(100, 100, 100, 200); // 深灰色边框
    static constexpr QColor kTextColor = QColor(255, 255, 255); // 白色文字
};


#endif // SELLZONEITEM_H