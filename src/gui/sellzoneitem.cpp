#include "sellzoneitem.h"

SellZoneItem::SellZoneItem(const QPointF& center, double width, double height, QGraphicsItem* parent)
    : QGraphicsObject(parent)
    , m_rect(-width / 2.0, -height / 2.0, width, height)
    , m_highlighted(false)
{
    setPos(center);
}

QRectF SellZoneItem::boundingRect() const {
    // 边框外扩 3px，确保边框线不被裁剪
    return m_rect.adjusted(-4, -4, 4, 4);
}

void SellZoneItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    painter->setRenderHint(QPainter::Antialiasing);

    // 背景色
    QColor bg = m_highlighted ? kHighlightBg : kDefaultBg;

    // 圆角矩形背景
    painter->setPen(Qt::NoPen);
    painter->setBrush(bg);
    painter->drawRoundedRect(m_rect, 5, 5);

    // 边框
    painter->setPen(QPen(kBorder, 2.0));
    painter->setBrush(Qt::NoBrush);
    painter->drawRoundedRect(m_rect, 5, 5);

    // 文字 "出售"
    QFont font;
    font.setPointSize(11);
    font.setBold(true);
    painter->setFont(font);
    painter->setPen(kTextColor);
    painter->drawText(m_rect, Qt::AlignCenter, QString::fromUtf8("出售"));
}

void SellZoneItem::setHighlighted(bool on) {
    if (m_highlighted != on) {
        m_highlighted = on;
        update(); // 触发重绘
    }
}

bool SellZoneItem::isHighlighted() const {
    return m_highlighted;
}
