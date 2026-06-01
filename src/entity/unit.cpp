#include "unit.h"

int Unit::s_nextId = 0; // 静态成员变量初始化，确保每个单位都有一个唯一的ID，从0开始递增

Unit::Unit(const QString& name)
    : m_id(s_nextId++)
    , m_name(name)
    , m_position(0, 0)
{}
