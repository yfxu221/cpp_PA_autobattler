#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include "core/game.h"

class QGraphicsView;
class QPushButton;
class QVBoxLayout;
class QHBoxLayout;

class GameWindow : public QMainWindow // 主窗口类，包含游戏视图和控制按钮
{
    Q_OBJECT

public:
    explicit GameWindow(QWidget* parent = nullptr);
    ~GameWindow();

private slots:
    void onResetButtonClicked();
    void onBattleButtonClicked();
    void onPhaseChanged(GamePhase phase);  // 根据阶段更新按钮状态
    void refreshInfoBar(); // 刷新信息栏显示玩家状态等信息
    void onBuyXpButtonClicked(); // 购买经验按钮点击事件处理函数

private:
    void setupUI();

    QWidget* m_centralWidget; // 中央部件，包含游戏视图和按钮
    QVBoxLayout* m_mainLayout; // 主布局，垂直排列游戏视图和按钮
    QGraphicsView* m_view; // 游戏视图
    QPushButton* m_resetButton; // 重置按钮
    Game* m_game;

    QPushButton* m_battleButton;   // "开始战斗"按钮

    QWidget* m_infoBar; // 玩家状态栏
    QHBoxLayout* m_infoLayout; // 玩家状态栏布局，水平排列各种状态标签
    QLabel* m_levelLabel; // 等级标签
    QLabel* m_xpLabel; // 经验值标签
    QLabel* m_goldLabel; // 金币标签
    QLabel* m_hpLabel; // 生命值标签
    QLabel* m_fieldLabel; // 棋盘单位数量标签
    QLabel* m_traitLabel; // 羁绊标签

    QPushButton* m_buyXpButton; // 购买经验按钮
};

#endif // GAMEWINDOW_H
