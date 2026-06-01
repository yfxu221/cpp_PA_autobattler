#ifndef UNIT_H
#define UNIT_H

#include <QPoint>
#include <QString>

class Unit // 代表一个单位的类，包含单位的ID、名称和位置等属性
{
public:
    explicit Unit(const QString& name = QString("Unit"));
    ~Unit() = default;

    int id() const { return m_id; } // 获取单位的唯一ID，单位ID在创建时自动分配，确保每个单位都有一个独特的标识符
    QString name() const { return m_name; } // 获取单位的名称
    QPoint position() const { return m_position; } // 获取单位在棋盘上的位置，使用网格坐标表示（列, 行）

    void setName(const QString& name) { m_name = name; }
    void setPosition(const QPoint& pos) { m_position = pos; }

private:
    static int s_nextId; // 静态成员变量，用于生成唯一的单位ID，每创建一个单位，s_nextId就会递增，确保每个单位都有一个独特的ID

    int m_id; // 单位的唯一ID，由s_nextId自动分配
    QString m_name; // 单位的名称，可以在创建单位时指定，也可以后续修改
    QPoint m_position; // 单位在棋盘上的位置，使用网格坐标表示（列, 行）
};

#endif // UNIT_H
