#ifndef PLAYER_H
#define PLAYER_H

class Player {
public:
    Player();
    bool canAfford(int cost) const; // 检查玩家是否有足够的金币支付指定的费用
    void spendGold(int cost); // 花费指定数量的金币，减少玩家的金币
    void addGold(int amount); // 增加指定数量的金币，增加玩家的金币
    void takeDamage(int damage); // 受到伤害，减少玩家的生命值
    bool isAlive() const; // 判断玩家是否存活
    void addXp(int amount); // 增加指定数量的经验值，增加玩家的经验
    int maxFieldUnits() const; // 获取玩家在棋盘上可以拥有的最大单位数量，与玩家等级相关
    void setHp(int hp) { m_hp = hp; } // 设置玩家的生命值
    void setGold(int gold) { m_gold = gold; } // 设置玩家的金币数量
    void setXp(int xp) { m_xp = xp; } // 设置玩家的经验值

    int gold() const { return m_gold; } // 获取玩家当前的金币数量
    int hp() const { return m_hp; } // 获取玩家当前的生命值
    int maxHp() const { return m_maxHp; } // 获取玩家最大生命值
    int level() const { return m_level; } // 获取玩家当前的等级
    int xp() const { return m_xp; } // 获取玩家当前的经验值
    int xpToNext() const { return m_xpToNext; } // 获取玩家升级所需的经验值
    int maxXp() const {return calculateMaxXpForLevel(m_maxLevel);} // 获取玩家可以达到的最大经验值，等同于达到最高等级所需的经验值


private:
    int m_gold; // 玩家当前的金币数量
    int m_hp; // 玩家当前的生命值
    int m_maxHp; // 玩家最大生命值
    int m_level; // 玩家当前的等级
    int m_xp; // 玩家当前的经验值
    int m_xpToNext; // 升级所需的经验值
    int m_maxLevel = 7; // 玩家可以达到的最高等级

    int calculateMaxXpForLevel(int level) const { return 4 + (level - 1) * 2; } // 计算指定等级所需的最大经验值

};

#endif // PLAYER_H