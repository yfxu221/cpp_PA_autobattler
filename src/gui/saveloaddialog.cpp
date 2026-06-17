#include "saveloaddialog.h"
#include <QMouseEvent>
#include <QEnterEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QApplication>
#include <QMessageBox>


SaveSlotCard::SaveSlotCard(int slot, QWidget* parent)
    : QFrame(parent)
    , m_slot(slot)
{
    setFixedSize(200, 120);
    setCursor(Qt::PointingHandCursor);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 8, 12, 8);
    layout->setSpacing(3);

    // 顶部行：槽位编号 + 状态文本
    auto* topRow = new QHBoxLayout();
    m_slotNumberLabel = new QLabel(QString("[%1]").arg(slot + 1), this);
    m_slotNumberLabel->setStyleSheet("font-size:13px; font-weight:bold; color:#cccccc; background:transparent;");
    topRow->addWidget(m_slotNumberLabel);
    topRow->addStretch();

    m_statusLabel = new QLabel(this);
    m_statusLabel->setStyleSheet("font-size:10px; color:#888888; background:transparent;");
    topRow->addWidget(m_statusLabel);
    layout->addLayout(topRow);

    // 轮次
    m_roundLabel = new QLabel(this);
    m_roundLabel->setStyleSheet("font-size:12px; color:#cccccc; background:transparent;");
    layout->addWidget(m_roundLabel);

    // 摘要：等级 + HP + 金币
    m_summaryLabel = new QLabel(this);
    m_summaryLabel->setStyleSheet("font-size:12px; color:#cccccc; background:transparent;");
    layout->addWidget(m_summaryLabel);

    // 备注预览
    m_labelPreview = new QLabel(this);
    m_labelPreview->setStyleSheet("font-size:11px; color:#ffcc66; background:transparent;");
    layout->addWidget(m_labelPreview);

    layout->addStretch();

    applyStyleSheet();
}

void SaveSlotCard::setData(const SaveMeta& meta)
{
    m_meta = meta;

    if (meta.isEmpty) {
        m_statusLabel->setText("— 空 —");
        m_roundLabel->hide();
        m_summaryLabel->hide();
        m_labelPreview->hide();
    } else {
        m_statusLabel->setText(meta.timestamp);
        m_roundLabel->setText(QString("第 %1 轮  Lv.%2")
            .arg(meta.battleIndex).arg(meta.playerLevel));
        m_roundLabel->show();

        m_summaryLabel->setText(QString("HP:<span style='color:#66cc66;'>%1</span>  金币:<span style='color:#ffcc00;'>%2</span>")
            .arg(meta.playerHp).arg(meta.playerGold));
        m_summaryLabel->show();

        if (meta.label.isEmpty()) {
            m_labelPreview->hide();
        } else {
            m_labelPreview->setText(QString("「%1」").arg(meta.label));
            m_labelPreview->show();
        }
    }

    applyStyleSheet();
}

void SaveSlotCard::setSelected(bool selected)
{
    m_selected = selected;
    applyStyleSheet();
}

void SaveSlotCard::applyStyleSheet()
{
    QString borderColor;
    QString bgColor;

    if (m_selected) {
        borderColor = "#5599dd";
        bgColor = "#2a3a4a";
    } else if (m_hovered) {
        borderColor = "#666688";
        bgColor = "#2a2a35";
    } else {
        borderColor = "#444455";
        bgColor = "#252530";
    }

    if (m_meta.isEmpty) {
        bgColor = "#222228";
    }

    setStyleSheet(QString(
        "SaveSlotCard {"
        "  background-color: %1;"
        "  border: 2px solid %2;"
        "  border-radius: 8px;"
        "}"
    ).arg(bgColor, borderColor));
}

void SaveSlotCard::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        emit clicked(m_slot);
    }
    QFrame::mousePressEvent(event);
}

void SaveSlotCard::enterEvent(QEnterEvent* event)
{
    m_hovered = true;
    applyStyleSheet();
    QFrame::enterEvent(event);
}

void SaveSlotCard::leaveEvent(QEvent* event)
{
    m_hovered = false;
    applyStyleSheet();
    QFrame::leaveEvent(event);
}


SaveLoadDialog::SaveLoadDialog(Mode mode, QWidget* parent)
    : QDialog(parent)
    , m_mode(mode)
{
    setupUI();
    refreshSlots();
}

void SaveLoadDialog::setupUI()
{
    const QString titleText = (m_mode == Mode::Save)
        ? QStringLiteral("💾 保存游戏")
        : QStringLiteral("📂 读取存档");

    setWindowTitle(titleText);
    setFixedSize(920, 520);
    setModal(true);

    // 禁用默认标题栏的问号按钮
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // 全局暗色风格
    setStyleSheet(R"(
        QDialog {
            background-color: #1e1e28;
        }
        QLabel {
            color: #d0d0d0;
            background: transparent;
        }
        QPushButton {
            background-color: #2f2f3a;
            color: #f2f2f2;
            border: 1px solid #555566;
            border-radius: 4px;
            padding: 8px 24px;
            font-size: 14px;
            min-width: 100px;
        }
        QPushButton:hover {
            background-color: #3a3a48;
        }
        QPushButton:pressed {
            background-color: #242430;
        }
        QPushButton:disabled {
            background-color: #252530;
            color: #666666;
            border-color: #3a3a40;
        }
        QLineEdit {
            background-color: #2a2a35;
            color: #f2f2f2;
            border: 1px solid #555566;
            border-radius: 4px;
            padding: 6px 10px;
            font-size: 13px;
        }
    )");

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(24, 16, 24, 16);
    root->setSpacing(12);

    // 标题
    auto* titleLabel = new QLabel(titleText, this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size:18px; font-weight:bold; color:#f0f0f0; padding-bottom:4px;");
    root->addWidget(titleLabel);

    // 保存模式：当前状态摘要
    if (m_mode == Mode::Save) {
        m_currentSummaryLabel = new QLabel(this);
        m_currentSummaryLabel->setAlignment(Qt::AlignCenter); // 居中显示
        m_currentSummaryLabel->setStyleSheet(
            "font-size:12px; color:#999999; background:#252530;"
            "border:1px solid #3a3a44; border-radius:4px; padding:4px 12px;"
        );
        m_currentSummaryLabel->setText("当前游戏状态将保存到选中的槽位");
        root->addWidget(m_currentSummaryLabel);
    }

    // 4×2 槽位网格
    auto* grid = new QGridLayout();
    grid->setSpacing(16);

    for (int i = 0; i < SaveManager::MAX_SLOTS; ++i) {
        auto* card = new SaveSlotCard(i, this);
        connect(card, &SaveSlotCard::clicked, this, &SaveLoadDialog::onSlotClicked);
        m_cards.push_back(card);

        int row = i / 4;
        int col = i % 4;
        grid->addWidget(card, row, col, Qt::AlignCenter);
    }

    root->addLayout(grid);

    // 备注输入（仅保存模式）
    if (m_mode == Mode::Save) {
        auto* labelRow = new QHBoxLayout();
        auto* labelLbl = new QLabel("备注：", this);
        labelLbl->setStyleSheet("font-size:13px; color:#cccccc;");
        labelRow->addWidget(labelLbl);

        m_labelInput = new QLineEdit(this);
        m_labelInput->setPlaceholderText("可选：输入存档备注...");
        m_labelInput->setMaxLength(30);
        m_labelInput->setFixedWidth(400);
        labelRow->addWidget(m_labelInput);
        labelRow->addStretch();
        root->addLayout(labelRow);
    }

    root->addStretch();

    // 底部按钮
    auto* btnRow = new QHBoxLayout();
    btnRow->addStretch();

    m_confirmButton = new QPushButton(
        (m_mode == Mode::Save) ? "保存" : "读取", this);
    m_confirmButton->setEnabled(false);
    connect(m_confirmButton, &QPushButton::clicked, this, &SaveLoadDialog::onConfirmClicked);
    btnRow->addWidget(m_confirmButton);

    m_cancelButton = new QPushButton("取消", this);
    connect(m_cancelButton, &QPushButton::clicked, this, &SaveLoadDialog::onCancelClicked);
    btnRow->addWidget(m_cancelButton);

    root->addLayout(btnRow);
}

void SaveLoadDialog::refreshSlots()
{
    auto metas = SaveManager::getAllMetas();
    for (auto* card : m_cards) {
        int s = card->slot();
        if (s >= 0 && s < static_cast<int>(metas.size())) {
            card->setData(metas[s]);
        }
    }
}

void SaveLoadDialog::onSlotClicked(int slot)
{
    const auto metas = SaveManager::getAllMetas();

    // 取消所有选中
    for (auto* card : m_cards) {
        card->setSelected(false);
    }

    // 读档模式：空槽位不可选
    if (m_mode == Mode::Load && metas[slot].isEmpty) {
        m_selectedSlot = -1;
        m_confirmButton->setEnabled(false);
        return;
    }

    // 选中当前
    m_selectedSlot = slot;
    if (slot >= 0 && slot < static_cast<int>(m_cards.size())) {
        m_cards[slot]->setSelected(true);
    }

    m_confirmButton->setEnabled(true);

    // 保存模式：点击已有存档 → 自动填入备注
    if (m_mode == Mode::Save && m_labelInput && !metas[slot].isEmpty) {
        m_labelInput->setText(metas[slot].label);
    }
}

void SaveLoadDialog::onConfirmClicked()
{
    if (m_selectedSlot < 0) return;

    // 保存模式：覆盖已有存档需确认
    if (m_mode == Mode::Save) {
        auto metas = SaveManager::getAllMetas();
        if (!metas[m_selectedSlot].isEmpty) {
            const auto& meta = metas[m_selectedSlot];
            QString msg = QString(
                "槽位 %1 已有存档：\n\n"
                "  时间：%2\n"
                "  第 %3 轮  Lv.%4  HP:%5  金币:%6\n\n"
                "是否覆盖？")
                .arg(m_selectedSlot + 1)
                .arg(meta.timestamp)
                .arg(meta.battleIndex)
                .arg(meta.playerLevel)
                .arg(meta.playerHp)
                .arg(meta.playerGold);

            if (!meta.label.isEmpty()) {
                msg += QString("\n备注：「%1」").arg(meta.label);
            }

            auto btn = QMessageBox::warning(
                this,
                "覆盖确认",
                msg,
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No);

            if (btn != QMessageBox::Yes) {
                return; // 用户取消覆盖
            }
        }
    }

    accept();
}

void SaveLoadDialog::onCancelClicked()
{
    m_selectedSlot = -1;
    reject();
}
