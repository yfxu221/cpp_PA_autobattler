#include "gui/storerefreshbutton.h"
#include <QPainter>
#include <QPen>
#include <QFont>
#include <QtMath>

StoreRefreshButton::StoreRefreshButton(const QPointF& center,
                                       double width, double height,
                                       QGraphicsItem* parent)
    : QGraphicsObject(parent)
    , m_rect(-width / 2.0, -height / 2.0, width, height)
{
    setPos(center);
    setAcceptHoverEvents(true);
}

QRectF StoreRefreshButton::boundingRect() const {
    // 边框外扩 3px，确保边框线不被裁剪
    return m_rect.adjusted(-4, -4, 4, 4);
}

void StoreRefreshButton::paint(QPainter* painter,
                               const QStyleOptionGraphicsItem*,
                               QWidget*) {
    painter->setRenderHint(QPainter::Antialiasing);

    // 背景色
    QColor bg;
    QColor border = kBorder;
    QColor text  = m_affordable ? kTextColor : kTextDimmed;

    if (m_pressed) {
        bg = kPressedBg;
        border = QColor(255, 255, 255);
    } else if (m_hovered) {
        bg = kHoverBg;
        border = QColor(220, 220, 220);
    } else {
        bg = m_affordable ? kDefaultBg : kDefaultBg.darker(130);
    }

    // 圆角矩形背景
    painter->setPen(Qt::NoPen);
    painter->setBrush(bg);
    painter->drawRoundedRect(m_rect, 5, 5);

    // 边框
    painter->setPen(QPen(border, 2.0));
    painter->setBrush(Qt::NoBrush);
    painter->drawRoundedRect(m_rect, 5, 5);

    // 文字 "刷新 💰2"
    QFont font;
    font.setPointSize(11);
    font.setBold(true);
    painter->setFont(font);
    painter->setPen(text);
    painter->drawText(m_rect, Qt::AlignCenter,
                      QString::fromUtf8("\342\231\273 刷新  2"));
}

void StoreRefreshButton::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() != Qt::LeftButton) {
        QGraphicsObject::mousePressEvent(event);
        return;
    }

    m_pressed = true;
    update();
    emit clicked();

    // 短暂按下反馈后恢复
    if (!m_pressTimer) {
        m_pressTimer = new QTimer(this);
        m_pressTimer->setSingleShot(true);
        connect(m_pressTimer, &QTimer::timeout, this, [this]() {
            m_pressed = false;
            update();
        });
    }
    m_pressTimer->start(100);
    event->accept();
}

void StoreRefreshButton::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
    m_hovered = true;
    update();
    QGraphicsObject::hoverEnterEvent(event);
}

void StoreRefreshButton::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
    m_hovered = false;
    update();
    QGraphicsObject::hoverLeaveEvent(event);
}

void StoreRefreshButton::setAffordable(bool canAfford) {
    if (m_affordable != canAfford) {
        m_affordable = canAfford;
        update();
    }
}
