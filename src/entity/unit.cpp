#include "unit.h"

int Unit::s_nextId = 0;

Unit::Unit(const QString& name)
    : m_id(s_nextId++)
    , m_name(name)
    , m_position(0, 0)
{}
