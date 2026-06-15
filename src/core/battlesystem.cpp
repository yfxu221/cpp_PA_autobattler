#include "battlesystem.h"
#include <algorithm>
#include <QHash>
#include "entity/skill.h"

// ============================================================
// 匿名命名空间 — 辅助函数，仅在当前翻译单元可见
// ============================================================
namespace {

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

    // 优先释放技能
    QVector<TargetInfo> info = unit->hasSkill() ? unit->skill()->selectTargets(*unit, getBoardUnits()) : QVector<TargetInfo>(); 
    if (unit->canUseSkill() && !info.isEmpty()) {
        unit->setState(UnitState::Casting);
        return {unit, QPoint(-1, -1), {}}; // 技能的目标信息会在 makeSkill 里通过 skill()->execute() 获取，这里不需要提前放在 PlannedAction 里
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
    skillAction(actions);
    attackAction(actions);
}

void BattleSystem::moveAction(QList<PlannedAction>& actions)
{
    QSet<QPoint> actualOccupied; // 不移动单位的当前位置，以及已确定移动落点的位置
    QHash<QPoint, int> targetCounts; // 移动单位目标位置的计数（处理时递减，仅用于记账）

    for (const auto& action : actions) {
        if (action.unit->state() == UnitState::Moving)
            targetCounts[action.targetPos]++;
        else
            actualOccupied.insert(action.unit->position());
    }

    QList<PlannedAction> moveActions;
    std::copy_if(actions.begin(), actions.end(),
                 std::back_inserter(moveActions),
                 [](const auto& a) { return a.unit->state() == UnitState::Moving; });
    std::sort(moveActions.begin(), moveActions.end(), moveActionLess);

    for (const auto& action : moveActions) {
        // 撤销自己的目标预留，targetCounts 不再参与阻塞判断
        if (--targetCounts[action.targetPos] <= 0)
            targetCounts.remove(action.targetPos);

        if (!actualOccupied.contains(action.targetPos)) {
            makeMove(action.unit, action.targetPos);
            actualOccupied.insert(action.targetPos);
        } else {
            QList<QPoint> candidates;
            candidates.append(action.unit->position()); // 原地不动作为兜底
            for (const QPoint& neighbor : Pathfinder::hexNeighbors(action.unit->position())) {
                if (!actualOccupied.contains(neighbor) && m_board->isBoardPosition(neighbor))
                    candidates.append(neighbor);
            }

            std::sort(candidates.begin(), candidates.end(),
                      [&](const QPoint& a, const QPoint& b) {
                          return fallbackLess(a, b, action.targetPos);
                      });

            if (candidates[0] != action.unit->position()) {
                makeMove(action.unit, candidates[0]);
                actualOccupied.insert(candidates[0]);
            } else {
                action.unit->setState(UnitState::Idle);
                actualOccupied.insert(action.unit->position());
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
    attacker->setMana(std::min(attacker->maxMana(), attacker->mana() + 5)); // 每次攻击增加5点法力值
}

void BattleSystem::skillAction(QList<PlannedAction>& actions) {
    QList<PlannedAction> skillActions;
    std::copy_if(actions.begin(), actions.end(),
                 std::back_inserter(skillActions),
                 [](const auto& a) { return a.unit->state() == UnitState::Casting; });

    for (const auto& action : skillActions) {
        makeSkill(action.unit, getBoardUnits());
    }
}

void BattleSystem::makeSkill(Unit* caster, const QVector<Unit*>& allUnits) {
    const auto& skill = caster->skill();
    if (!skill) return;
    SkillResult skillResult = skill->execute(*caster, allUnits);
    if (!skillResult.success) return;
    for (const auto& hit : skillResult.hits) {
        m_pendingDamageEvents.append({caster, {hit.target}, -hit.value}); // HitInfo.value 正数=治疗量, 负数=伤害量
    }
    caster->setMana(0); // 使用技能后法力值归零
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

QVector<Unit*> BattleSystem::getBoardUnits() const {
    QVector<Unit*> boardUnits;
    for (Unit* unit : *m_units) {
        if (unit->isAlive() && m_board->isBoardPosition(unit->position())) {
            boardUnits.append(unit);
        }
    }
    return boardUnits;
}