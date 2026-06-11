#include "pathfinder.h"
#include <QQueue>
#include <QVector>

Pathfinder::Cube Pathfinder::offsetToCube(const QPoint& hex) { // even-r 场景坐标转换为立方体坐标
    int x = hex.x() - (hex.y() + (hex.y() & 1)) / 2;
    int z = hex.y();
    int y = -x - z;
    return {x, y, z};
}

QPoint Pathfinder::cubeToOffset(const Cube& cube) { // 立方体坐标转换为 even-r 场景坐标
    int col = cube.x + (cube.z + (cube.z & 1)) / 2;
    int row = cube.z;
    return QPoint(col, row);
}

int Pathfinder::hexDistance(const QPoint& a, const QPoint& b) {
    Cube ac = offsetToCube(a);
    Cube bc = offsetToCube(b);
    return (std::abs(ac.x - bc.x) + std::abs(ac.y - bc.y) + std::abs(ac.z - bc.z)) / 2;
}

int Pathfinder::hexDistance(const Cube& a, const Cube& b) {
    return (std::abs(a.x - b.x) + std::abs(a.y - b.y) + std::abs(a.z - b.z)) / 2;
}

QList<QPoint> Pathfinder::hexNeighbors(const QPoint& hex) {
    Cube c = offsetToCube(hex);
    QList<Cube> neighbors = hexNeighbors(c);
    QList<QPoint> result;
    for (const Cube& nc : neighbors) {
        result.append(cubeToOffset(nc));
    }
    return result;
}

QList<Pathfinder::Cube> Pathfinder::hexNeighbors(const Cube& cube) {
    static const Cube directions[6] = {
        {1, -1, 0}, {1, 0, -1}, {0, 1, -1},
        {-1, 1, 0}, {-1, 0, 1}, {0, -1, 1}
    };
    QList<Cube> neighbors;
    for (const auto& dir : directions) {
        neighbors.append({cube.x + dir.x, cube.y + dir.y, cube.z + dir.z});
    }
    return neighbors;
}

QList<QPoint> Pathfinder::findPath(const QPoint& start, const QPoint& target, int attackRange, const QSet<QPoint>& occupied, int boardRows, int boardCols) {
    if (start == target) {
        return {};
    }
    
    if (hexDistance(start, target) <= attackRange) {
        return {start};
    }

    QVector<QVector<bool>> visited(boardRows, QVector<bool>(boardCols, false));
    QVector<QVector<QPoint>> parent(boardRows, QVector<QPoint>(boardCols, QPoint(-1, -1))); // 记录路径
    QQueue<QPoint> queue;
    queue.enqueue(start);
    visited[start.y()][start.x()] = true;

    QPoint destination(-1, -1);
    bool foundPath = false;

    while (!queue.isEmpty()) {
        QPoint curr = queue.dequeue();

        if (hexDistance(curr, target) <= attackRange) {
            destination = curr;
            foundPath = true;
            break;
        }

        QList<QPoint> neighbors = hexNeighbors(curr);
        for (const QPoint& next : neighbors) {
            int nx = next.x();
            int ny = next.y();

            if (nx < 0 || nx >= boardCols || ny < 0 || ny >= boardRows) {
                continue;
            }
            if (visited[ny][nx]) {
                continue;
            }
            if (occupied.contains(next) && next != target) {
                continue;
            }

            visited[ny][nx] = true;
            parent[ny][nx] = curr;
            queue.enqueue(next);
        }
    }

    if (!foundPath) {
        return QList<QPoint>();
    }

    QList<QPoint> path;
    QPoint curr = destination;
    while (curr != start) {
        path.append(curr);
        curr = parent[curr.y()][curr.x()];
    }
    path.append(start);

    std::reverse(path.begin(), path.end()); // 翻转顺序
    return path;
}