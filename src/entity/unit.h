#ifndef UNIT_H
#define UNIT_H

#include <QPoint>
#include <QString>

class Unit
{
public:
    explicit Unit(const QString& name = QString("Unit"));
    ~Unit() = default;

    int id() const { return m_id; }
    QString name() const { return m_name; }
    QPoint position() const { return m_position; }

    void setName(const QString& name) { m_name = name; }
    void setPosition(const QPoint& pos) { m_position = pos; }

private:
    static int s_nextId;

    int m_id;
    QString m_name;
    QPoint m_position;
};

#endif // UNIT_H
