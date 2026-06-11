#include "board.h"

BoardANDBench::BoardANDBench()
    : m_cells(BOARD_ROWS * BOARD_COLS + BENCH_ROW * BENCH_COL, nullptr)
{}

void BoardANDBench::addUnit(Unit* unit, const QPoint& pos)
{
    const int idx = indexOf(pos);
    if (!unit || idx < 0 || m_cells[idx]) {
        return;
    }

    m_cells[idx] = unit;
    m_unitToPosition[unit] = pos;
    unit->setPosition(pos);
}

void BoardANDBench::removeUnit(Unit* unit)
{
    if (!unit || !m_unitToPosition.contains(unit)) {
        return;
    }

    const int idx = indexOf(m_unitToPosition.value(unit));
    if (idx >= 0) {
        m_cells[idx] = nullptr;
    }
    m_unitToPosition.remove(unit);
}

void BoardANDBench::moveUnit(Unit* unit, const QPoint& newPos)
{
    if (!unit || !m_unitToPosition.contains(unit)) {
        return;
    }

    const int newIdx = indexOf(newPos);
    if (newIdx < 0 || m_cells[newIdx]) {
        return;  // 目标位置无效或已被占据
    }

    const QPoint oldPos = m_unitToPosition.value(unit);
    const int oldIdx = indexOf(oldPos);

    // 清除旧位置
    if (oldIdx >= 0) {
        m_cells[oldIdx] = nullptr;
    }

    // 占据新位置
    m_cells[newIdx] = unit;
    m_unitToPosition[unit] = newPos;
    unit->setPosition(newPos);
}

Unit* BoardANDBench::getUnitAt(const QPoint& pos) const // 获取指定位置的单位
{
    const int idx = indexOf(pos);
    return idx < 0 ? nullptr : m_cells[idx];
}

bool BoardANDBench::hasUnitAt(const QPoint& pos) const // 检查指定位置是否有单位
{
    return getUnitAt(pos) != nullptr;
}

bool BoardANDBench::isBoardPosition(const QPoint& pos) const // 检查位置是否在棋盘范围内
{
    
    return pos.x() >= 0 && pos.x() < BOARD_COLS && pos.y() >= 0 && pos.y() < BOARD_ROWS; 
}

bool BoardANDBench::isBenchPosition(const QPoint& pos) const // 检查位置是否在备战区范围内
{
    return pos.x() >= 0 && pos.x() < BENCH_COL && pos.y() >= BOARD_ROWS && pos.y() < BOARD_ROWS + BENCH_ROW;
}

bool BoardANDBench::isValidPosition(const QPoint& pos) const // 网格位置是否合法（在棋盘范围内）
{
    return (pos.x() >= 0 && pos.x() < BOARD_COLS && pos.y() >= 0 && pos.y() < BOARD_ROWS) || (pos.x() >= 0 && pos.x() < BENCH_COL && pos.y() >= BOARD_ROWS && pos.y() < BOARD_ROWS + BENCH_ROW);
}

bool BoardANDBench::isPlayerHalf(const QPoint& pos) const // 是否为玩家半区(包括备战席)
{
    return pos.y() >= BOARD_ROWS / 2 && pos.y() < BOARD_ROWS + BENCH_ROW; // 玩家半区包括棋盘下半部分和备战区
}

void BoardANDBench::clear()
{
    std::fill(m_cells.begin(), m_cells.end(), nullptr);
    m_unitToPosition.clear();
}

int BoardANDBench::indexOf(const QPoint& pos) const // 将二维坐标转换为一维索引
{
    if (!isValidPosition(pos)) {
        return -1;
    }
    if (isBoardPosition(pos)) {
        return pos.y() * BOARD_COLS + pos.x();
    }
    return BOARD_ROWS * BOARD_COLS + (pos.y() - BOARD_ROWS) * BENCH_COL + pos.x();
    
}
