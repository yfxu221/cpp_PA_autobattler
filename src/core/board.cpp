#include "board.h"

Board::Board()
    : m_cells(ROWS * COLS, nullptr)
{}

void Board::addUnit(Unit* unit, const QPoint& pos)
{
    const int idx = indexOf(pos);
    if (!unit || idx < 0 || m_cells[idx]) {
        return;
    }

    m_cells[idx] = unit;
    m_unitToPosition[unit] = pos;
    unit->setPosition(pos);
}

void Board::removeUnit(Unit* unit)
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

Unit* Board::getUnitAt(const QPoint& pos) const // 获取指定位置的单位
{
    const int idx = indexOf(pos);
    return idx < 0 ? nullptr : m_cells[idx];
}

bool Board::hasUnitAt(const QPoint& pos) const // 检查指定位置是否有单位
{
    return getUnitAt(pos) != nullptr;
}

bool Board::isValidPosition(const QPoint& pos) const // 网格位置是否合法（在棋盘范围内）
{
    return pos.x() >= 0 && pos.x() < COLS && pos.y() >= 0 && pos.y() < ROWS;
}

bool Board::isPlayerHalf(const QPoint& pos) const // 是否为玩家半区
{
    return pos.y() >= ROWS / 2;
}

void Board::clear()
{
    std::fill(m_cells.begin(), m_cells.end(), nullptr);
    m_unitToPosition.clear();
}

int Board::indexOf(const QPoint& pos) const // 将二维坐标转换为一维索引
{
    if (!isValidPosition(pos)) {
        return -1;
    }
    return pos.y() * COLS + pos.x();
}
