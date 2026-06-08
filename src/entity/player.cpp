#include "player.h"
#include <algorithm>

Player::Player()
    : m_gold(20)
    , m_hp(100)
    , m_maxHp(100)
    , m_level(1)
    , m_xp(0)
    , m_xpToNext(4)
{}

bool Player::canAfford(int cost) const
{
    return m_gold >= cost;
}

void Player::spendGold(int cost)
{
    if (canAfford(cost)) {
        m_gold -= cost;
    }
}

void Player::addGold(int amount)
{
    m_gold += amount;
}

void Player::takeDamage(int damage)
{
    m_hp = std::max(0, m_hp - damage);
}

bool Player::isAlive() const
{
    return m_hp > 0;
}

void Player::addXp(int amount)
{
    m_xp += amount;
    while (m_xp >= m_xpToNext) {
        m_xp -= m_xpToNext;
        m_level++;
        m_xpToNext = calculateMaxXpForLevel(m_level);
    }
}

int Player::maxFieldUnits() const
{
    return 2 + (m_level - 1) / 2; // 每2级增加1个单位位，初始为2个
}

