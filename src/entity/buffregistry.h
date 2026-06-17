#ifndef BUFFREGISTRY_H
#define BUFFREGISTRY_H

#include <QString>
#include <QMap>
#include <QList>

// buff 大类：决定战斗系统中如何处理
enum class BuffCategory {
    Control, // 控制类 — 阻止单位行动（眩晕等）
    Dot, // 持续伤害 — 每 tick 造成伤害
    StatMod, // 属性修正 — 临时修改单位属性（正=增益, 负=减益）
};

// 叠加规则
enum class BuffStackRule {
    Refresh,  // 刷新：同 key → max(剩余时间, 新时间)，效果取 max
    UniquePerSource, // 同源唯一：同 source + 同 key → Refresh；异 source + 同 key → 独立实例
    Independent,  // 完全独立：每次施加都是新实例，各自计时
};

// 受影响的属性（仅 StatMod 类有意义）
enum class BuffStat {
    None,  // Control / Dot 类无属性
    ATK,  // 攻击力
    MaxMana,  // 最大法力值
};

// 单个 buff 类型定义
struct BuffDef {
    QString key;
    BuffCategory category = BuffCategory::Control; // 技能类型
    BuffStackRule stackRule = BuffStackRule::Refresh; // 刷新叠加规则
    BuffStat stat = BuffStat::None; // 仅 StatMod 类别有效
};

// Buff 运行时实例 — 挂在 Unit 上
struct BuffInstance {
    QString buffKey;  // 对应 BuffDef::key
    int sourceUnitId = -1; // 谁施加的（UniquePerSource 同源判定用）
    int remainingTicks = 0; // 剩余 tick 数
    int totalTicks = 0; // 原始总 tick 数（Refresh 规则下用于 max 比较）
    float magnitude = 0.0f; // 效果值（施放时计算，之后不变）
    int instanceId = 0;  // 唯一实例 ID（Independent 叠加区分用）
    int damageInterval = 6;   // DoT 伤害间隔（tick数），默认6=300ms
    int damageIntervalCounter = 0;   // 当前间隔倒计时（0=本tick可触发）
};


class BuffRegistry {
public:
    static BuffRegistry* instance();

    // 从 bufftypes.json 加载所有 buff 类型定义
    bool load(const QString& jsonPath);

    // 按 key 查找定义
    const BuffDef* get(const QString& key) const;

    // 返回所有 key
    QList<QString> allKeys() const;

private:
    BuffRegistry() = default;
    QMap<QString, BuffDef> m_defs;
};

#endif // BUFFREGISTRY_H
