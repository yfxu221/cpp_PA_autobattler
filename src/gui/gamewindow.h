#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include "core/game.h"

class QGraphicsView;
class QPushButton;
class QHBoxLayout;
class QGroupBox;

class GameWindow : public QMainWindow // 主窗口类，包含游戏视图和右侧信息面板
{
    Q_OBJECT

public:
    explicit GameWindow(QWidget* parent = nullptr);
    ~GameWindow();

private slots:
    void onResetButtonClicked();
    void onBattleButtonClicked();
    void onPhaseChanged(GamePhase phase);  // 根据阶段更新按钮状态
    void refreshInfoBar(); // 刷新侧边栏显示双方状态等信息
    void onBuyXpButtonClicked(); // 购买经验按钮点击事件处理函数
    void onSettlementReady(const SettlementInfo& info); // 结算界面显示
    void onEquipBarOverflow(); // 装备栏溢出弹窗

private:
    void setupUI();
    QGroupBox* createInfoGroup(const QString& title); // 创建带标题的分组框
    QString hpText(int hp, int maxHp) const; // 生成带颜色的HP文本（绿/橙/红）
    QString traitHtml(const QHash<QString, int>& traits) const; // 生成羁绊彩色HTML

    QWidget* m_centralWidget; // 中央部件
    QHBoxLayout* m_mainLayout; // 主布局，水平排列：左视图 + 右侧面板
    QGraphicsView* m_view; // 游戏视图
    Game* m_game;

    // 右侧面板
    QWidget* m_sidePanel; // 右侧面板容器

    // 敌方信息
    QGroupBox* m_enemyGroup;
    QLabel* m_enemyLevelLabel; // 敌方等级标签
    QLabel* m_enemyHpLabel; // 敌方HP标签
    QLabel* m_enemyGoldLabel; // 敌方金币标签
    QLabel* m_enemyFieldLabel; // 敌方场上单位标签
    QLabel* m_enemyTraitLabel; // 敌方羁绊标签

    // 战斗轮次
    QLabel* m_roundLabel; // 当前轮次显示

    // 玩家信息
    QGroupBox* m_playerGroup;
    QLabel* m_playerLevelLabel; // 玩家等级标签
    QLabel* m_playerXpLabel; // 玩家经验值标签
    QLabel* m_playerHpLabel; // 玩家HP标签
    QLabel* m_playerGoldLabel; // 玩家金币标签
    QLabel* m_playerFieldLabel; // 玩家场上单位标签
    QLabel* m_playerTraitLabel; // 玩家羁绊标签

    // 按钮
    QPushButton* m_resetButton; // 重置按钮
    QPushButton* m_battleButton;   // "开始战斗"按钮
    QPushButton* m_buyXpButton; // 购买经验按钮
};

#endif // GAMEWINDOW_H
