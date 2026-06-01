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
    , m_game(new Game(this))
{
    setupUI();
    m_game->initialize();
}

GameWindow::~GameWindow() = default;

void GameWindow::onResetButtonClicked()
{
    if (m_game) {
        m_game->reset();
    }
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

    QWidget* controlBar = new QWidget(this); // 控制按钮栏
    QHBoxLayout* controlLayout = new QHBoxLayout(controlBar); // 水平布局，用于放置控制按钮
    controlLayout->setContentsMargins(0, 0, 0, 0); // 去除布局边距
    controlLayout->addWidget(m_resetButton); // 将重置按钮添加到控制栏
    controlLayout->addStretch(); // 添加一个伸缩项，使按钮靠左显示
    m_mainLayout->addWidget(controlBar); // 将控制栏添加到主布局中

    connect(m_resetButton, &QPushButton::clicked,
            this, &GameWindow::onResetButtonClicked); // 连接重置按钮的点击信号到槽函数

    m_view->setScene(m_game->scene()); // 将游戏的图形场景设置为视图的场景，使游戏内容能够显示在视图中
}
