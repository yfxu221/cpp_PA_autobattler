#include "gamewindow.h"
#include "core/game.h"
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QPainter>
#include <QPushButton>
#include <QVBoxLayout>

GameWindow::GameWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_centralWidget(new QWidget(this))
    , m_mainLayout(new QVBoxLayout())
    , m_view(new QGraphicsView(this))
    , m_resetButton(new QPushButton("Reset", this))
    , m_battleButton(new QPushButton("开始战斗", this))
    , m_game(new Game(this))
    , m_buyXpButton(new QPushButton("购买2经验", this))
{
    setupUI();
    m_game->initialize();
}

GameWindow::~GameWindow() = default;

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

void GameWindow::refreshInfoBar() {
    Player* p = m_game->player();
    m_levelLabel->setText(QString("等级: %1").arg(p->level()));
    m_xpLabel->setText(QString("经验: %1/%2").arg(p->xp()).arg(p->xpToNext()));
    m_goldLabel->setText(QString("金币: %1").arg(p->gold()));
    m_hpLabel->setText(QString("生命值: %1/%2").arg(p->hp()).arg(p->maxHp()));
    m_fieldLabel->setText(QString("场上: %1/%2").arg(m_game->countFieldUnits(PlayerCtrl)).arg(p->maxFieldUnits()));
    m_buyXpButton->setText(QString("购买2经验 (2金币)"));
    m_buyXpButton->setEnabled(p->gold() >= 2);

    QStringList traitParts;
    const auto traits = m_game->getTraitCounts(PlayerCtrl);
    for (auto it = traits.begin(); it != traits.end(); ++it) {
        QColor color = traitColor(it.key());
        QString hex = QString("#%1%2%3")
        .arg(color.red(), 2, 16, QChar('0'))
        .arg(color.green(), 2, 16, QChar('0'))
        .arg(color.blue(), 2, 16, QChar('0'));
    traitParts << QString("<span style='color:%1'>%2×%3</span>")
        .arg(hex).arg(it.key()).arg(it.value());
    }
    m_traitLabel->setText(traitParts.isEmpty() ? "羁绊: 无" : "羁绊: " + traitParts.join(" · "));
}

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
        m_resetButton->setEnabled(true);
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

void GameWindow::onBuyXpButtonClicked() {
    m_game->buyXp(2); // 购买2点经验
}


void GameWindow::setupUI()
{
    setCentralWidget(m_centralWidget);
    m_centralWidget->setLayout(m_mainLayout);

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
    )");

    m_view->setRenderHint(QPainter::Antialiasing, true); // 开启抗锯齿
    m_view->setDragMode(QGraphicsView::NoDrag); // 禁止拖动画布
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // 禁止水平滚动条
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // 禁止垂直滚动条
    m_view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse); // 设置变换锚点为鼠标位置，缩放时以鼠标位置为中心
    m_view->setResizeAnchor(QGraphicsView::AnchorViewCenter); // 设置调整大小锚点为视图中心，调整窗口大小时以视图中心为锚点
    m_view->setMouseTracking(true); // 启用鼠标跟踪，即使没有按下鼠标按钮也能接收鼠标移动事件
    m_view->viewport()->setMouseTracking(true); // 启用视口的鼠标跟踪，确保在视口内移动鼠标时也能接收事件

    m_mainLayout->addWidget(m_view, 1); // 将游戏视图添加到主布局中，并设置伸缩因子为1，使其占满剩余空间

    // InfoBar部分
    m_infoBar = new QWidget(this);
    m_infoLayout = new QHBoxLayout(m_infoBar);
    m_infoLayout->setContentsMargins(8, 4, 8, 4);
    m_levelLabel = new QLabel("等级: -", m_infoBar);
    m_xpLabel = new QLabel("经验: -/-", m_infoBar);
    m_goldLabel = new QLabel("金币: -", m_infoBar);
    m_hpLabel = new QLabel("生命值: -/-", m_infoBar);
    m_fieldLabel = new QLabel("场上: -/-", m_infoBar);
    m_traitLabel = new QLabel("羁绊: 无", m_infoBar);
    m_infoLayout->addWidget(m_levelLabel);
    m_infoLayout->addWidget(new QLabel(" | ", m_infoBar));
    m_infoLayout->addWidget(m_xpLabel);
    m_infoLayout->addWidget(new QLabel(" | ", m_infoBar));
    m_infoLayout->addWidget(m_goldLabel);
    m_infoLayout->addWidget(new QLabel(" | ", m_infoBar));
    m_infoLayout->addWidget(m_hpLabel);
    m_infoLayout->addWidget(new QLabel(" | ", m_infoBar));
    m_infoLayout->addWidget(m_fieldLabel);
    m_infoLayout->addWidget(new QLabel(" | ", m_infoBar));
    m_infoLayout->addWidget(m_traitLabel);
    m_infoLayout->addStretch();
    m_mainLayout->addWidget(m_infoBar);


    QWidget* controlBar = new QWidget(this); // 控制按钮栏
    QHBoxLayout* controlLayout = new QHBoxLayout(controlBar); // 水平布局，用于放置控制按钮
    controlLayout->setContentsMargins(0, 0, 0, 0); // 去除布局边距
    controlLayout->addWidget(m_resetButton); // 将重置按钮添加到控制栏
    controlLayout->addWidget(m_battleButton); // 将"开始战斗"按钮添加到控制栏
    controlLayout->addWidget(m_buyXpButton); // 将购买经验按钮添加到控制栏
    controlLayout->addStretch(); // 添加一个伸缩项，使按钮靠左显示
    m_mainLayout->addWidget(controlBar); // 将控制栏添加到主布局中

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

    m_view->setScene(m_game->scene()); // 将游戏的图形场景设置为视图的场景，使游戏内容能够显示在视图中
}
