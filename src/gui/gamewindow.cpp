#include "gamewindow.h"
#include "core/game.h"
#include "gui/settlementdialog.h"
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QPainter>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QFrame>

GameWindow::GameWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_centralWidget(new QWidget(this))
    , m_mainLayout(new QHBoxLayout())
    , m_view(new QGraphicsView(this))
    , m_game(new Game(this))
    , m_resetButton(new QPushButton("Reset", this))
    , m_battleButton(new QPushButton("开始战斗", this))
    , m_buyXpButton(new QPushButton("购买2经验", this))
{
    setupUI();
    m_game->initialize();
}

GameWindow::~GameWindow() = default;

//  按钮回调
void GameWindow::onResetButtonClicked() // 重置按钮点击事件处理函数，调用游戏的reset方法重置游戏状态
{
    if (m_game) {
        m_game->reset();
    }
}

void GameWindow::onBattleButtonClicked() {
    if (m_game) {
        m_game->startBattle();
    }
}

void GameWindow::onBuyXpButtonClicked() {
    m_game->buyXp(2); // 购买2点经验
}


//  结算
void GameWindow::onSettlementReady(const SettlementInfo& info) {
    SettlementDialog dialog(info, this);
    dialog.exec(); // 显示结算对话框，用户点击后继续下一回合或重置游戏

    if (info.isGameOver) {
        m_game->reset();
    } else {
        m_game->startPreparation();
    }
}


//  侧边栏刷新
QString GameWindow::hpText(int hp, int maxHp) const
{
    double ratio = maxHp > 0 ? static_cast<double>(hp) / maxHp : 0.0;
    QString color;
    if (ratio > 0.6)       color = "#66cc66";  // 绿
    else if (ratio > 0.3)  color = "#ffaa33";  // 橙
    else                   color = "#ff4444";  // 红

    return QString("<span style='color:%1;'>HP: %2 / %3</span>")
        .arg(color).arg(hp).arg(maxHp);
}

QString GameWindow::traitHtml(const QHash<QString, int>& traits) const
{
    if (traits.isEmpty()) {
        return QStringLiteral("羁绊: 无");
    }

    QStringList parts;
    for (auto it = traits.begin(); it != traits.end(); ++it) {
        QColor color = traitColor(it.key());
        QString hex = QString("#%1%2%3")
            .arg(color.red(), 2, 16, QChar('0'))
            .arg(color.green(), 2, 16, QChar('0'))
            .arg(color.blue(), 2, 16, QChar('0'));
        parts << QString("<span style='color:%1;'>%2×%3</span>")
            .arg(hex).arg(it.key()).arg(it.value());
    }
    return QStringLiteral("羁绊: ") + parts.join(" · ");
}

void GameWindow::refreshInfoBar()
{
    Player* p = m_game->player();
    Player* e = m_game->enemy();

    // ---- 敌方 ----
    m_enemyLevelLabel->setText(QString("等级: %1").arg(e->level()));
    m_enemyHpLabel->setText(hpText(e->hp(), e->maxHp()));
    m_enemyGoldLabel->setText(QString("金币: <span style='color:#ffcc00;'>%1</span>").arg(e->gold()));
    m_enemyFieldLabel->setText(QString("场上: %1/%2")
        .arg(m_game->countFieldUnits(EnemyCtrl)).arg(e->maxFieldUnits()));
    m_enemyTraitLabel->setText(traitHtml(m_game->getTraitCounts(EnemyCtrl)));

    // ---- 玩家 ----
    m_playerLevelLabel->setText(QString("等级: %1").arg(p->level()));
    m_playerXpLabel->setText(QString("经验: <span style='color:#66cccc;'>%1/%2</span>")
        .arg(p->xp()).arg(p->xpToNext()));
    m_playerHpLabel->setText(hpText(p->hp(), p->maxHp()));
    m_playerGoldLabel->setText(QString("金币: <span style='color:#ffcc00;'>%1</span>").arg(p->gold()));
    m_playerFieldLabel->setText(QString("场上: %1/%2")
        .arg(m_game->countFieldUnits(PlayerCtrl)).arg(p->maxFieldUnits()));
    m_playerTraitLabel->setText(traitHtml(m_game->getTraitCounts(PlayerCtrl)));

    // ---- 轮次 ----
    m_roundLabel->setText(QString("第 %1 轮").arg(m_game->battleIndex()));

    // ---- 按钮 ----
    m_buyXpButton->setText(QString("购买2经验 (2金币)"));
    m_buyXpButton->setEnabled(m_game->phase() == GamePhase::Preparation && p->gold() >= 2);
}

//  按钮禁用/启用的阶段管理
void GameWindow::onPhaseChanged(GamePhase phase) {
    switch (phase) {
    case GamePhase::Preparation:
        m_battleButton->setText("开始战斗");
        m_battleButton->setEnabled(true);
        m_resetButton->setEnabled(true);
        if (m_game->player()->xp() < m_game->player()->maxXp() && m_game->player()->gold() >= 2) {
            m_buyXpButton->setEnabled(true);
        } else {
            m_buyXpButton->setEnabled(false);
        }
        break;
    case GamePhase::Battle:
        m_battleButton->setText("战斗中...");
        m_battleButton->setEnabled(false);
        m_resetButton->setEnabled(false);
        m_buyXpButton->setEnabled(false);
        break;
    case GamePhase::Settlement:
        m_battleButton->setText("结算中...");
        m_battleButton->setEnabled(false);
        m_resetButton->setEnabled(false);
        m_buyXpButton->setEnabled(false);
        break;
    }
}

//  布局搭建
QGroupBox* GameWindow::createInfoGroup(const QString& title)
{
    QGroupBox* group = new QGroupBox(title, this);
    group->setAlignment(Qt::AlignCenter);
    group->setStyleSheet(R"(
        QGroupBox {
            background-color: #333333;
            border: 1px solid #565656;
            border-radius: 6px;
            margin-top: 10px;
            padding: 20px 12px 10px 12px;
            color: #f2f2f2;
            font-size: 13px;
            font-weight: bold;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 12px;
            padding: 0 8px;
            color: #e0e0e0;
        }
    )");
    return group;
}

void GameWindow::setupUI()
{
    setCentralWidget(m_centralWidget);
    m_centralWidget->setLayout(m_mainLayout);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    setStyleSheet(R"(
        QMainWindow {
            background-color: #2b2b2b;
        }
        QWidget {
            background-color: #2b2b2b;
            color: #f2f2f2;
        }
        QPushButton {
            background-color: #2f2f2f;
            color: #f2f2f2;
            border: 1px solid #565656;
            border-radius: 4px;
            padding: 6px 14px;
            font-size: 13px;
        }
        QPushButton:hover {
            background-color: #3a3a3a;
        }
        QPushButton:pressed {
            background-color: #242424;
        }
        QPushButton:disabled {
            background-color: #252525;
            color: #808080;
        }
        QPushButton:disabled:hover {
            background-color: #252525;
        }
        QPushButton:disabled:pressed {
            background-color: #252525;
        }
        QLabel {
            background: transparent;
            color: #d0d0d0;
            font-size: 12px;
        }
    )");

    // ---- 左侧：游戏棋盘 ----
    m_view->setRenderHint(QPainter::Antialiasing, true); // 开启抗锯齿
    m_view->setDragMode(QGraphicsView::NoDrag); // 禁止拖动画布
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // 禁止水平滚动条
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // 禁止垂直滚动条
    m_view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse); // 设置变换锚点为鼠标位置，缩放时以鼠标位置为中心
    m_view->setResizeAnchor(QGraphicsView::AnchorViewCenter); // 设置调整大小锚点为视图中心，调整窗口大小时以视图中心为锚点
    m_view->setMouseTracking(true); // 启用鼠标跟踪，即使没有按下鼠标按钮也能接收鼠标移动事件
    m_view->viewport()->setMouseTracking(true); // 启用视口的鼠标跟踪，确保在视口内移动鼠标时也能接收事件
    m_view->setMinimumWidth(620);
    m_mainLayout->addWidget(m_view, 1); // 将游戏视图添加到主布局中，并设置伸缩因子为1

    // ---- 右侧：信息面板 ----
    m_sidePanel = new QWidget(this);
    m_sidePanel->setObjectName("sidePanel");
    m_sidePanel->setFixedWidth(220);
    m_sidePanel->setStyleSheet("#sidePanel { background-color: #2a2a2a; }");

    auto* sideLayout = new QVBoxLayout(m_sidePanel);
    sideLayout->setContentsMargins(8, 8, 8, 8);
    sideLayout->setSpacing(6);

    // ----- 敌方信息组 -----
    m_enemyGroup = createInfoGroup(QStringLiteral("⚔ 敌方"));
    auto* enemyLayout = new QVBoxLayout(m_enemyGroup);
    enemyLayout->setSpacing(3);

    m_enemyLevelLabel = new QLabel("等级: -", this);
    m_enemyHpLabel     = new QLabel("HP: -/-", this);
    m_enemyGoldLabel   = new QLabel("金币: -", this);
    m_enemyFieldLabel  = new QLabel("场上: -/-", this);
    m_enemyTraitLabel  = new QLabel("羁绊: 无", this);

    for (QLabel* lbl : {m_enemyLevelLabel, m_enemyHpLabel, m_enemyGoldLabel,
                         m_enemyFieldLabel, m_enemyTraitLabel}) {
        lbl->setStyleSheet("font-size: 15px; background: transparent;");
    }

    enemyLayout->addWidget(m_enemyLevelLabel);
    enemyLayout->addWidget(m_enemyHpLabel);
    enemyLayout->addWidget(m_enemyGoldLabel);
    enemyLayout->addWidget(m_enemyFieldLabel);
    enemyLayout->addWidget(m_enemyTraitLabel);
    sideLayout->addWidget(m_enemyGroup);

    // ----- 轮次显示 -----
    QFrame* roundFrame = new QFrame(this);
    roundFrame->setStyleSheet(R"(
        QFrame {
            background-color: #3a3528;
            border: 1px solid #ffcc66;
            border-radius: 6px;
        }
    )");
    auto* roundLayout = new QVBoxLayout(roundFrame);
    roundLayout->setContentsMargins(8, 4, 8, 4);
    m_roundLabel = new QLabel("第 1 轮", this);
    m_roundLabel->setAlignment(Qt::AlignCenter);
    m_roundLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #ffcc66; background: transparent;");
    roundLayout->addWidget(m_roundLabel);
    sideLayout->addWidget(roundFrame);

    // ----- 玩家信息组 -----
    m_playerGroup = createInfoGroup(QStringLiteral("🛡 玩家"));
    auto* playerLayout = new QVBoxLayout(m_playerGroup);
    playerLayout->setSpacing(3);

    m_playerLevelLabel = new QLabel("等级: -", this);
    m_playerXpLabel    = new QLabel("经验: -/-", this);
    m_playerHpLabel    = new QLabel("HP: -/-", this);
    m_playerGoldLabel  = new QLabel("金币: -", this);
    m_playerFieldLabel = new QLabel("场上: -/-", this);
    m_playerTraitLabel = new QLabel("羁绊: 无", this);

    for (QLabel* lbl : {m_playerLevelLabel, m_playerXpLabel, m_playerHpLabel,
                         m_playerGoldLabel, m_playerFieldLabel, m_playerTraitLabel}) {
        lbl->setStyleSheet("font-size: 15px; background: transparent;");
    }

    playerLayout->addWidget(m_playerLevelLabel);
    playerLayout->addWidget(m_playerXpLabel);
    playerLayout->addWidget(m_playerHpLabel);
    playerLayout->addWidget(m_playerGoldLabel);
    playerLayout->addWidget(m_playerFieldLabel);
    playerLayout->addWidget(m_playerTraitLabel);
    sideLayout->addWidget(m_playerGroup);

    // ----- 弹性空间 -----
    sideLayout->addStretch();

    // ----- 按钮组 -----
    auto* btnLayout = new QVBoxLayout();
    btnLayout->setSpacing(4);
    btnLayout->addWidget(m_buyXpButton);
    btnLayout->addWidget(m_battleButton);
    btnLayout->addWidget(m_resetButton);
    sideLayout->addLayout(btnLayout);

    m_mainLayout->addWidget(m_sidePanel);

    // ---- 信号连接 ----
    connect(m_resetButton, &QPushButton::clicked,
            this, &GameWindow::onResetButtonClicked); // 连接重置按钮的点击信号到槽函数

    connect(m_buyXpButton, &QPushButton::clicked,
            this, &GameWindow::onBuyXpButtonClicked); // 连接购买经验按钮的点击信号到槽函数

    connect(m_battleButton, &QPushButton::clicked,
            this, &GameWindow::onBattleButtonClicked); // 连接"开始战斗"按钮的点击信号到槽函数

    connect(m_game, &Game::phaseChanged,
            this, &GameWindow::onPhaseChanged); // 连接游戏阶段变化信号到槽函数

    connect(m_game, &Game::stateUpdated,
            this, &GameWindow::refreshInfoBar); // 连接游戏状态更新信号到刷新信息栏的槽函数

    connect(m_game, &Game::settlementReady,
            this, &GameWindow::onSettlementReady); // 连接结算信号到显示结算对话框的槽函数

    m_view->setScene(m_game->scene()); // 将游戏的图形场景设置为视图的场景，使游戏内容能够显示在视图中
}
