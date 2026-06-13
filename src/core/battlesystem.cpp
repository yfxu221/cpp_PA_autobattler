#include "battlesystem.h"
#include <algorithm>

// ============================================================
// 匿名命名空间 — 辅助函数，仅在当前翻译单元可见
// ============================================================
namespace {

// 收集当前所有已占据的位置
//   移动中单位 → 目标位置；其他 → 当前位置
QList<QPoint> collectOccupiedPositions(const QList<PlannedAction>& actions)
{
    QList<QPoint> positions;
    for (const auto& action : actions) {
        if (action.unit->state() == UnitState::Moving)
            positions.append(action.targetPos);
        else
            positions.append(action.unit->position());
    }
    return positions;
}

// 移动优先级排序：速度高 → ID 小 → Y 大 → X 大
bool moveActionLess(const PlannedAction& a, const PlannedAction& b)
{
    if (a.unit->speed() != b.unit->speed())
        return a.unit->speed() > b.unit->speed();
    if (a.unit->id() != b.unit->id())
        return a.unit->id() < b.unit->id();
    if (a.unit->position().y() != b.unit->position().y())
        return a.unit->position().y() > b.unit->position().y();
    return a.unit->position().x() > b.unit->position().x();
}

// 备选位置排序：离目标越近越优先，距离相同时 Y 大 → X 大
bool fallbackLess(const QPoint& a, const QPoint& b, const QPoint& target)
{
    int distA = Pathfinder::hexDistance(a, target);
    int distB = Pathfinder::hexDistance(b, target);
    if (distA != distB) return distA < distB;
    if (a.y() != b.y()) return a.y() > b.y();
    return a.x() > b.x();
}

// 选择攻击目标排序：距离近 → 血量低 → Y 小 → X 小
bool targetLess(Unit* self, Unit* a, Unit* b)
{
    int distA = Pathfinder::hexDistance(self->position(), a->position());
    int distB = Pathfinder::hexDistance(self->position(), b->position());
    if (distA != distB) return distA < distB;
    if (a->hp() != b->hp()) return a->hp() < b->hp();
    if (a->position().y() != b->position().y())
        return a->position().y() < b->position().y();
    if (a->position().x() != b->position().x())
        return a->position().x() < b->position().x();
    return false;
}

} // namespace



BattleSystem::BattleSystem(QObject *parent)
    : QObject(parent)
{
}

void BattleSystem::start(BoardANDBench& board,
                         QList<Unit*>& units,
                         Player* player,
                         Player* enemy)
{
    m_board = &board;
    m_units = &units;
    m_player = player;
    m_enemy = enemy;

    if (!m_timer) {
        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, this, &BattleSystem::onBattleTick);
    }
    m_tickCount = 0;
    m_timer->start(TICK_INTERVAL_MS);
}

void BattleSystem::stop() {
    if (m_timer) {
        m_timer->stop();
    }
}

Unit* BattleSystem::selectTarget(Unit* self, const QList<Unit*>& enemies) {
    if (enemies.isEmpty()) {
        return nullptr;
    }

    auto it = std::min_element(enemies.begin(), enemies.end(),
        [self](Unit* a, Unit* b) { return targetLess(self, a, b); });

    return *it;
    
}


void BattleSystem::updateUnits()
{
    for(Unit* unit : *m_units)
    {
        if(!unit->isAlive()){
            unit->setState(UnitState::Dead);
            continue;
        }
        unit->processCooldown();
    }
}

void BattleSystem::onBattleTick() {
    QList<PlannedAction> actions;
    for(Unit* unit : *m_units){
        if(unit->isAlive()){
            actions.push_back(decideAction(unit));
        }
    }
    resolveActions(actions);
    resolveDamage();
    updateUnits();
    resolveDeaths();

    emit stateUpdated();
    BattleResult result = checkEndCondition();
    if (result != BattleResult::Ongoing) {
        battleFinished(result);   // 战斗结束，通知 Game 进入结算
        return;
    }
}

PlannedAction BattleSystem::decideAction(Unit* unit) {
    // 备战席上的单位不参与战斗寻路
    if (!m_board->isBoardPosition(unit->position())) {
        unit->setState(UnitState::Idle);
        return {unit, QPoint(-1,-1), {}};
    }

    Unit* target = selectTarget(unit, getUnitsByOwner(unit->owner() == PlayerCtrl ? EnemyCtrl : PlayerCtrl));
    if(target == nullptr) {
        unit->setState(UnitState::Idle);
        return {unit, QPoint(-1,-1), {}};
    }
    // 优先攻击范围内的目标
    if(Pathfinder::hexDistance(unit->position(), target->position()) <= unit->range()) {
        if (!unit->isCooldownReady()) {
            unit->setState(UnitState::Idle);
            return {unit, QPoint(-1,-1), {}};
        }
        unit->setState(UnitState::Attacking);
        return {unit, target->position(), {target}};
    }
    // 否则移动到攻击范围内
    else{
        QList<QPoint> path = Pathfinder::findPath(unit->position(), target->position(), unit->range(), getOccupiedPositions(), m_board->BOARD_ROWS, m_board->BOARD_COLS);
        // path 为空 → 无路可达；path.size()<=1 → 已在攻击范围内（findPath 返回 {start}）
        if(path.isEmpty() || path.size() <= 1) {
            unit->setState(UnitState::Idle);
            return {unit, QPoint(-1,-1), {}};
        }
        unit->setState(UnitState::Moving);
        return {unit, path[1], {}};
    }
}

void BattleSystem::resolveActions(QList<PlannedAction>& actions) {
    moveAction(actions);
    attackAction(actions);
}

void BattleSystem::moveAction(QList<PlannedAction>& actions)
{
    // 1. 收集当前占据位置
    QList<QPoint> occupiedPositions = collectOccupiedPositions(actions);

    // 2. 筛选出移动单位，按优先级排序
    QList<PlannedAction> moveActions;
    std::copy_if(actions.begin(), actions.end(),
                 std::back_inserter(moveActions),
                 [](const auto& a) { return a.unit->state() == UnitState::Moving; });
    std::sort(moveActions.begin(), moveActions.end(), moveActionLess);

    // 3. 按优先级依次执行移动
    for (const auto& action : moveActions) {
        // 先排除当前单位自己的目标标记，判断是否被其他单位占据
        occupiedPositions.removeOne(action.targetPos);

        if (!occupiedPositions.contains(action.targetPos)) {
            // 目标仅被自己标记过 → 直接移动
            makeMove(action.unit, action.targetPos);
            occupiedPositions.append(action.targetPos);
        } else {
            // 目标确实被其他单位占据 → 收集空闲邻居备选
            QList<QPoint> candidates;
            candidates.append(action.unit->position()); // 原地不动作为兜底
            for (const QPoint& neighbor : Pathfinder::hexNeighbors(action.unit->position())) {
                if (!occupiedPositions.contains(neighbor) && m_board->isBoardPosition(neighbor))
                    candidates.append(neighbor);
            }

            // 按离目标位置的距离排序，取最佳备选
            std::sort(candidates.begin(), candidates.end(),
                      [&](const QPoint& a, const QPoint& b) {
                          return fallbackLess(a, b, action.targetPos);
                      });

            if (candidates[0] != action.unit->position()) {
                makeMove(action.unit, candidates[0]);
                occupiedPositions.append(candidates[0]);
            } else {
                action.unit->setState(UnitState::Idle);
                occupiedPositions.append(action.unit->position());
            }
        }
    }
}

void BattleSystem::makeMove(Unit* unit, const QPoint& targetPos) {
    m_board->moveUnit(unit, targetPos);
}

void BattleSystem::attackAction(QList<PlannedAction>& actions){
    QList<PlannedAction> attackActions;
    std::copy_if(actions.begin(), actions.end(),
                 std::back_inserter(attackActions),
                 [](const auto& a) { return a.unit->state() == UnitState::Attacking; });

    for (const auto& action : attackActions) {
        makeAttack(action.unit, action.targetUnits);
    }
}

void BattleSystem::makeAttack(Unit* attacker, const QSet<Unit*>& targets) {
    m_pendingDamageEvents.append({attacker, targets, attacker->atk()});
    attacker->resetCooldown();
}

void BattleSystem::resolveDamage() {
    for (const auto& event : m_pendingDamageEvents) {
        for (Unit* target : event.targets) {
            target->takeDamage(event.damage);
        }
    }
    m_pendingDamageEvents.clear();
}

void BattleSystem::resolveDeaths() {
    for (Unit* unit : *m_units) {
        if (unit->isAlive()) {continue;}
        unit->setState(UnitState::Dead);
        // 可能的死亡处理
    }
}

BattleResult BattleSystem::checkEndCondition()
{
    bool playerAlive = false;
    bool enemyAlive = false;

    for (Unit* unit : *m_units) {
        if (!unit->isAlive()) continue;
        if (unit->owner() == PlayerCtrl)
            playerAlive = true;
        else
            enemyAlive = true;
    }
    if (!playerAlive && !enemyAlive)
        return BattleResult::Draw;
    if (!playerAlive)
        return BattleResult::EnemyWin;
    if (!enemyAlive)
        return BattleResult::PlayerWin;
    return BattleResult::Ongoing;
}







QList<Unit*> BattleSystem::getUnitsByOwner(Owner owner) const {
    QList<Unit*> result;
    for (Unit* unit : *m_units) {
        if (unit->owner() == owner && unit->isAlive()) {
            result.append(unit);
        }
    }
    return result;
}

QSet<QPoint> BattleSystem::getOccupiedPositions() const {
    QSet<QPoint> occupied;
    for (Unit* unit : *m_units) {
        if (unit->isAlive()) {
            occupied.insert(unit->position());
        }
    }
    return occupied;
}