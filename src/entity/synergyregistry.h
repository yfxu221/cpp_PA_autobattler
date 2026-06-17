#ifndef SYNERGYREGISTRY_H
#define SYNERGYREGISTRY_H

#include <QString>
#include <QMap>

// 单个羁绊提供的加成数值
struct SynergyBonus {
    int bonusAtk = 0;
    int bonusMaxHp = 0;
    int bonusMaxMana = 0;
    int bonusSpeed = 0;
    int dotIntervalReduction = 0;
    float bonusMoveSpeed = 0.0f;

    SynergyBonus& operator+=(const SynergyBonus& other) {
        bonusAtk += other.bonusAtk;
        bonusMaxHp += other.bonusMaxHp;
        bonusMaxMana += other.bonusMaxMana;
        bonusSpeed += other.bonusSpeed;
        dotIntervalReduction += other.dotIntervalReduction;
        bonusMoveSpeed += other.bonusMoveSpeed;
        return *this;
    }
};

// 单个阈值条目
struct SynergyThreshold {
    int count; // 需要的单位数量
    SynergyBonus bonus; // 达到该数量时的加成
};

// 用于 UI 展示的阈值信息
struct ThresholdInfo {
    int count;
    SynergyBonus bonus;
    bool reached; // 是否已达到
};

class SynergyRegistry {
public:
    static SynergyRegistry* instance();

    bool load(const QString& jsonPath);  // 加载 synergies.json

    // 根据 trait 和场上数量，返回该 trait 提供的加成（取最高满足的阈值）
    SynergyBonus getBonus(const QString& trait, int count) const;

    // 返回所有已注册的 trait 名称
    QStringList allTraits() const;

    // 返回某个 trait 的全部阈值信息（用于 UI 展示 "神: 2/4, ATK+10"）
    QVector<SynergyThreshold> thresholds(const QString& trait) const;

private:
    // trait名称 → 该trait的所有阈值（已按count升序排列）
    QMap<QString, QVector<SynergyThreshold>> m_rules;
};



#endif // SYNERGYREGISTRY_H