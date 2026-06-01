#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QMainWindow>

class Game;
class QGraphicsView;
class QPushButton;
class QVBoxLayout;

class GameWindow : public QMainWindow // 主窗口类，包含游戏视图和控制按钮
{
    Q_OBJECT

public:
    explicit GameWindow(QWidget* parent = nullptr);
    ~GameWindow();

private slots:
    void onResetButtonClicked();

private:
    void setupUI();

    QWidget* m_centralWidget; // 中央部件，包含游戏视图和按钮
    QVBoxLayout* m_mainLayout; // 主布局，垂直排列游戏视图和按钮
    QGraphicsView* m_view; // 游戏视图
    QPushButton* m_resetButton; // 重置按钮
    Game* m_game;
};

#endif // GAMEWINDOW_H
