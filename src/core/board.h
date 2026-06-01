#ifndef BOARD_H
#define BOARD_H

#include <QHash>
#include <QPoint>
#include <QVector>
#include "entity/unit.h"

class BoardANDBench // 包含棋盘和备战区的类
{
public:
    static constexpr int BOARD_ROWS = 8;
    static constexpr int BOARD_COLS = 8;
    static constexpr int BENCH_ROW = 1;
    static constexpr int BENCH_COL = 10;

    BoardANDBench();
    ~BoardANDBench() = default;

    void addUnit(Unit* unit, const QPoint& pos);
    void removeUnit(Unit* unit);
    Unit* getUnitAt(const QPoint& pos) const;
    bool hasUnitAt(const QPoint& pos) const;

    bool isBoardPosition(const QPoint& pos) const; // 检查位置是否在棋盘范围内
    bool isBenchPosition(const QPoint& pos) const; // 检查位置是否在备战区范围内

    bool isValidPosition(const QPoint& pos) const;
    bool isPlayerHalf(const QPoint& pos) const;

    void clear();

private:
    int indexOf(const QPoint& pos) const;

    QVector<Unit*> m_cells; // 格子 -> 单位的映射，nullptr表示无单位
    QHash<Unit*, QPoint> m_unitToPosition; // 单位 -> 格子位置的映射，方便快速查找单位位置
};

#endif // BOARD_H
