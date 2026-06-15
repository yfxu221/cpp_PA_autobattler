#include "gui/equipslotitem.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPen>

EquipSlotItem::EquipSlotItem(int index, const QPointF& topLeft, double size, QGraphicsItem* parent)
    : QGraphicsObject(parent)
    , m_index(index)
    , m_size(size)
    , m_radius(6.0)
{
    setPos(topLeft);
    setAcceptedMouseButtons(Qt::LeftButton);
    setAcceptHoverEvents(true);
}

QRectF EquipSlotItem::boundingRect() const
{
    return QRectF(0, 0, m_size, m_size);
}

void EquipSlotItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->setRenderHint(QPainter::Antialiasing);

    // 确定底色
    QColor fill;
    if (m_draggingOut) {
        fill = kDragOutColor;
    } else if (m_hovered && !isEmpty()) {
        fill = kHoverColor;
    } else if (!isEmpty()) {
        fill = kFilledColor;
    } else {
        fill = kEmptyColor;
    }

    // 绘制圆角正方形背景
    painter->setPen(QPen(kBorderColor, 2.0));
    painter->setBrush(fill);
    painter->drawRoundedRect(boundingRect(), m_radius, m_radius);

    if (isEmpty()) return;

    // 有装备：尝试绘制图标
    ensureSpriteLoaded();

    const double iconMargin = 4.0;
    const QRectF iconRect(iconMargin, iconMargin, m_size - iconMargin * 2, m_size - iconMargin * 2);

    if (!m_sprite.isNull()) {
        painter->drawPixmap(iconRect.toRect(), m_sprite);
    } else {
        // 回退：圆角矩形 + 装备名首字
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(80, 80, 100));
        painter->drawRoundedRect(iconRect, 4, 4);

        if (m_equipment && !m_equipment->name.isEmpty()) {
            painter->setPen(Qt::white);
            QFont font = painter->font();
            font.setPointSize(12);
            font.setBold(true);
            painter->setFont(font);
            painter->drawText(iconRect, Qt::AlignCenter, m_equipment->name.left(1));
        }
    }
}

void EquipSlotItem::setEquipment(std::shared_ptr<Equipment> eq)
{
    m_equipment = std::move(eq);
    m_sprite = QPixmap();
    m_spriteTried = false;
    m_draggingOut = false;
    update();
}

void EquipSlotItem::setDraggingOut(bool dragging)
{
    m_draggingOut = dragging;
    update();
}


void EquipSlotItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) {
        QGraphicsObject::mousePressEvent(event);
        return;
    }

    m_dragging = true;
    emit equipDragStarted(m_index, event->scenePos());
    event->accept();
}

void EquipSlotItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (!m_dragging) {
        QGraphicsObject::mouseMoveEvent(event);
        return;
    }

    emit equipDragMoved(m_index, event->scenePos());
    event->accept();
}

void EquipSlotItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (!m_dragging || event->button() != Qt::LeftButton) {
        QGraphicsObject::mouseReleaseEvent(event);
        return;
    }

    m_dragging = false;
    emit equipDragDropped(m_index, event->scenePos());
    event->accept();
}

void EquipSlotItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    m_hovered = true;
    update();
}

void EquipSlotItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    m_hovered = false;
    update();
}


void EquipSlotItem::ensureSpriteLoaded() const
{
    if (m_spriteTried) return;
    m_spriteTried = true;

    if (!m_equipment || m_equipment->spritePath.isEmpty()) return;

    const QString appDir = QCoreApplication::applicationDirPath();
    const QString roots[] = {
        QFileInfo(appDir + "/..").canonicalFilePath(),
        QFileInfo(appDir + "/../..").canonicalFilePath()
    };

    for (const QString& root : roots) {
        if (root.isEmpty()) continue;
        const QString fullPath = root + "/" + m_equipment->spritePath;
        QPixmap pix;
        pix.load(fullPath);
        if (!pix.isNull()) {
            m_sprite = pix.scaled(static_cast<int>(m_size - 8),
                                  static_cast<int>(m_size - 8),
                                  Qt::KeepAspectRatio,
                                  Qt::SmoothTransformation);
            return;
        }
    }
}
