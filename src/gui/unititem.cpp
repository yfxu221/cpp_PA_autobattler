#include "gui/unititem.h"
#include "entity/unit.h"
#include <QCoreApplication>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QFileInfo>

UnitItem::UnitItem(Unit* unit, QGraphicsItem* parent)
    : QGraphicsObject(parent)
    , m_unit(unit)
    , m_gridPos(-1, -1)
    , m_dragging(false)
    , m_spriteTried(false)
{
    setAcceptedMouseButtons(Qt::LeftButton);
}

QRectF UnitItem::boundingRect() const
{
    return QRectF(-42, -42, 84, 84);
}

void UnitItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) // 绘制单位图形项，根据是否成功加载精灵图片来决定绘制内容，如果加载成功则绘制精灵图片，否则绘制一个简单的六边形徽章，并在中心显示单位名称的首字母
{
    painter->setRenderHint(QPainter::Antialiasing); // 启用抗锯齿

    ensureSpriteLoaded();

    if (!m_sprite.isNull()) { // 如果精灵图片已加载成功，则将其绘制在单位图形项的中心位置，目标矩形为(-40, -40, 80, 80)，源矩形为精灵图片的整个区域
        const QRectF targetRect(-40, -40, 80, 80);
        painter->drawPixmap(targetRect, m_sprite, m_sprite.rect());
    }else { // 如果精灵图片未加载成功，则绘制一个简单的六边形徽章，底色为半透明的深灰色，边框为较亮的蓝色，内部填充为较亮的蓝色，并在中心显示单位名称的首字母，使用白色字体，字号为12，粗体
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(20, 20, 20, 110));
        painter->drawEllipse(QRectF(-14, 8, 28, 10));

        QPolygonF badge; // 定义一个六边形徽章，顶点坐标相对于单位图形项的中心，形成一个宽约26像素、高约30像素的六边形
        badge << QPointF(0, -15)
            << QPointF(13, -7)
            << QPointF(13, 7)
            << QPointF(0, 15)
            << QPointF(-13, 7)
            << QPointF(-13, -7);

        painter->setPen(QPen(QColor(18, 18, 18), 1.5));
        painter->setBrush(QColor(100, 150, 200));
        painter->drawPolygon(badge);

        if (m_unit) { // 如果单位对象存在，则在徽章中心绘制单位名称的首字母，使用白色字体，字号为12，粗体
            painter->setPen(Qt::white);
            QFont font = painter->font();
            font.setPointSize(12);
            font.setBold(true);
            painter->setFont(font);
            painter->drawText(QRectF(-13, -13, 26, 26), Qt::AlignCenter, m_unit->name().left(1));
        }
    }
    if(m_unit) { // 画单位血量、mana、attack、敌我方等信息
        drawStarLevel(painter);
        drawAttackValue(painter);
        drawTeamIndicator(painter);
        drawStatus(painter);
    }
}

void UnitItem::drawStatus(QPainter* painter)
{
    if (!m_unit) return;

    int hp = m_unit->hp();
    int maxHp = m_unit->maxHp();
    int mana = m_unit->mana();
    int maxMana = m_unit->maxMana();

    QFont font;
    font.setPointSize(7);
    painter->setFont(font);

    QFontMetrics metrics(font);
    QString hpText = QString("HP:%1/%2").arg(hp).arg(maxHp);
    QString manaText = QString("MP:%1/%2").arg(mana).arg(maxMana);

    int hpW = metrics.horizontalAdvance(hpText);
    int manaW = maxMana > 0 ? metrics.horizontalAdvance(manaText) : 0;
    int textW = qMax(hpW, manaW);
    int textH = metrics.height();
    int lineH = textH + 3;

    int margin = 3;
    qreal bgX = -45 + margin;
    qreal bgY = -40 + margin;
    qreal bgW = textW + margin * 2;
    qreal bgH = (maxMana > 0 ? lineH * 2 : lineH) + margin;

    QRectF bgRect(bgX, bgY, bgW, bgH);

    // 半透明圆角背景
    painter->setBrush(QColor(0, 0, 0, 160));
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(bgRect, 3, 3);

    painter->setPen(QColor(255, 120, 120));
    painter->drawText(QRectF(bgX, bgY, bgW, lineH), Qt::AlignCenter, hpText);

    if (maxMana > 0) {
        painter->setPen(QColor(120, 160, 255));
        painter->drawText(QRectF(bgX, bgY + lineH, bgW, lineH), Qt::AlignCenter, manaText);
    }
}


void UnitItem::drawStarLevel(QPainter* painter)
{
    if (!m_unit) {
        return;
    }
    int starLevel = m_unit->starLevel();
    if (starLevel <= 0) return;

    QString starText(starLevel, QChar(0x2605));
    QFont font;
    font.setPointSize(10);
    painter->setFont(font);

    QFontMetrics metrics(font);
    int textW = metrics.horizontalAdvance(starText); // 获取文本宽度
    int textH = metrics.height();

    int margin = 3;
    QRectF bgRect(35 - textW - margin, -40 + margin, textW + margin*2, textH);

    // 半透明圆角背景
    painter->setBrush(QColor(180,180,180,160));
    painter->setPen(Qt::NoPen); // 无边框
    painter->drawRoundedRect(bgRect, 3,3); //圆角矩形

    painter->setPen(QColor(255, 215, 0)); // 设为金色笔
    painter->drawText(bgRect, Qt::AlignCenter, starText); // 绘制星级文本
}

void UnitItem::drawAttackValue(QPainter* painter)
{
    if (!m_unit) {
        return;
    }
    int atk = m_unit->atk();
    if (atk <= 0) return;

    QString atkText = QString("ATK: %1").arg(atk);
    QFont font;
    font.setPointSize(7);
    painter->setFont(font);

    QFontMetrics metrics(font);
    int textW = metrics.horizontalAdvance(atkText); // 获取文本宽度
    int textH = metrics.height();

    int margin = 3;
    QRectF bgRect(-40 + margin, 35 - textH - margin, textW + margin*2, textH);

    // 半透明圆角背景
    painter->setBrush(QColor(0,0,0,160));
    painter->setPen(Qt::NoPen); // 无边框
    painter->drawRoundedRect(bgRect, 3,3); //圆角矩形

    painter->setPen(Qt::red); // 红色字体
    painter->drawText(bgRect, Qt::AlignCenter, atkText); // 绘制攻击力文本
}

void UnitItem::drawTeamIndicator(QPainter* painter)
{
    if (!m_unit) {
        return;
    }
    Owner owner = m_unit->owner();

    int margin = 3;
    int r = 5;

    // 背景框
    QRectF bgRect(32 - r*2 - margin, 28 - r*2 - margin, r*2 + margin*2, r*2 + margin*2);
    painter->setBrush(QColor(180,180,180,160));
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(bgRect, 3, 3);

    // 圆点本身
    painter->setBrush(owner == Owner::PlayerCtrl ? Qt::blue : Qt::red);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(bgRect.center(), r, r);
}

void UnitItem::ensureSpriteLoaded() const // 确保单位图形项的精灵图片已加载，如果尚未尝试加载过，则根据单位类型构建相对路径并尝试加载图片，加载成功后将其缩放到适当大小
{
    if (m_spriteTried) {
        return;
    }

    m_spriteTried = true;
    const QString relativePath = spriteRelativePathForUnit();
    if (relativePath.isEmpty()) {
        return;
    }

    const QString appDir = QCoreApplication::applicationDirPath();
    const QString roots[] = {
        QFileInfo(appDir + "/..").canonicalFilePath(),
        QFileInfo(appDir + "/../..").canonicalFilePath()
    };

    QPixmap pix;
    for (const QString& root : roots) {
        if (root.isEmpty()) {
            continue;
        }
        pix.load(root + "/" + relativePath);
        if (!pix.isNull()) {
            break;
        }
    }

    if (pix.isNull()) {
        return;
    }

    m_sprite = pix.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

QString UnitItem::spriteRelativePathForUnit() const
{
    if (!m_unit) {
        return QString();
    }

    const QString path = m_unit->spritePath();
    if (!path.isEmpty()) {
        return path;
    }

    // 回退：旧硬编码映射，兼容没有 sprite 数据的旧单位
    const QString name = m_unit->name();
    if (name == QString::fromUtf8("战术")) {
        return QStringLiteral("assets/craftpix-reaper-man-chibi-2d-game-sprites/Reaper_Man_1/PNG/PNG Sequences/Idle/0_Reaper_Man_Idle_000.png");
    }
    if (name == QString::fromUtf8("弓手")) {
        return QStringLiteral("assets/craftpix-satyr-tiny-style-2d-sprites/PNG/Satyr_01/PNG Sequences/Idle/Satyr_01_Idle_000.png");
    }
    if (name == QString::fromUtf8("法师")) {
        return QStringLiteral("assets/craftpix-wraith-tiny-style-2d-sprites/PNG/Wraith_02/PNG Sequences/Idle/Wraith_02_Idle_000.png");
    }

    return QString();
}

int UnitItem::unitId() const
{
    return m_unit ? m_unit->id() : -1;
}

void UnitItem::setGridPos(const QPoint& gridPos) // 设置单位图形项的网格坐标
{
    m_gridPos = gridPos;
}

void UnitItem::mousePressEvent(QGraphicsSceneMouseEvent* event) // 处理鼠标按下事件，如果按下的是左键，则开始拖拽单位图形项，并发出dragStarted信号，传递单位ID、源网格坐标和场景坐标
{
    if (event->button() != Qt::LeftButton) {
        QGraphicsObject::mousePressEvent(event);
        return;
    }

    m_dragging = true;
    emit dragStarted(unitId(), m_gridPos, event->scenePos());
    event->accept();
}

void UnitItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (!m_dragging) {
        QGraphicsObject::mouseMoveEvent(event);
        return;
    }

    emit dragMoved(unitId(), m_gridPos, event->scenePos());
    event->accept();
}

void UnitItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (!m_dragging || event->button() != Qt::LeftButton) {
        QGraphicsObject::mouseReleaseEvent(event);
        return;
    }

    m_dragging = false;
    emit dragDropped(unitId(), m_gridPos, event->scenePos());
    event->accept();
}
