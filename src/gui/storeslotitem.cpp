#include "storeslotitem.h"
#include <QPainter>
#include <QPen>
#include <QFont>
#include <QGraphicsScene>

StoreSlotItem::StoreSlotItem(int index, const QPointF& center, double radius, QGraphicsItem* parent)
    : QGraphicsObject(parent), m_index(index), m_occupied(false), m_radius(radius)
{
    // polygon 以 (0,0) 为中心（局部坐标）
    for (int i = 0; i < 6; ++i) {
        double angle = qDegreesToRadians(60.0 * i - 90.0);
        m_polygon << QPointF(m_radius * std::cos(angle),
                             m_radius * std::sin(angle));
    }
    setPos(center); // 将图形项移动到指定中心位置

    setAcceptHoverEvents(true); // 允许接收悬停事件以改变外观
    
}

void StoreSlotItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    painter->setRenderHint(QPainter::Antialiasing); // 开启抗锯齿

    QColor fillColor;
    QColor borderColor;
    if(m_pressed) {
        // 按下：填充更亮 + 亮白色边框
        fillColor = (m_occupied ? kOccupiedColor : kEmptyColor).lighter(150);
        borderColor = QColor(255, 255, 255);
    } else if (m_hovered) {
        // 悬停：填充变亮 + 白色边框
        fillColor = (m_occupied ? kOccupiedColor : kEmptyColor).lighter(120);
        borderColor = QColor(220, 220, 220);
    } else if (m_occupied) {
        fillColor = kOccupiedColor;
        borderColor = kOccupiedBorder;
    } else {
        fillColor = kEmptyColor;
        borderColor = kBorderColor;
    }

    painter->setBrush(fillColor);
    painter->setPen(QPen(borderColor, 2.0));
    painter->drawPolygon(m_polygon);

    // 价格标签（有单位时显示在六边形右侧）
    if (m_unitItem && m_unitItem->unit()) {
        const int price = m_unitItem->unit()->price();
        const QRectF tagRect(m_radius + kPriceTagGap,
                             -kPriceTagH / 2.0,
                             kPriceTagW, kPriceTagH);

        painter->setPen(QPen(kPriceTagBorder, 1.5));
        painter->setBrush(kPriceTagBg);
        painter->drawRoundedRect(tagRect, 4, 4);

        QFont font;
        font.setPointSize(10);
        font.setBold(true);
        painter->setFont(font);
        painter->setPen(kPriceTagText);
        painter->drawText(tagRect, Qt::AlignCenter,
                          QString::fromUtf8("💰 %1").arg(price));
    }

    // 悬停时加粗边框
    if (m_hovered) {
        painter->setPen(QPen(borderColor, 3.0));
        painter->setBrush(Qt::NoBrush);
        painter->drawPolygon(m_polygon);
    }
}

void StoreSlotItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_pressed = true;
        update();
        emit clicked(m_index);
        m_pressTimer = new QTimer(this);
        m_pressTimer->setSingleShot(true);
        connect(m_pressTimer, &QTimer::timeout, this, [this]() {
            m_pressed = false;
            update();
        });
        m_pressTimer->start(100); // 100ms后自动释放
    }
}

void StoreSlotItem::setOccupied(bool occupied) {
    m_occupied = occupied;
    update(); // 触发重绘以更新外观
}

QRectF StoreSlotItem::boundingRect() const {
    const double m = 2.0; // 边框 margin
    return QRectF(-m_radius - m, -m_radius - m,
                  (m_radius + m) * 2 + kPriceTagGap + kPriceTagW + m,
                  (m_radius + m) * 2);
}

void StoreSlotItem::setHoverActive(bool active) {

}

void StoreSlotItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
    m_hovered = true;
    update();
}

void StoreSlotItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
    m_hovered = false;
    update();
}

void StoreSlotItem::setUnit(Unit* unit) {
    // 清理旧的
    if (m_unitItem) {
        m_unitItem->setParentItem(nullptr);
        if (scene()) scene()->removeItem(m_unitItem);
        delete m_unitItem;
        m_unitItem = nullptr;
    }

    if (unit) {
        m_unitItem = new UnitItem(unit, this);  // parent = StoreSlotItem
        m_unitItem->setAcceptedMouseButtons(Qt::NoButton);  // 鼠标穿透
        m_unitItem->setAcceptHoverEvents(false);
        m_unitItem->setPos(0, 0);  // 居中于六边形
    }

    m_occupied = (unit != nullptr);
    update();  // 重绘背景（空/占用颜色不同）
}
