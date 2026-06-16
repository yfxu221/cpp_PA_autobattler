#include "game.h"
#include "entity/unit.h"
#include "gui/griditem.h"
#include "gui/unititem.h"
#include "gui/storerefreshbutton.h"
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QtMath>
#include "entity/unitdata.h"
#include "entity/equipmentdata.h"
#include "gui/equipslotitem.h"
#include <QDebug>
#include <cstdlib>
#include <algorithm>
#include <set>
#include "core/enemyspawner.h"

namespace {
constexpr qreal kZGrid = 0.0; // 网格z坐标
constexpr qreal kZUnit = 1.0; // 单位z坐标
constexpr qreal kZDraggingUnit = 2.0; // 拖拽单位z坐标
}

Game::Game(QObject* parent)
    : QObject(parent)
    , m_scene(new QGraphicsScene(this))
    , m_dragActive(false)
    , m_activeUnitId(-1)
    , m_sourceGrid(-1, -1)
    , m_board_rows(BoardANDBench::BOARD_ROWS)
    , m_board_cols(BoardANDBench::BOARD_COLS)
    , m_bench_rows(BoardANDBench::BENCH_ROW)
    , m_bench_cols(BoardANDBench::BENCH_COL)
    , m_radius(46.0)
    , m_rowSpacing(69.0)
    , m_BBSpacing(69.0)
    , m_player()
    , m_enemy()
    , m_dragStartPlace(DragStartPlace::None)
    , m_phase(GamePhase::Preparation)
{}

Game::~Game()
{
    // unique_ptr 自动析构，无需手动清理
}

void Game::initialize()
{
    // 加载羁绊规则
    if (!SynergyRegistry::instance()->load("")) {
        qWarning() << "Game: 加载羁绊数据失败";
    }
    createStarterUnitsIfNeeded();
    buildScene();
    buildStoreScene();
    buildEquipBar();
    reset();
}

void Game::reset()
{
    m_phase = GamePhase::Preparation;
    emit phaseChanged(GamePhase::Preparation);
    m_board.clear();

    // 玩家单位 → 备战席; 敌方单位 → 其 plan.position
    int benchCol = 0;
    for (const auto& uptr : m_units) {
        if (uptr->owner() == Owner::PlayerCtrl) {
            const QPoint benchPos(benchCol, m_board_rows); // 备战席第一行
            if (m_board.isValidPosition(benchPos)) {
                m_board.addUnit(uptr.get(), benchPos);
                ++benchCol;
            }
        } else {
            m_board.addUnit(uptr.get(), uptr->position());
        }
    }

    recalculateSynergies();
    syncFromBoard();
}

void Game::handleDragStarted(int unitId, const QPoint& sourceGrid, const QPointF&)
{   
    if (m_phase != GamePhase::Preparation) {
        return;
    }
    
    m_dragActive = true;
    m_activeUnitId = unitId;
    m_sourceGrid = sourceGrid;
    if (m_board.isValidPosition(sourceGrid) && !m_board.isBenchPosition(sourceGrid)) {
        m_dragStartPlace = DragStartPlace::Board;
    }else{
        m_dragStartPlace = DragStartPlace::Bench;
    }

    UnitItem* item = findUnitItem(unitId);
    if (item) {
        item->setZValue(kZDraggingUnit); // 提升正在拖动的单位的Z值，使其显示在其他元素之上
    }
}

void Game::handleDragMoved(int unitId, const QPoint&, const QPointF& scenePos)
{
    if (!m_dragActive) {
        return;
    }

    clearGridHighlights();

    // 先判断是否在出售区上
    if (m_sellZone && isOverSellZone(scenePos)) {
        m_sellZone->setHighlighted(true);
        return;
    } else if(m_sellZone){
        m_sellZone->setHighlighted(false);
    }

    // 不在出售区上，继续判断是否在棋盘格子上
    const QPoint target = worldToGrid(scenePos);
    GridItem* targetItem = findGridItem(target);
    if (!targetItem) {
        return;
    }

    targetItem->setHoverActive(true);

    if (canApplyDrop(unitId, m_sourceGrid, target)) {
        targetItem->setDropActive(true);
    }
}

void Game::handleDropCommand(int unitId, const QPoint& sourceGrid, const QPointF& scenePos)
{
    if (!m_dragActive) {
        return;
    }

    // 先判断是否拖动到出售区
    if (isOverSellZone(scenePos)) {
        sellUnit(unitId);
        // 清理拖拽状态
        m_dragActive = false;
        m_activeUnitId = -1;
        m_sourceGrid = QPoint(-1, -1);
        if (m_sellZone) {
            m_sellZone->setHighlighted(false);
        }
        recalculateSynergies();
        syncFromBoard();
        return;
    }
    if (m_sellZone) {
        m_sellZone->setHighlighted(false);
    }

    const QPoint target = worldToGrid(scenePos);

    clearGridHighlights();
    if (canApplyDrop(unitId, sourceGrid, target)) {
        applyDrop(unitId, target);
    }

    UnitItem* item = findUnitItem(m_activeUnitId);
    if (item) {
        item->setZValue(kZUnit);
    }

    m_dragActive = false;
    m_activeUnitId = -1;
    m_sourceGrid = QPoint(-1, -1);

    recalculateSynergies();
    syncFromBoard();
}

void Game::createStarterUnitsIfNeeded()
{
    if (!m_units.empty()) {
        return;
    }

    SkillRegistry::instance(); // 加载技能
    if (!SkillRegistry::instance()->load("")) {
        qFatal("无法加载技能数据文件");
    }
    EquipmentRegistry::instance(); // 加载装备
    if (!EquipmentRegistry::instance()->load("")) {
        qWarning() << "Game: 加载装备数据失败";
    }
    UnitData* unitData = UnitData::instance();
    if (!unitData->load("")) {
        qFatal("无法加载单位数据文件");
    }

    // 加载敌人生成器
    EnemySpawner::instance()->load("");

    // ━━ 随机生成 2 个玩家初始单位 (1星, 无装备) ━━
    const QStringList keys = unitData->allKeys();
    if (keys.isEmpty()) {
        qFatal("单位数据为空，无法生成初始单位");
    }

    for (int i = 0; i < 2; ++i) {
        const QString key = keys[rand() % keys.size()];
        auto u = unitData->createUnit(key, Owner::PlayerCtrl, 1);
        if (u) {
            m_units.push_back(std::move(u));
        } else {
            qWarning() << "创建初始单位失败，key:" << key;
        }
    }

    // ━━ 首轮敌方单位生成 (battleIndex = 1) ━━
    generateEnemyUnits();
}

Unit* Game::findUnitById(int unitId) const
{
    for (const auto& uptr : m_units) {
        if (uptr && uptr->id() == unitId) {
            return uptr.get();
        }
    }
    return nullptr;
}

GridItem* Game::findGridItem(const QPoint& gridPos) const // 根据网格坐标查找对应的GridItem
{
    for (GridItem* item : m_gridItems) {
        if (item && item->gridPos() == gridPos) {
            return item;
        }
    }
    return nullptr;
}

UnitItem* Game::findUnitItem(int unitId) const // 根据单位ID查找对应的UnitItem，使用哈希表进行快速查找
{
    auto it = m_unitItemById.find(unitId); //没找到返回m_unitItemById.end()
    if (it == m_unitItemById.end()) {
        return nullptr;
    }
    return it->second; //取值
}

void Game::clearGridHighlights() // 清除所有格子的高亮状态
{
    for (GridItem* item : m_gridItems) {
        if (!item) {
            continue;
        }
        item->setHoverActive(false);
        item->setDropActive(false);
    }
}

bool Game::canApplyDrop(int unitId, const QPoint& source, const QPoint& target) const
{   
    if(m_phase != GamePhase::Preparation) {
        return false;
    }

    bool reachMaxFieldUnits = countFieldUnits(Owner::PlayerCtrl) >= m_player.maxFieldUnits();
    if (m_board.isBoardPosition(target) && m_dragStartPlace == DragStartPlace::Bench && reachMaxFieldUnits) {
        return false;
        
    }

    Unit* unit = findUnitById(unitId);
    if (!unit) {
        return false;
    }

    if (!m_board.isValidPosition(source) || !m_board.isValidPosition(target)) {
        return false;
    }

    if (!m_board.isPlayerHalf(source) || !m_board.isPlayerHalf(target)) {
        return false;
    }

    if (source == target || m_board.hasUnitAt(target)) {
        return false;
    }

    return m_board.getUnitAt(source) == unit;
}

void Game::applyDrop(int unitId, const QPoint& target)
{
    Unit* unit = findUnitById(unitId);
    if (!unit) {
        return;
    }

    m_board.removeUnit(unit);
    m_board.addUnit(unit, target);
}

void Game::buildScene() // 根据当前棋盘状态构建图形场景，创建GridItem和UnitItem，并设置它们的位置和层级关系
{   
    // 清除现有场景中的所有项和状态
    m_scene->clear();
    m_sellZone = nullptr;
    m_gridItems.clear();
    m_unitItems.clear();
    m_unitItemById.clear();

    QRectF totalBounds;
    bool first = true;
    for (int row = 0; row < m_board_rows; ++row) {
        for (int col = 0; col < m_board_cols; ++col) {
            const QPolygonF poly = cellHexPolygon(row, col);
            GridItem* gridItem = new GridItem(row, col, poly);
            gridItem->setZValue(kZGrid);
            gridItem->setBaseColor(row < m_board_rows / 2 ? QColor(80, 60, 60) : QColor(60, 60, 80));

            m_scene->addItem(gridItem); // 把格子项添加到场景中
            m_gridItems.push_back(gridItem);

            const QRectF bounds = gridItem->boundingRect();
            totalBounds = first ? bounds : totalBounds.united(bounds);
            first = false;
        }
    }

    for (int row = m_board_rows; row < m_board_rows + m_bench_rows; ++row) {
        for (int col = 0; col < m_bench_cols; ++col) {
            const QPolygonF poly = cellHexPolygon(row, col);
            GridItem* gridItem = new GridItem(row, col, poly);
            gridItem->setZValue(kZGrid);
            gridItem->setBaseColor(QColor(60, 80, 60));

            m_scene->addItem(gridItem); 
            m_gridItems.push_back(gridItem);

            const QRectF bounds = gridItem->boundingRect();
            totalBounds = totalBounds.united(bounds);
            first = false;
        }
    }    

    for (const auto& uptr : m_units) {
        UnitItem* unitItem = new UnitItem(uptr.get());
        unitItem->setZValue(kZUnit);
        m_scene->addItem(unitItem); // 把单位项添加到场景中
        m_unitItems.push_back(unitItem);
        m_unitItemById[uptr->id()] = unitItem;

        connect(unitItem, &UnitItem::dragStarted,
                this, &Game::handleDragStarted);
        connect(unitItem, &UnitItem::dragMoved,
                this, &Game::handleDragMoved);
        connect(unitItem, &UnitItem::dragDropped,
                this, &Game::handleDropCommand);
        connect(unitItem, &UnitItem::equipmentUnequipRequested,
                this, &Game::handleEquipUnequip);
    }
    m_scene->setSceneRect(m_scene->itemsBoundingRect().adjusted(-20, -20, 20, 20));
}

void Game::syncFromBoard() // 根据棋盘状态更新所有单位图形项的位置和可见性
{
    clearGridHighlights();

    for (UnitItem* item : m_unitItems) {
        if (!item || !item->unit()) {
            continue;
        }

        const QPoint pos = item->unit()->position();
        if (!m_board.isValidPosition(pos) || !item->unit()->isAlive() || m_board.getUnitAt(pos) != item->unit()) {
            item->setVisible(false);
            continue;
        }

        item->setVisible(true);
        item->setGridPos(pos);
        item->setPos(gridToWorld(pos.y(), pos.x()));
        item->setZValue(kZUnit);
        item->update(); // 强制重绘，确保每 tick 血量等信息刷新
    }
    emit stateUpdated(); // 通知 UI 更新状态变化
}

QPointF Game::gridToWorld(int row, int col) const // 将网格坐标转换为世界坐标
{   
    const qreal colSpacing = m_radius * qSqrt(3.0); // 列间距，等于半径乘以sqrt(3)，这是六边形网格的标准水平间距
    const qreal xOffset = (row % 2 == 0) ? colSpacing * 0.5 : 0.0; // 偶数行需要水平偏移半个列间距，以实现交错排列
    qreal boardCenterX = (m_board_cols - 0.5) * colSpacing / 2.0; // 计算棋盘中心的X坐标，考虑到最后一列的偏移
    qreal benchCenterX = (m_bench_cols - 0.5) * colSpacing / 2.0; // 计算备战区中心的X坐标，考虑到最后一列的偏移
    qreal benchStartX = boardCenterX - benchCenterX; // 计算备战区的起始X坐标，使其相对于棋盘居中
    if (row >= m_board_rows) { // 如果行号超过棋盘行数，说明在备战区，需要加上备战区的起始X坐标
        const qreal x = benchStartX + xOffset + col * colSpacing;
        const qreal y = row * m_rowSpacing + m_BBSpacing;
        return QPointF(x, y);
    }
    const qreal x = xOffset + col * colSpacing;
    const qreal y = row * m_rowSpacing;
    return QPointF(x, y);
}

QPoint Game::worldToGrid(const QPointF& world) const // 将鼠标坐标转换为网格坐标
{
    QPoint best(-1, -1);
    qreal bestDist = 1e18;

    for (int row = 0; row < m_board_rows; ++row) {
        for (int col = 0; col < m_board_cols; ++col) {
            const QPointF center = gridToWorld(row, col);
            const qreal dx = world.x() - center.x();
            const qreal dy = world.y() - center.y();
            const qreal d2 = dx * dx + dy * dy;
            if (d2 < bestDist) {
                bestDist = d2;
                best = QPoint(col, row);
            }
        }
    }
    for (int row = m_board_rows; row < m_board_rows + m_bench_rows; ++row) {
        for (int col = 0; col < m_bench_cols; ++col) {
            const QPointF center = gridToWorld(row, col);
            const qreal dx = world.x() - center.x();
            const qreal dy = world.y() - center.y();
            const qreal d2 = dx * dx + dy * dy;
            if (d2 < bestDist) {
                bestDist = d2;
                best = QPoint(col, row);
            }
        }
    }
    // 可能待添加的逻辑：在棋盘和备战区之间的间距中不被认为是选中了某个格子
    return best;
}

QPolygonF Game::cellHexPolygon(int row, int col) const // 计算指定网格单元的六边形多边形，用于绘制网格单元的形状
{
    const QPointF center = gridToWorld(row, col);
    QPolygonF poly;
    poly.reserve(6);

    for (int i = 0; i < 6; ++i) {
        const qreal angleDeg = 60.0 * i - 90.0;
        const qreal angleRad = qDegreesToRadians(angleDeg);
        poly.append(QPointF(
            center.x() + m_radius * qCos(angleRad),
            center.y() + m_radius * qSin(angleRad)
        ));
    }

    return poly;
}

void Game::startBattle() {
    if (m_phase != GamePhase::Preparation) return;
    m_phase = GamePhase::Battle;
    emit phaseChanged(m_phase);
    m_battleIndex ++;
    onBattleStart();
}

void Game::startSettlement() {
    if (m_phase != GamePhase::Battle) return;
    m_phase = GamePhase::Settlement;
    emit phaseChanged(m_phase);
    onSettlementStart();
}

void Game::startPreparation() {
    if (m_phase != GamePhase::Settlement) return;
    m_phase = GamePhase::Preparation;
    emit phaseChanged(m_phase);
    onPreparationStart();
    emit stateUpdated(); // 通知 UI 更新状态变化
}

void Game::onBattleStart() {
    if(!m_battleSystem){
        m_battleSystem = new BattleSystem(this);
        connect(m_battleSystem, &BattleSystem::stateUpdated,
                this, &Game::syncFromBoard);           // 更新GUI
        connect(m_battleSystem, &BattleSystem::battleFinished,
                this, &Game::onBattleFinished);       // 战斗结束处理
    }
    // 只让棋盘上的单位参战，备战席单位不参与寻路和羁绊计算
    m_battleUnits.clear();
    for (const auto& uptr : m_units) {
        if (uptr->isAlive() && m_board.isBoardPosition(uptr->position())) {
            m_battleUnits.append(uptr.get());
        }
    }
    // 开战前锁定羁绊加成
    recalculateSynergies();

    // 保存玩家单位快照，用于战斗结束后恢复（敌方每回合重新生成，无需快照）
    m_unitsSnapshot.clear();
    for (const auto& uptr : m_units) {
        if (uptr->owner() == Owner::PlayerCtrl)
            m_unitsSnapshot.push_back(uptr->clone());
    }

    // 启动战斗
    m_battleSystem->start(m_board, m_battleUnits, &m_player, &m_enemy);
}

void Game::onBattleFinished(BattleResult result) {
    m_battleSystem->stop();
    // 记录结算信息，不改变状态
    m_settlementInfo.result = result; // 记录战斗结果，供结算界面显示
    if(result == BattleResult::Draw){
        // 平局处理
        m_settlementInfo.playerHpBefore = m_player.hp();
        m_settlementInfo.playerHpAfter = m_player.hp() - 10;
        m_settlementInfo.playerGoldBefore = m_player.gold();
        m_settlementInfo.playerGoldAfter = m_player.gold() + m_battleIndex * 2;
        m_settlementInfo.enemyHpBefore = m_enemy.hp();
        m_settlementInfo.enemyHpAfter = m_enemy.hp() - 10;
        m_settlementInfo.enemyGoldBefore = m_enemy.gold();
        m_settlementInfo.enemyGoldAfter = m_enemy.gold() + m_battleIndex * 2;
        m_settlementInfo.isGameOver = (m_settlementInfo.playerHpAfter <= 0 || m_settlementInfo.enemyHpAfter <= 0);
    }
    else if(result == BattleResult::PlayerWin) {
        // 处理winner和loser
        m_settlementInfo.playerHpBefore = m_player.hp();
        m_settlementInfo.playerHpAfter = m_player.hp();
        m_settlementInfo.playerGoldBefore = m_player.gold();
        m_settlementInfo.playerGoldAfter = m_player.gold() + m_battleIndex * 5;
        m_settlementInfo.enemyHpBefore = m_enemy.hp();
        m_settlementInfo.enemyHpAfter = m_enemy.hp() - 10 - m_battleIndex * 5;
        m_settlementInfo.enemyGoldBefore = m_enemy.gold();
        m_settlementInfo.enemyGoldAfter = m_enemy.gold() + m_battleIndex;
        m_settlementInfo.isGameOver = (m_settlementInfo.enemyHpAfter <= 0);
    }
    else if(result == BattleResult::EnemyWin) { 
        m_settlementInfo.playerHpBefore = m_player.hp();
        m_settlementInfo.playerHpAfter = m_player.hp() - 10 - m_battleIndex * 5;
        m_settlementInfo.playerGoldBefore = m_player.gold();
        m_settlementInfo.playerGoldAfter = m_player.gold() + m_battleIndex;
        m_settlementInfo.enemyHpBefore = m_enemy.hp();
        m_settlementInfo.enemyHpAfter = m_enemy.hp();
        m_settlementInfo.enemyGoldBefore = m_enemy.gold();
        m_settlementInfo.enemyGoldAfter = m_enemy.gold() + m_battleIndex * 5;
        m_settlementInfo.isGameOver = (m_settlementInfo.playerHpAfter <= 0);
    }
    // 掉落判定，敌方每个死亡单位按星级独立计算，上限 3 件
    m_settlementInfo.droppedEquipmentKeys.clear();
    {
        constexpr int kMaxDrops = 3;
        int dropCount = 0;
        auto& reg = *EquipmentRegistry::instance();
        const auto keys = reg.allKeys();
        for (Unit* unit : m_battleUnits) {
            if (dropCount >= kMaxDrops) break;
            if (!unit || unit->owner() != Owner::EnemyCtrl) continue;
            if (unit->isAlive()) continue;

            double dropChance = 0.0;
            switch (unit->starLevel()) {
            case 1: dropChance = 0.08; break; // 1星 8%
            case 2: dropChance = 0.25; break; // 2星 25%
            case 3: dropChance = 0.55; break; // 3星 55%
            default: dropChance = 0.08; break;
            }

            if (!keys.isEmpty() && (rand() % 1000) < static_cast<int>(dropChance * 1000)) {
                m_settlementInfo.droppedEquipmentKeys.append(keys[rand() % keys.size()]);
                ++dropCount;
                qDebug() << "掉落装备 (" << unit->starLevel() << "星):"
                         << m_settlementInfo.droppedEquipmentKeys.last();
            }
        }
    }
    startSettlement(); // 进入结算阶段
    
}

void Game::onSettlementStart() {
    // 待填充
    m_player.setHp(m_settlementInfo.playerHpAfter);
    m_player.setGold(m_settlementInfo.playerGoldAfter);
    m_enemy.setHp(m_settlementInfo.enemyHpAfter);
    m_enemy.setGold(m_settlementInfo.enemyGoldAfter);
    emit settlementReady(m_settlementInfo); // 通知 UI 结算信息准备好，可以显示结算界面
    emit stateUpdated();
}

void Game::onPreparationStart() {
    // 移除旧的 UnitItem
    for (UnitItem* item : m_unitItems) {
        if (item) {
            m_scene->removeItem(item);
            delete item;
        }
    }
    m_unitItems.clear();
    m_unitItemById.clear();

    m_board.clear();
    m_units.clear();

    // 从快照恢复玩家单位 (快照只含玩家单位)
    for (const auto& uptr : m_unitsSnapshot) {
        auto clone = uptr->clone();
        const QPoint pos = clone->position();
        if (m_board.isValidPosition(pos)) {
            m_board.addUnit(clone.get(), pos);
        }
        m_units.push_back(std::move(clone));
    }

    // 生成新一轮敌方单位
    generateEnemyUnits();

    // 为所有单位创建新的 UnitItem
    for (const auto& uptr : m_units) {
        UnitItem* unitItem = new UnitItem(uptr.get());
        unitItem->setZValue(kZUnit);
        m_scene->addItem(unitItem);
        m_unitItems.push_back(unitItem);
        m_unitItemById[uptr->id()] = unitItem;

        connect(unitItem, &UnitItem::dragStarted,
                this, &Game::handleDragStarted);
        connect(unitItem, &UnitItem::dragMoved,
                this, &Game::handleDragMoved);
        connect(unitItem, &UnitItem::dragDropped,
                this, &Game::handleDropCommand);
        connect(unitItem, &UnitItem::equipmentUnequipRequested,
                this, &Game::handleEquipUnequip);
    }

    recalculateSynergies();
    syncFromBoard();
}

void Game::generateEnemyUnits() {
    const int nextBattleIndex = m_battleIndex + 1;
    QVector<EnemySpawnPlan> plans = EnemySpawner::instance()->generateEnemy(nextBattleIndex, m_board);

    for (const auto& plan : plans) {
        auto u = UnitData::instance()->createUnit(plan.unitKey, Owner::EnemyCtrl, plan.starLevel);
        if (!u) continue;
        u->setPosition(plan.position);
        m_board.addUnit(u.get(), plan.position);
        m_units.push_back(std::move(u));
    }
}


int Game::countFieldUnits(Owner owner) const {
    int count = 0;
    for (int row = 0; row < m_board_rows; ++row) {
        for (int col = 0; col < m_board_cols; ++col) {
            Unit* unit = m_board.getUnitAt(QPoint(col, row));
            if (unit && unit->owner() == owner) {
                count++;
            }
        }
    }
    return count;
}

QHash<QString, int> Game::getTraitCounts(Owner owner) const {
    QHash<QString, int> traitCounts;
    for (int row = 0; row < m_board_rows; ++row) {
        for (int col = 0; col < m_board_cols; ++col) {
            Unit* unit = m_board.getUnitAt(QPoint(col, row));
            if (unit && unit->owner() == owner) {
                for (const QString& trait : unit->traits()) {
                    traitCounts[trait]++;
                }
            }
        }
    }
    return traitCounts;
}

void Game::recalculateSynergies() {

    for (auto& uptr : m_units) {
        if (uptr) {
            uptr->clearSynergyBonuses();
        }
    }

    for (int ownerIdx = 0; ownerIdx < 2; ++ownerIdx) {
        const Owner owner = (ownerIdx == 0) ? Owner::PlayerCtrl : Owner::EnemyCtrl;

        // 统计该所有者在场上的 trait 数量
        const QHash<QString, int> traitCounts = getTraitCounts(owner);

        // 查表：每种 trait 在当前数量下提供的加成
        QHash<QString, SynergyBonus> traitBonuses;
        for (auto it = traitCounts.begin(); it != traitCounts.end(); ++it) {
            traitBonuses[it.key()] = SynergyRegistry::instance()->getBonus(it.key(), it.value());
        }

        // 应用加成：遍历该所有者的单位，根据它们的 trait 应用对应的加成
        for (int row = 0; row < m_board_rows; ++row) {
            for (int col = 0; col < m_board_cols; ++col) {
                Unit* unit = m_board.getUnitAt(QPoint(col, row));
                if (!unit || unit->owner() != owner) {
                    continue;
                }

                SynergyBonus totalBonus;
                for (const QString& trait : unit->traits()) {
                    if (traitBonuses.contains(trait)) {
                        totalBonus += traitBonuses[trait];
                    }
                }
                unit->applySynergyBonuses(totalBonus);
            }
        }
    }
}

void Game::buyXp(int amount) {
    if (m_phase != GamePhase::Preparation || m_player.gold() < amount) return;
    m_player.addXp(amount);
    m_player.spendGold(amount); // 1:1兑换
    emit stateUpdated(); // 通知 UI 更新状态变化
}

void Game::populateStore() {
    UnitData* data = UnitData::instance();
    QStringList keys = data->allKeys();
    if (keys.isEmpty()) return;

    for (int i = 0; i < Store::STORE_SIZE; ++i) {
        QString key = keys[rand() % keys.size()];
        int star = (rand() % 3) + 1;  // 1~3 星，后续根据 m_store.level() 调整概率
        auto u = data->createUnit(key, Owner::PlayerCtrl, star);
        if (u) {
            m_store.addUnit(std::move(u), i);
        }
    }
}

void Game::buildStoreScene() {
    // 初始化随机单位
    populateStore();

    // Store 自己管理显示
    m_store.buildDisplay(m_scene, m_radius);

    // “出售”
    m_sellZone = new SellZoneItem(QPointF(-150, 580), 100, 100);
    m_scene->addItem(m_sellZone);

    // Game 只连信号
    for (int i = 0; i < Store::STORE_SIZE; ++i) {
        connect(m_store.slotItem(i), &StoreSlotItem::clicked,
                this, &Game::onStoreSlotClicked);
    }
    connect(m_store.refreshButton(), &StoreRefreshButton::clicked,
            this, &Game::refreshStore);

    // 更新 scene rect
    m_scene->setSceneRect(m_scene->itemsBoundingRect().adjusted(-20, -20, 15, 5)); // 左上右下
}
void Game::onStoreSlotClicked(int index) {
    if (m_phase != GamePhase::Preparation) return;
    if (index < 0 || index >= Store::STORE_SIZE) return;
    Unit* unit = m_store.getUnitAt(index);
    if (!unit) return; // 没有单位

    int price = unit->price();
    if (!m_player.canAfford(price)) return; // 钱不够

    QPoint benchPos = findEmptyBenchSlot();
    if (benchPos == QPoint(-1, -1)) return; // 没有空位

    auto boughtUnitPtr = m_store.buyUnit(index);
    if (!boughtUnitPtr) return; // 购买失败
    Unit* boughtUnit = boughtUnitPtr.get();

    m_player.spendGold(price); // 扣钱

    m_board.addUnit(boughtUnit, benchPos); // 放到备战区
    m_units.push_back(std::move(boughtUnitPtr));
    UnitItem* item = new UnitItem(boughtUnit);
    item->setZValue(kZUnit);
    m_scene->addItem(item);
    m_unitItems.push_back(item);
    m_unitItemById[boughtUnit->id()] = item;

    connect(item, &UnitItem::dragStarted, this, &Game::handleDragStarted);
    connect(item, &UnitItem::dragMoved,  this, &Game::handleDragMoved);
    connect(item, &UnitItem::dragDropped,this, &Game::handleDropCommand);
    connect(item, &UnitItem::equipmentUnequipRequested,
            this, &Game::handleEquipUnequip);

    tryMergeStar(boughtUnit); // 购买后尝试升星合并

    recalculateSynergies();
    syncFromBoard(); // 更新界面
    emit stateUpdated(); // 通知 UI 更新状态变化
}

QPoint Game::findEmptyBenchSlot() const {
    for (int row = m_board_rows; row < m_board_rows + m_bench_rows; ++row) {
        for (int col = 0; col < m_bench_cols; ++col) {
            if (!m_board.hasUnitAt(QPoint(col, row))) {
                return QPoint(col, row);
            }
        }
    }
    return QPoint(-1, -1); // 没有空位
}

void Game::refreshStore() {
    if (m_phase != GamePhase::Preparation) return;

    // 费用
    constexpr int kRefreshCost = 2;
    if (!m_player.canAfford(kRefreshCost)) return;

    // 清空旧单位，随机生成 5 个新单位
    m_store.refresh();
    populateStore();

    // 更新显示
    m_store.refreshDisplay();
    m_player.spendGold(kRefreshCost);
    emit stateUpdated();
}

bool Game::isOverSellZone(const QPointF& scenePos) const {
    if (!m_sellZone) return false;
    return m_sellZone->sceneBoundingRect().contains(scenePos);
}

void Game::sellUnit(int unitId) {
    Unit* unit = findUnitById(unitId);
    if (!unit) return;
    if (unit->owner() != Owner::PlayerCtrl) return;

    int refund = unit->price();  // 返还原价

    // 出售前将单位身上的装备返还装备栏
    if (m_equipBar) {
        std::vector<std::shared_ptr<Equipment>> overflowEqs; // 暂存溢出的装备
        while (unit->equipmentCount() > 0) {
            auto eq = unit->unequip(0);
            if (!eq) continue;
            if (!m_equipBar->addEquipment(eq)) {
                overflowEqs.push_back(eq); // 栏满暂存
            }
        }
        if (!overflowEqs.empty()) {
            m_pendingOverflowEquipments = std::move(overflowEqs);
            m_pendingOverflowContext = LootDialog::LootContext::Unequip;
            m_pendingOverflowUnitId = -1; // 单位已售出，无需回滚
            emit equipBarOverflow(); // 通知 UI 显示溢出装备对话框
        }
    }

    m_board.removeUnit(unit);

    auto it = std::find_if(m_units.begin(), m_units.end(),
        [unitId](const auto& u) { return u && u->id() == unitId; });
    if (it != m_units.end()) {
        m_units.erase(it);
    }

    if (auto item = findUnitItem(unitId)) {
        m_scene->removeItem(item);
        auto itemIt = std::find(m_unitItems.begin(), m_unitItems.end(), item);
        if (itemIt != m_unitItems.end()) m_unitItems.erase(itemIt);
        m_unitItemById.erase(unitId);
        delete item;
    }

    m_player.addGold(refund);
}

void Game::tryMergeStar(Unit* newUnit)
{
    if (!newUnit) return;
    if (m_phase != GamePhase::Preparation) return;
    if (newUnit->owner() != Owner::PlayerCtrl) return;
    if (newUnit->starLevel() >= 3) return; // 已达上限

    const QString key = newUnit->key();
    const int star = newUnit->starLevel();

    QList<Unit*> candidates;
    for (const auto& uptr : m_units) {
        if (uptr->owner() == Owner::PlayerCtrl
            && uptr->key() == key
            && uptr->starLevel() == star
            && uptr->isAlive()) {
            candidates.append(uptr.get());
        }
    }

    if (candidates.size() < 3) return;

    // 优先保留 newUnit，其余任选2个消耗
    Unit* keep = candidates.contains(newUnit) ? newUnit : candidates[0];

    QList<Unit*> consume;
    for (Unit* c : candidates) {
        if (c != keep && consume.size() < 2) {
            consume.append(c);
        }
    }

    // 销毁被消耗的单位
    for (Unit* u : consume) {
        m_board.removeUnit(u);

        auto it = std::find_if(m_units.begin(), m_units.end(),
            [u](const auto& ptr) { return ptr.get() == u; });
        if (it != m_units.end()) {
            m_units.erase(it);
        }

        if (auto* item = findUnitItem(u->id())) {
            m_scene->removeItem(item);
            auto itemIt = std::find(m_unitItems.begin(), m_unitItems.end(), item);
            if (itemIt != m_unitItems.end()) m_unitItems.erase(itemIt);
            m_unitItemById.erase(u->id());
            delete item;
        }
    }

    // 升级保留的单位
    keep->upgradeStar();

    recalculateSynergies();
    syncFromBoard();

    // 递归检查连锁升星
    tryMergeStar(keep);
}

// 装备栏
void Game::buildEquipBar()
{
    const qreal colSpacing = m_radius * qSqrt(3.0);
    const qreal boardCenterX = (m_board_cols - 0.5) * colSpacing / 2.0;
    const qreal benchBottomY = (m_board_rows + m_bench_rows) * m_rowSpacing + m_BBSpacing;

    constexpr int kEquipSlots = 8;
    constexpr double kSlotSize = 40.0;
    constexpr double kSlotSpacing = 8.0;
    const double totalW = kEquipSlots * kSlotSize + (kEquipSlots - 1) * kSlotSpacing;
    const QPointF topLeft(boardCenterX - totalW / 2.0, benchBottomY + 8.0);

    m_equipBar = new EquipBar();
    m_equipBar->buildBar(m_scene, topLeft, kEquipSlots, kSlotSize, kSlotSpacing);

    // 测试装备
    auto& reg = *EquipmentRegistry::instance();
    m_equipBar->addEquipment(reg.createEquipment("bf_sword"));
    m_equipBar->addEquipment(reg.createEquipment("giant_belt"));
    m_equipBar->addEquipment(reg.createEquipment("tear"));
    m_equipBar->addEquipment(reg.createEquipment("recurve_bow"));
    m_equipBar->addEquipment(reg.createEquipment("bf_sword"));
    m_equipBar->addEquipment(reg.createEquipment("giant_belt"));
    m_equipBar->addEquipment(reg.createEquipment("tear"));
    m_equipBar->addEquipment(reg.createEquipment("recurve_bow"));

    // 连接拖拽信号
    for (int i = 0; i < kEquipSlots; ++i) {
        EquipSlotItem* slot = m_equipBar->slot(i);
        if (!slot) continue;
        connect(slot, &EquipSlotItem::equipDragStarted,
                this, &Game::handleEquipDragStarted);
        connect(slot, &EquipSlotItem::equipDragMoved,
                this, &Game::handleEquipDragMoved);
        connect(slot, &EquipSlotItem::equipDragDropped,
                this, &Game::handleEquipDropCommand);
    }

    // 更新 scene rect 以包含装备栏
    m_scene->setSceneRect(m_scene->itemsBoundingRect().adjusted(-20, -20, 15, 5));
}

UnitItem* Game::findUnitItemAtScenePos(const QPointF& scenePos) const
{
    for (UnitItem* item : m_unitItems) {
        if (!item || !item->isVisible()) continue;
        if (item->sceneBoundingRect().contains(scenePos))
            return item;
    }
    return nullptr;
}


void Game::handleEquipDragStarted(int slotIndex, const QPointF& scenePos)
{
    if (m_phase != GamePhase::Preparation) return;

    EquipSlotItem* slot = m_equipBar ? m_equipBar->slot(slotIndex) : nullptr;
    if (!slot || slot->isEmpty()) return;

    m_equipDragActive = true;
    m_activeEquipSlotIndex = slotIndex;
    slot->setDraggingOut(true);

    // 创建拖拽幽灵图标
    auto eq = slot->equipment();
    QPixmap ghostPix(36, 36);
    ghostPix.fill(Qt::transparent);
    {
        QPainter p(&ghostPix);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(QPen(QColor(180, 180, 200), 2.0));
        p.setBrush(QColor(80, 80, 100, 180));
        p.drawRoundedRect(QRectF(2, 2, 32, 32), 5, 5);
        if (eq && !eq->name.isEmpty()) {
            p.setPen(Qt::white);
            QFont f = p.font();
            f.setPointSize(12);
            f.setBold(true);
            p.setFont(f);
            const QString txt = eq->name.left(1);
            p.drawText(QRectF(2, 2, 32, 32), static_cast<int>(Qt::AlignCenter), txt);
        }
    }

    m_equipDragGhost = new QGraphicsPixmapItem(ghostPix);
    m_equipDragGhost->setZValue(kZDraggingUnit);
    m_equipDragGhost->setPos(scenePos - QPointF(18, 18));
    m_scene->addItem(m_equipDragGhost);
}

void Game::handleEquipDragMoved(int slotIndex, const QPointF& scenePos)
{
    if (!m_equipDragActive) return;

    // 移动幽灵图标
    if (m_equipDragGhost) {
        m_equipDragGhost->setPos(scenePos - QPointF(18, 18));
    }
}

void Game::handleEquipDropCommand(int slotIndex, const QPointF& scenePos)
{
    if (!m_equipDragActive) return;

    // 清理幽灵图标
    if (m_equipDragGhost) {
        m_scene->removeItem(m_equipDragGhost);
        delete m_equipDragGhost;
        m_equipDragGhost = nullptr;
    }

    EquipSlotItem* sourceSlot = m_equipBar ? m_equipBar->slot(m_activeEquipSlotIndex) : nullptr;
    if (sourceSlot) {
        sourceSlot->setDraggingOut(false);
    }

    UnitItem* targetUnitItem = findUnitItemAtScenePos(scenePos);
    if (targetUnitItem && targetUnitItem->unit()
        && targetUnitItem->unit()->owner() == Owner::PlayerCtrl
        && targetUnitItem->unit()->canEquip()
        && sourceSlot && !sourceSlot->isEmpty())
    {
        // 执行装备
        auto eq = m_equipBar->removeEquipment(m_activeEquipSlotIndex);
        if (eq) {
            targetUnitItem->unit()->equip(eq);
        }
    }

    m_equipDragActive = false;
    m_activeEquipSlotIndex = -1;

    recalculateSynergies();  // 更新属性加成
    syncFromBoard();
    emit stateUpdated();
}

void Game::handleEquipUnequip(int unitId, int equipIndex)
{
    if (m_phase != GamePhase::Preparation) return;

    Unit* unit = findUnitById(unitId);
    if (!unit || unit->owner() != Owner::PlayerCtrl) return;

    auto eq = unit->unequip(equipIndex);
    if (!eq || !m_equipBar) return;

    // 先尝试直接放回空位
    if (m_equipBar->addEquipment(eq)) {
        recalculateSynergies();
        syncFromBoard();
        return;
    }

    // 装备栏满 → 弹出 LootDialog 处理
    m_pendingOverflowEquipments.clear();
    m_pendingOverflowEquipments.push_back(eq);
    m_pendingOverflowContext = LootDialog::LootContext::Unequip;
    m_pendingOverflowUnitId = unitId;
    emit equipBarOverflow();
    // 先不调用 recalculateSynergies/syncFromBoard，等LootDialog处理完毕后由resolveEquipOverflow统一刷新
}

void Game::resolveEquipOverflow(const LootDialog::LootResult& result)
{
    if (result.cancelled) {
        // 用户点 × 关闭 → 回滚：把暂存装备放回原单位
        if (m_pendingOverflowUnitId >= 0) {
            Unit* unit = findUnitById(m_pendingOverflowUnitId);
            if (unit) {
                for (auto& eq : m_pendingOverflowEquipments) {
                    if (eq) unit->equip(eq);
                }
            }
        }
        // 单位已售出（unitId == -1）且用户取消 → 装备丢了就丢了
    } else {
        // 先丢弃选中的装备栏槽位
        // 从高 index 向低 index 删除，避免索引偏移
        std::vector<int> sortedDiscards(result.discardBarIndices.begin(),
                                         result.discardBarIndices.end());
        std::sort(sortedDiscards.begin(), sortedDiscards.end(), std::greater<int>());
        for (int idx : sortedDiscards) {
            m_equipBar->removeEquipment(idx);
        }

        // 把未标记丢弃的新装备放入空位
        for (size_t i = 0; i < m_pendingOverflowEquipments.size(); ++i) {
            if (result.discardNewIndices.count(static_cast<int>(i)) > 0) continue;
            m_equipBar->addEquipment(m_pendingOverflowEquipments[i]);
        }
    }

    m_pendingOverflowEquipments.clear();
    m_pendingOverflowUnitId = -1;
    m_equipBar->refreshDisplay();
    recalculateSynergies();
    syncFromBoard();
}
