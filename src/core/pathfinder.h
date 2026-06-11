#ifndef PATHFINDER_H
#define PATHFINDER_H

#include <QPoint>
#include <QList>
#include <QSet>

class Pathfinder {
public:

    struct Cube {int x, y, z;};
    
    static QList<QPoint> findPath(const QPoint& start, 
                                const QPoint& target, 
                                int attackRange, 
                                const QSet<QPoint>& occupied,
                                int boardRows,
                                int boardCols
                            );

    static int hexDistance(const QPoint& a, const QPoint& b); // 直角坐标的六边形距离
    static int hexDistance(const Cube& a, const Cube& b); // 立方体坐标的六边形距离
    static QList<QPoint> hexNeighbors(const QPoint& hex); // 获取直角坐标系下六边形的邻居
    static QList<Cube> hexNeighbors(const Cube& cube); // 获取立方体坐标的邻居

    static Cube offsetToCube(const QPoint& hex); // 将偏移坐标转换为立方体坐标
    static QPoint cubeToOffset(const Cube& cube); // 将立方体坐标转换为偏移坐标

private:
    



};

#endif // PATHFINDER_H