#include "enemyspawner.h"

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <algorithm>
#include <climits>
#include <QSet>
#include <QRandomGenerator>
#include "entity/unitdata.h"

namespace {
    UnitRole roleFromType(const QString& type) {
    if (type == "坦克" || type == "战士") return UnitRole::Frontline;
    if (type == "刺客") return UnitRole::Midline;
    if (type == "射手" || type == "辅助") return UnitRole::Backline;
    qWarning() << "Unknown unit type:" << type << ", defaulting to Frontline";
    return UnitRole::Frontline; // 安全默认
}

struct FormationSlot {
    int rowOffset;        // 相对行偏移（负值=更后排，0=前排基准）
    int colOffset;        // 相对列偏移（负=左，0=中，正=右）
    UnitRole preferredRole; // 这个槽位期望的角色
};

struct FormationTemplate {
    QString name; // 调试用名称
    QVector<FormationSlot> m_slots;
};

} // namespace



// 返回所有预定义的阵型模板
static QVector<FormationTemplate> buildTemplateLibrary() {
    using R = UnitRole;

    return {
        //  1 单位 
        {"solo-1", {
            { 0,  0, R::Frontline},                    // 居中，任何角色都行
        }},

        //  2 单位 
        {"line-2", {
            { 0, -1, R::Frontline},                    // 左前
            { 0, +1, R::Frontline},                    // 右前
        }},

        //  3 单位 
        {"line-3", {
            { 0, -1, R::Frontline},                    // 左前
            { 0,  0, R::Frontline},                    // 中前
            { 0, +1, R::Frontline},                    // 右前
        }},
        {"wedge-3", {
            { 0, -1, R::Frontline},                    // 左前
            { 0, +1, R::Frontline},                    // 右前
            {-1,  0, R::Backline},                     // 中后——前排双卫夹一个后排
        }},

        //  4 单位 
        {"box-4", {
            { 0, -1, R::Frontline},                    // 左前
            { 0, +1, R::Frontline},                    // 右前
            {-1, -1, R::Backline},                     // 左后
            {-1, +1, R::Backline},                     // 右后
        }},
        {"diamond-4", {
            { 0,  0, R::Frontline},                    // 前中——单前排
            {-1, -1, R::Midline},                      // 左中
            {-1, +1, R::Midline},                      // 右中
            {-2,  0, R::Backline},                     // 后中
        }},

        //  5 单位 
        {"v-5", {
            { 0, -1, R::Frontline},                    // 左前
            { 0,  0, R::Frontline},                    // 中前
            { 0, +1, R::Frontline},                    // 右前
            {-1, -1, R::Backline},                     // 左后
            {-1, +1, R::Backline},                     // 右后
        }},
        {"layer-5", {
            { 0, -1, R::Frontline},                    // 左前
            { 0, +1, R::Frontline},                    // 右前
            {-1, -1, R::Midline},                      // 左中
            {-1, +1, R::Midline},                      // 右中
            {-2,  0, R::Backline},                     // 后中
        }},

        //  6 单位
        {"double-6", {
            { 0, -1, R::Frontline},                    // 左前
            { 0,  0, R::Frontline},                    // 中前
            { 0, +1, R::Frontline},                    // 右前
            {-1, -1, R::Backline},                     // 左后
            {-1,  0, R::Backline},                     // 中后
            {-1, +1, R::Backline},                     // 右后
        }},
        {"wedge-6", {
            { 0, -1, R::Frontline},                    // 左前
            { 0,  0, R::Frontline},                    // 中前
            { 0, +1, R::Frontline},                    // 右前
            {-1, -1, R::Midline},                      // 左中
            {-1, +1, R::Midline},                      // 右中
            {-2,  0, R::Backline},                     // 后中
        }},

        //  7 单位 
        {"full-7", {
            { 0, -2, R::Frontline},                    // 左前
            { 0,  0, R::Frontline},                    // 中前
            { 0, +2, R::Frontline},                    // 右前
            {-1, -1, R::Midline},                      // 左中
            {-1, +1, R::Midline},                      // 右中
            {-2, -1, R::Backline},                     // 左后
            {-2, +1, R::Backline},                     // 右后
        }},

        //  8 单位(动态生成的后备)
        {"block-8", {
            { 0, -2, R::Frontline},  { 0, -1, R::Frontline},
            { 0, +1, R::Frontline},  { 0, +2, R::Frontline},
            {-1, -2, R::Backline},   {-1, -1, R::Backline},
            {-1, +1, R::Backline},   {-1, +2, R::Backline},
        }},
    };
}



// 代价矩阵
static int mismatchCost(UnitRole unitRole, UnitRole slotRole) {
    // 代价矩阵: costMatrix[unitRole][slotPreferredRole]
    //           slot: F   M   B
    static const int cost[3][3] = {
        /* unit F */ { 0,  1,  3},
        /* unit M */ { 2,  0,  1},
        /* unit B */ {10,  2,  0},
    };
    auto toIdx = [](UnitRole r) -> int {
        switch (r) {
        case UnitRole::Frontline: return 0;
        case UnitRole::Midline: return 1;
        case UnitRole::Backline: return 2;
        }
        return 0;
    };
    return cost[toIdx(unitRole)][toIdx(slotRole)];
}

// 给定一个排列 perm (unit[i] → slot[perm[i]])，计算总代价
static int evaluateAssignment(const QVector<UnitRole>& unitRoles,
                              const FormationTemplate& tmpl,
                              const QVector<int>& perm) {
    int total = 0;
    for (int i = 0; i < perm.size(); ++i) {
        total += mismatchCost(unitRoles[i], tmpl.m_slots[perm[i]].preferredRole);
    }
    return total;
}

// 全排列搜索最优 unit→slot 分配
// 返回 perm: unit[i] → slot[perm[i]]
static QVector<int> solveAssignment(const QVector<UnitRole>& unitRoles, const FormationTemplate& tmpl) {
    const int n = unitRoles.size(); // 单位数量
    Q_ASSERT(n == tmpl.m_slots.size() && n > 0);

    // 初始化排列: 0, 1, 2, ..., n-1
    QVector<int> indices(n);
    for (int i = 0; i < n; ++i) {
        indices[i] = i;
    }

    QVector<int> bestPerm = indices;
    int bestCost = evaluateAssignment(unitRoles, tmpl, indices);

    // 按字典序遍历所有排列
    while (std::next_permutation(indices.begin(), indices.end())) {
        const int cost = evaluateAssignment(unitRoles, tmpl, indices);
        if (cost < bestCost) {
            bestCost = cost;
            bestPerm = indices;
        }
    }

    return bestPerm;
}

// 分配结果
struct AssignmentResult {
    const FormationTemplate* tmpl; // 选中的模板（指向 buildTemplateLibrary() 返回的静态数据）
    QVector<int> perm; // unit[i] → slot[perm[i]] 的最优排列
    int totalCost; // 总代价，用于判断是否需要 HP 降级兜底
};

// 对所有槽位数匹配的模板逐一求解，返回全局最优
// 如果没有任何模板匹配 unitRoles.size()，返回 tmpl=nullptr，调用方应走动态生成
static AssignmentResult findBestFormation(const QVector<UnitRole>& unitRoles) {
    static const QVector<FormationTemplate> library = buildTemplateLibrary();
    const int n = unitRoles.size();

    // 筛选槽位数匹配的模板
    QVector<const FormationTemplate*> candidates;
    for (const auto& tmpl : library) {
        if (tmpl.m_slots.size() == n) {
            candidates.push_back(&tmpl);
        }
    }

    if (candidates.isEmpty()) {
        return {nullptr, {}, INT_MAX};
    }

    AssignmentResult best = {nullptr, {}, INT_MAX};
    for (const auto* tmpl : candidates) {
        QVector<int> perm = solveAssignment(unitRoles, *tmpl);
        int cost = evaluateAssignment(unitRoles, *tmpl, perm);
        if (cost < best.totalCost) {
            best = {tmpl, std::move(perm), cost};
        }
    }

    return best;
}

// 坐标映射 & 随机扰动 & HP 降级 & 动态生成
static constexpr int ENEMY_FRONT_ROW = 3;  // 敌方最前排 (row 3, 紧邻玩家半区 row 4~7)
static constexpr int CENTER_COL = 3;  // 棋盘中央偏左列 (8列取3, 与4构成双中心)

// 对单个 slot 做约束随机扰动, 返回合法绝对坐标
// occupied 用于防止单位重叠, 调用方按前排→后排顺序保证前排先占位
static QPoint jitteredPosition(const FormationSlot& slot,
                               QSet<QPoint>& occupied,
                               const BoardANDBench& board) {
    const int baseCol = CENTER_COL + slot.colOffset;
    const int baseRow = ENEMY_FRONT_ROW + slot.rowOffset;

    // 5轮尝试, 逐轮缩小扰动幅度
    for (int attempt = 0; attempt < 5; ++attempt) {
        int colJitter, rowJitter; // 行列偏移
        if (attempt < 2) {
            colJitter = QRandomGenerator::global()->bounded(3) - 1; // {-1, 0, +1}
            rowJitter = QRandomGenerator::global()->bounded(2); // {0, +1} 只向后不向前
        } else if (attempt < 4) {
            colJitter = QRandomGenerator::global()->bounded(3) - 1;
            rowJitter = 0;  // 仅列抖动
        } else {
            colJitter = 0;
            rowJitter = 0;  // 不抖, 回退基准
        }

        const QPoint candidate(baseCol + colJitter, baseRow + rowJitter);
        if (board.isBoardPosition(candidate) &&
            !board.isPlayerHalf(candidate) &&
            !occupied.contains(candidate)) {
            occupied.insert(candidate);
            return candidate;
        }
    }

    const QPoint base(baseCol, baseRow);
    occupied.insert(base);
    return base;
}

// HP 降级兜底: 当 cost ≥ 10 (有脆皮被迫上前排) 时, 无视角色,
// 纯按 HP 降序 → 前排 slot 分配 (HP 最高的顶最前)
static QVector<int> hpFallbackPerm(const QVector<int>& unitHps,
                                   const FormationTemplate& tmpl) {
    const int n = unitHps.size();

    // unit 索引按 HP 降序
    QVector<int> unitByHp(n);
    for (int i = 0; i < n; ++i) unitByHp[i] = i;
    std::sort(unitByHp.begin(), unitByHp.end(),
              [&](int a, int b) { return unitHps[a] > unitHps[b]; });

    // slot 索引按 "前排优先, 同排居中优先"
    QVector<int> slotByFront(n);
    for (int i = 0; i < n; ++i) slotByFront[i] = i;
    std::sort(slotByFront.begin(), slotByFront.end(),
              [&](int a, int b) {
                  const auto& sa = tmpl.m_slots[a];
                  const auto& sb = tmpl.m_slots[b];
                  if (sa.rowOffset != sb.rowOffset)
                      return sa.rowOffset > sb.rowOffset;  // 0 > -1 > -2
                  return std::abs(sa.colOffset) < std::abs(sb.colOffset);   // 居中优先
              });

    // 组装: HP 最高 → 最前 slot
    QVector<int> newPerm(n);
    for (int i = 0; i < n; ++i)
        newPerm[unitByHp[i]] = slotByFront[i];
    return newPerm;
}

// 按模板 + 分配结果放置单位, 前排→后排顺序逐个扰动
static void applyTemplateFormation(QVector<EnemySpawnPlan>& plans,
                                   const FormationTemplate& tmpl,
                                   const QVector<int>& perm,
                                   const BoardANDBench& board) {
    const int n = plans.size();
    QSet<QPoint> occupied;

    // 建立 slotIndex → unitIndex 逆向映射
    QVector<int> slotToUnit(n);
    for (int i = 0; i < n; ++i)
        slotToUnit[perm[i]] = i;

    // 按 rowOffset 从大到小 (前排→后排) 排序, 保证前排先占格
    QVector<int> slotOrder(n);
    for (int i = 0; i < n; ++i) slotOrder[i] = i;
    std::sort(slotOrder.begin(), slotOrder.end(),
              [&](int a, int b) {
                  return tmpl.m_slots[a].rowOffset > tmpl.m_slots[b].rowOffset;
              });

    for (int slotIdx : slotOrder) {
        int unitIdx = slotToUnit[slotIdx];
        plans[unitIdx].position = jitteredPosition(tmpl.m_slots[slotIdx], occupied, board);
    }
}

// 动态生成: 无模板匹配时, 按 "前排度" 排序后逐行填充
static void applyDynamicFormation(QVector<EnemySpawnPlan>& plans,
                                  const QVector<UnitRole>& unitRoles,
                                  const QVector<int>& unitHps,
                                  const BoardANDBench& board) {
    const int n = plans.size();
    if (n == 0) return;

    // 收集单位信息并按 "前排度" 排序: Frontline(0) > Midline(1) > Backline(2)
    // 同角色内 HP 高优先
    struct Indexed { int idx; UnitRole role; int hp; };
    QVector<Indexed> sorted(n); // idx → (role, hp)
    for (int i = 0; i < n; ++i)
        sorted[i] = {i, unitRoles[i], unitHps[i]};

    auto frontScore = [](UnitRole r) {
        if (r == UnitRole::Frontline) return 0;
        if (r == UnitRole::Midline) return 1;
        else return 2;
    };
    std::sort(sorted.begin(), sorted.end(), [&](const Indexed& a, const Indexed& b) {
        int sa = frontScore(a.role), sb = frontScore(b.role);
        if (sa != sb) return sa < sb;
        return a.hp > b.hp;
    });

    // 每行最多4个, 行数自适应
    const int colsPerRow = std::min(4, n);
    const int rowCount = (n + colsPerRow - 1) / colsPerRow; // 向上取整

    QSet<QPoint> occupied;
    int next = 0;

    for (int row = 0; row < rowCount; ++row) {
        const int unitsInRow = std::min(colsPerRow, n - next); // 当前行的单位数量
        int rowOffset = -row;  // row0→前排, row1→-1, …
        const int absRow = ENEMY_FRONT_ROW + rowOffset; // 绝对行号

        // 如果 rowOffset 超出棋盘顶部 (absRow < 0), 压缩回到 0~3 内循环
        if (absRow < 0)
            rowOffset = -(row % 4);

        for (int col = 0; col < unitsInRow; ++col) {
            const int colOffset = col - unitsInRow / 2;     // 居中分布
            FormationSlot slot = {rowOffset, colOffset, UnitRole::Frontline};
            int unitIdx = sorted[next++].idx;
            plans[unitIdx].position = jitteredPosition(slot, occupied, board);
        }
    }
}


EnemySpawner* EnemySpawner::instance() {
    static EnemySpawner ins;
    return &ins;
}


bool EnemySpawner::load(const QString& jsonPath) {
    QString resolvedPath = jsonPath;
    if (!QFileInfo::exists(resolvedPath)) {
        const QString appDir = QCoreApplication::applicationDirPath();
        const QString candidates[] = {
            appDir + "/data/stages.json",
            appDir + "/../data/stages.json",
            appDir + "/../../data/stages.json"
        };
        for (const auto& path : candidates) {
            if (QFileInfo::exists(path)) {
                resolvedPath = QFileInfo(path).absoluteFilePath();
                break;
            }
        }
    }

    QFile file(resolvedPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "EnemySpawner: cannot open" << resolvedPath;
        return false;
    }

    QByteArray data = file.readAll();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "EnemySpawner: JSON parse error" << parseError.errorString();
        return false;
    }
    if (!doc.isArray()) {
        qWarning() << "EnemySpawner: root must be an array";
        return false;
    }

    m_stageConfigs.clear();
    const QJsonArray array = doc.array();
    for (const auto& value : array) {
        if (!value.isObject()) {
            qWarning() << "EnemySpawner: array element is not an object, skipping";
            continue;
        }
        const QJsonObject obj = value.toObject();

        StageConfig config;
        config.battleIndex = obj.value("battleIndex").toInt(-1);
        config.minUnits = obj.value("minUnits").toInt(0);
        config.maxUnits = obj.value("maxUnits").toInt(0);
        config.star1Percent = obj.value("star1").toDouble(0.0);
        config.star2Percent = obj.value("star2").toDouble(0.0);
        config.star3Percent = obj.value("star3").toDouble(0.0);
        config.equipChance = obj.value("equipChance").toDouble(0.0);
        for (const auto& field : {"battleIndex", "minUnits", "maxUnits", "star1", "star2", "star3", "equipChance"}) {
            if (!obj.contains(field)) {
                qWarning() << "EnemySpawner: missing field" << field << "in battleIndex" << config.battleIndex;
            }
        }

        m_stageConfigs.insert(config.battleIndex, config);
    }

    qDebug() << "EnemySpawner: loaded" << m_stageConfigs.size() << "stage config(s)";
    return true;
}


QVector<EnemySpawnPlan> EnemySpawner::decideEnemy(int battleIndex) {
    QVector<EnemySpawnPlan> plans;
    if (m_stageConfigs.empty()) {
    qWarning() << "EnemySpawner: no stage configs loaded, cannot decide enemy spawns";
        return {};
    }

    auto it = std::lower_bound(m_stageConfigs.begin(), m_stageConfigs.end(), battleIndex,
        [](const StageConfig& c, int idx) { return c.battleIndex < idx; });

    const StageConfig& cfg = (it != m_stageConfigs.end()) ? *it : m_stageConfigs.last();

    int count = cfg.minUnits + (cfg.maxUnits > cfg.minUnits ? QRandomGenerator::global()->bounded(cfg.maxUnits - cfg.minUnits + 1) : 0);
    // 星级权重掷骰
    auto rollStar = [&cfg]() -> int {
        double r = QRandomGenerator::global()->bounded(100) / 100.0;
        if (r < cfg.star1Percent) return 1;
        if (r < cfg.star1Percent + cfg.star2Percent) return 2;
        return 3;
    };

    // 单位池
    const QStringList keys = UnitData::instance()->allKeys();
    if (keys.isEmpty()) {
        qWarning() << "EnemySpawner: unit data is empty, cannot decide enemy spawns";
        return {};
    }

    plans.reserve(count);

    for (int i = 0; i < count; ++i) {
        EnemySpawnPlan plan;
        plan.unitKey = keys[QRandomGenerator::global()->bounded(keys.size())];
        plan.starLevel = rollStar();
        plans.append(plan);
    }
    return plans;

}

QVector<EnemySpawnPlan> EnemySpawner::planFormation(QVector<EnemySpawnPlan>& initialPlans, const BoardANDBench& board) {
    if (initialPlans.isEmpty())
        return {};

    const auto unitTemplates = UnitData::instance()->allTemplates();
    const int n = initialPlans.size();

    // 收集角色和 HP (HP 用于极端情况降级排序)
    QVector<UnitRole> unitRoles(n);
    QVector<int> unitHps(n);
    for (int i = 0; i < n; ++i) {
        const auto& tmpl = unitTemplates.value(initialPlans[i].unitKey);
        unitRoles[i] = roleFromType(tmpl.type);
        unitHps[i] = static_cast<int>(tmpl.maxHp * Unit::starMultiplier(initialPlans[i].starLevel));
    }

    // 模板匹配 + 最优分配
    AssignmentResult assignment = findBestFormation(unitRoles);

    if (assignment.tmpl != nullptr) {
        // 代价过高 (有脆皮被迫上前排) → HP 降级兜底
        if (assignment.totalCost >= 10)
            assignment.perm = hpFallbackPerm(unitHps, *assignment.tmpl);

        applyTemplateFormation(initialPlans, *assignment.tmpl, assignment.perm, board);
    } else {
        // 无匹配模板 → 动态生成方阵
        applyDynamicFormation(initialPlans, unitRoles, unitHps, board);
    }

    return initialPlans;
}

QVector<EnemySpawnPlan> EnemySpawner::generateEnemy(int battleIndex, const BoardANDBench& board) {
    auto plans = decideEnemy(battleIndex);
    return planFormation(plans, board);
}