#include "gui/lootdialog.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QVBoxLayout>


EquipIconWidget::EquipIconWidget(std::shared_ptr<Equipment> eq, int id, double size,
                                 ClickCallback cb, QWidget* parent)
    : QWidget(parent)
    , m_equipment(std::move(eq))
    , m_id(id)
    , m_size(size)
    , m_radius(6.0)
    , m_callback(std::move(cb))
{
    setFixedSize(static_cast<int>(m_size), static_cast<int>(m_size));
}

void EquipIconWidget::setSelected(bool sel)
{
    if (m_selected != sel) {
        m_selected = sel;
        update();
    }
}

void EquipIconWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 底色
    const QColor fill = isEmpty() ? kEmptyColor : kFilledColor;

    // 边框
    QColor border = kBorderColor;
    if (m_selected) {
        border = kSelectBorder;
        painter.setBrush(QColor(90, 40, 40)); // 选中：红色底色
    } else {
        painter.setBrush(fill);
    }
    painter.setPen(QPen(border, m_selected ? 2.5 : 1.5));
    painter.drawRoundedRect(QRectF(1, 1, m_size - 2, m_size - 2), m_radius, m_radius);

    if (isEmpty()) return;

    // 装备图标
    ensureSpriteLoaded();

    const double iconMargin = 4.0;
    const QRectF iconRect(iconMargin, iconMargin, m_size - iconMargin * 2, m_size - iconMargin * 2);

    if (!m_sprite.isNull()) {
        painter.drawPixmap(iconRect.toRect(), m_sprite);
    } else {
        // 回退：小圆角矩形 + 首字
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(80, 80, 100));
        painter.drawRoundedRect(iconRect, 4, 4);

        if (m_equipment && !m_equipment->name.isEmpty()) {
            painter.setPen(Qt::white);
            QFont font = painter.font();
            font.setPointSize(12);
            font.setBold(true);
            painter.setFont(font);
            painter.drawText(iconRect, Qt::AlignCenter, m_equipment->name.left(1));
        }
    }

    // 选中时叠加半透明红色遮罩
    if (m_selected) {
        painter.setBrush(QColor(255, 0, 0, 40));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(QRectF(1, 1, m_size - 2, m_size - 2), m_radius, m_radius);
    }
}

void EquipIconWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_callback) {
        m_callback(m_id);
    }
    QWidget::mousePressEvent(event);
}

void EquipIconWidget::ensureSpriteLoaded() const
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
            const int iconSize = static_cast<int>(m_size - 8);
            m_sprite = pix.scaled(iconSize, iconSize,
                                  Qt::KeepAspectRatio,
                                  Qt::SmoothTransformation);
            return;
        }
    }
}



LootDialog::LootDialog(const std::vector<std::shared_ptr<Equipment>>& newItems,
                       const std::vector<std::shared_ptr<Equipment>>& barEquipments,
                       LootContext context,
                       QWidget* parent)
    : QDialog(parent)
    , m_newItems(newItems)
    , m_barEquipments(barEquipments)
    , m_context(context)
{
    m_newCount = static_cast<int>(m_newItems.size());
    m_barCount = static_cast<int>(m_barEquipments.size());

    buildUI();
}

void LootDialog::buildUI()
{
    const bool isSettlement = (m_context == LootContext::Settlement);
    const QString titleText = isSettlement
        ? QStringLiteral("\xF0\x9F\x8E\x81 掉落的装备")   // 🎁
        : QStringLiteral("\xF0\x9F\x93\xA6 卸下的装备");  // 📦

    // 弹窗大小自适应
    constexpr double kSlotSize = 52.0;
    constexpr int kSlotSpacing = 8;
    constexpr int kCols = 4;
    const int newRows = (m_newCount + kCols - 1) / kCols;
    const int barRows = (m_barCount + kCols - 1) / kCols;

    const int contentW = kCols * static_cast<int>(kSlotSize) + (kCols - 1) * kSlotSpacing;
    const int contentH = 50 /*标题*/ + newRows * 60 + 30 /*分隔*/ + barRows * 60 + 40 /*状态*/ + 50 /*按钮*/ + 50;
    const int dlgW = std::max(contentW + 40, 280);
    setFixedSize(dlgW, contentH);

    setWindowTitle(isSettlement
        ? QStringLiteral("战利品 — 掉落的装备")
        : QStringLiteral("战利品 — 卸下的装备"));

    setStyleSheet(R"(
        QDialog {
            background-color: #2b2b2b;
            color: #f2f2f2;
        }
        QGroupBox {
            background-color: #333333;
            border: 1px solid #565656;
            border-radius: 6px;
            margin-top: 8px;
            padding-top: 14px;
            color: #f2f2f2;
            font-size: 12px;
            font-weight: bold;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 4px;
        }
        QLabel {
            color: #d0d0d0;
            font-size: 13px;
            background: transparent;
        }
        QPushButton {
            background-color: #3a3a3a;
            color: #f2f2f2;
            border: 1px solid #565656;
            border-radius: 4px;
            padding: 8px 24px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #4a4a4a;
        }
        QPushButton:disabled {
            background-color: #252525;
            color: #808080;
        }
    )");

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(16, 12, 16, 12);

    // 标题
    auto* title = new QLabel(titleText, this);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 16px; font-weight: bold; color: #ffcc66;");
    mainLayout->addWidget(title);

    // 新装备区域（网格布局，最多 4 列）
    auto* newGroup = new QGroupBox(
        isSettlement ? QStringLiteral("⚡ 新掉落的装备") : QStringLiteral("⚡ 卸下的装备"), this);
    auto* newGrid = new QGridLayout();
    newGrid->setSpacing(kSlotSpacing);
    newGrid->setAlignment(Qt::AlignCenter);

    for (int i = 0; i < m_newCount; ++i) {
        auto* icon = new EquipIconWidget(m_newItems[i], i, kSlotSize,
            [this](int id) { onIconClicked(id); }, this);
        m_newIcons.push_back(icon);
        newGrid->addWidget(icon, i / kCols, i % kCols, Qt::AlignCenter);
    }
    newGroup->setLayout(newGrid);
    mainLayout->addWidget(newGroup);

    // 装备栏区域（4 列网格）
    auto* barGroup = new QGroupBox(QStringLiteral("🗃 当前装备栏（点击选中丢弃）"), this);
    auto* barLayout = new QGridLayout();
    barLayout->setSpacing(kSlotSpacing);
    barLayout->setAlignment(Qt::AlignCenter);

    for (int i = 0; i < m_barCount; ++i) {
        const int id = m_newCount + i; // id 偏移
        auto* icon = new EquipIconWidget(m_barEquipments[i], id, kSlotSize,
            [this](int id) { onIconClicked(id); }, this);
        m_barIcons.push_back(icon);
        barLayout->addWidget(icon, i / kCols, i % kCols, Qt::AlignCenter);
    }
    barGroup->setLayout(barLayout);
    mainLayout->addWidget(barGroup);

    // 状态标签
    m_statusLabel = new QLabel(this);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet("font-size: 13px; font-weight: bold;");
    mainLayout->addWidget(m_statusLabel);

    // 确认按钮
    m_confirmButton = new QPushButton(QStringLiteral("确认"), this);
    m_confirmButton->setEnabled(false);
    m_confirmButton->setStyleSheet(R"(
        QPushButton {
            background-color: #3a3a3a;
            color: #f2f2f2;
            border: 1px solid #565656;
            border-radius: 6px;
            padding: 10px 36px;
            font-size: 15px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #4a4a4a;
        }
        QPushButton:disabled {
            background-color: #252525;
            color: #808080;
        }
    )");
    connect(m_confirmButton, &QPushButton::clicked, this, &LootDialog::onConfirm);
    mainLayout->addWidget(m_confirmButton, 0, Qt::AlignCenter);

    updateStatusLabel();
}

void LootDialog::onIconClicked(int id)
{
    if (isNewItemId(id)) {
        // 新装备：切换选中
        int newIdx = id;
        if (m_result.discardNewIndices.count(newIdx)) {
            m_result.discardNewIndices.erase(newIdx);
        } else {
            m_result.discardNewIndices.insert(newIdx);
        }
    } else {
        // 装备栏槽位：切换选中
        int barIdx = idToBarIndex(id);
        if (m_result.discardBarIndices.count(barIdx)) {
            m_result.discardBarIndices.erase(barIdx);
        } else {
            m_result.discardBarIndices.insert(barIdx);
        }
    }

    // 更新图标选中状态
    for (auto* icon : m_newIcons) {
        icon->setSelected(m_result.discardNewIndices.count(icon->id()) > 0);
    }
    for (auto* icon : m_barIcons) {
        icon->setSelected(m_result.discardBarIndices.count(idToBarIndex(icon->id())) > 0);
    }

    updateStatusLabel();
}

void LootDialog::updateStatusLabel()
{
    // 计算总物品数：非空装备栏槽位 + 新装备
    int filledBar = 0;
    for (const auto& eq : m_barEquipments) {
        if (eq) ++filledBar;
    }
    const int totalItems = filledBar + m_newCount;

    const int discardedNew = static_cast<int>(m_result.discardNewIndices.size());
    const int discardedBar = static_cast<int>(m_result.discardBarIndices.size());
    const int remaining = totalItems - discardedNew - discardedBar;

    constexpr int kMaxSlots = 8;
    const bool ok = (remaining <= kMaxSlots);

    // 更新确认按钮
    m_confirmButton->setEnabled(ok);
    if (ok) {
        m_confirmButton->setStyleSheet(R"(
            QPushButton {
                background-color: #4a6a3a;
                color: #ffffff;
                border: 1px solid #6aaa4a;
                border-radius: 6px;
                padding: 10px 36px;
                font-size: 15px;
                font-weight: bold;
            }
            QPushButton:hover {
                background-color: #5a7a4a;
            }
        )");
    } else {
        m_confirmButton->setStyleSheet(R"(
            QPushButton {
                background-color: #3a3a3a;
                color: #808080;
                border: 1px solid #565656;
                border-radius: 6px;
                padding: 10px 36px;
                font-size: 15px;
                font-weight: bold;
            }
        )");
    }

    // 更新状态文字
    if (remaining <= kMaxSlots) {
        m_statusLabel->setStyleSheet("font-size: 13px; font-weight: bold; color: #66cc66;");
        m_statusLabel->setText(QString("\xE2\x9C\x93 已选丢弃: %1 件 | 剩余: %2 件 (≤%3)")
            .arg(discardedNew + discardedBar).arg(remaining).arg(kMaxSlots));
    } else {
        m_statusLabel->setStyleSheet("font-size: 13px; font-weight: bold; color: #ff6666;");
        m_statusLabel->setText(QString("\xE2\x9C\x97 已选丢弃: %1 件 | 剩余: %2 件 (需 ≤%3)")
            .arg(discardedNew + discardedBar).arg(remaining).arg(kMaxSlots));
    }
}

void LootDialog::onConfirm()
{
    m_result.cancelled = false;
    accept();
}

void LootDialog::reject()
{
    m_result.cancelled = true;
    m_result.discardBarIndices.clear();
    m_result.discardNewIndices.clear();
    QDialog::reject();
}
