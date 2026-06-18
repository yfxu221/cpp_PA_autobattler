#include "gui/settlementdialog.h"
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QGridLayout>
#include <QString>

SettlementDialog::SettlementDialog(const SettlementInfo& info, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("战斗结算"));
    setFixedSize(360, 440);
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
            padding-top: 16px;
            color: #f2f2f2;
            font-size: 13px;
            font-weight: bold;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 12px;
            padding: 0 6px;
        }
        QLabel {
            color: #e0e0e0;
            font-size: 13px;
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
    )");

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(20, 16, 20, 16);

    // ---- 标题 ----
    auto* titleLabel = new QLabel(info.isGameOver
        ? QStringLiteral("💀 游戏结束")
        : QStringLiteral("⚔ 战斗结算"), this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #ffcc66;");
    mainLayout->addWidget(titleLabel);

    // ---- 结果 ----
    auto* resultLabel = new QLabel(resultText(info.result), this);
    resultLabel->setAlignment(Qt::AlignCenter);
    resultLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #ffffff;");
    mainLayout->addWidget(resultLabel);

    // ---- 玩家信息 ----
    auto* playerGroup = new QGroupBox(QStringLiteral("🛡 玩家"), this);
    auto* playerGrid = new QGridLayout(playerGroup);
    playerGrid->setSpacing(4);
    playerGrid->addWidget(new QLabel(hpChangeLabel("HP", info.playerHpBefore, info.playerHpAfter), this), 0, 0);
    playerGrid->addWidget(new QLabel(goldChangeLabel("金币", info.playerGoldBefore, info.playerGoldAfter,
                                                     info.playerInterest, info.playerStreakBonus), this), 1, 0);
    playerGrid->addWidget(new QLabel(streakStatusLabel(info.playerWinStreak, info.playerLoseStreak), this), 2, 0);
    mainLayout->addWidget(playerGroup);

    // ---- 敌方信息 ----
    auto* enemyGroup = new QGroupBox(QStringLiteral("⚔ 敌方"), this);
    auto* enemyGrid = new QGridLayout(enemyGroup);
    enemyGrid->setSpacing(4);
    enemyGrid->addWidget(new QLabel(hpChangeLabel("HP", info.enemyHpBefore, info.enemyHpAfter), this), 0, 0);
    enemyGrid->addWidget(new QLabel(goldChangeLabel("金币", info.enemyGoldBefore, info.enemyGoldAfter,
                                                    info.enemyInterest, info.enemyStreakBonus), this), 1, 0);
    enemyGrid->addWidget(new QLabel(streakStatusLabel(info.enemyWinStreak, info.enemyLoseStreak), this), 2, 0);
    mainLayout->addWidget(enemyGroup);

    // ---- 按钮 ----
    auto* button = new QPushButton(info.isGameOver
        ? QStringLiteral("重新开始")
        : QStringLiteral("确定 → 下一回合"), this);
    button->setDefault(true);
    connect(button, &QPushButton::clicked, this, &QDialog::accept); // 点击按钮后关闭对话框
    mainLayout->addWidget(button, 0, Qt::AlignCenter);
}

QString SettlementDialog::resultText(BattleResult result) const
{
    switch (result) {
    case BattleResult::PlayerWin:
        return QStringLiteral("🏆 玩家胜利！");
    case BattleResult::EnemyWin:
        return QStringLiteral("💀 敌方胜利！");
    case BattleResult::Draw:
        return QStringLiteral("🤝 平局！");
    default:
        return {};
    }
}

QString SettlementDialog::hpChangeLabel(const QString& side, int before, int after) const
{
    int delta = after - before;
    QString deltaStr = delta >= 0
        ? QString("(+%1)").arg(delta)
        : QString("(%1)").arg(delta);
    QString color = delta >= 0 ? "#66cc66" : "#ff6666";
    return QString("<span style='color:%3'>%1:  %2  →  %4  %5</span>")
        .arg(side)
        .arg(before)
        .arg(color)
        .arg(after)
        .arg(deltaStr);
}

QString SettlementDialog::goldChangeLabel(const QString& side, int before, int after,
                                         int interest, int streakBonus) const
{
    int delta = after - before;
    QString deltaStr = delta >= 0
        ? QString("(+%1)").arg(delta)
        : QString("(%1)").arg(delta);
    QString color = delta >= 0 ? "#ffcc66" : "#ff6666";

    // 构建金币变动明细
    QStringList parts;
    if (interest > 0) {
        parts.append(QString("利息 +%1").arg(interest));
    }
    if (streakBonus > 0) {
        parts.append(QString("连胜/连败奖励 +%1").arg(streakBonus));
    }
    QString breakdown;
    if (!parts.isEmpty()) {
        breakdown = QString("<br><span style='font-size:11px; color:#aaaaaa;'>  (%2)</span>")
                        .arg(parts.join("，"));
    }

    return QString("<span style='color:%3'>%1:  %2  →  %4  %5</span>%6")
        .arg(side)
        .arg(before)
        .arg(color)
        .arg(after)
        .arg(deltaStr)
        .arg(breakdown);
}

QString SettlementDialog::streakStatusLabel(int winStreak, int loseStreak) const
{
    if (winStreak >= 2) {
        return QString("<span style='font-size:12px; color:#ff9944;'>🔥 连胜 x%1</span>").arg(winStreak);
    }
    if (loseStreak >= 2) {
        return QString("<span style='font-size:12px; color:#6699cc;'>❄ 连败 x%1</span>").arg(loseStreak);
    }
    return QStringLiteral(""); // 无连胜/连败时不显示
}
