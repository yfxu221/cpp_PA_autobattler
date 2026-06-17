#include "store.h"
#include "gui/storerefreshbutton.h"

Store::Store()
    : m_cells(STORE_SIZE)  // unique_ptr 默认初始化为 nullptr
    , m_level(1)
{}

void Store::addUnit(std::unique_ptr<Unit> unit, int index) {
    if (!unit || index < 0 || index >= STORE_SIZE || m_cells[index]) {
        return;
    }

    Unit* raw = unit.get();
    m_cells[index] = std::move(unit);
    m_unitToIndex[raw] = index;
}

void Store::removeUnit(int index) {
    if (index < 0 || index >= STORE_SIZE || !m_cells[index]) {
        return;
    }

    m_unitToIndex.remove(m_cells[index].get());
    m_cells[index].reset();
}

void Store::refresh() {
    // 先清 hash（避免 reset 后裸指针悬空），再销毁所有 Unit
    m_unitToIndex.clear();
    for (auto& cell : m_cells) {
        cell.reset();
    }
}

std::unique_ptr<Unit> Store::buyUnit(int index) {
    if (index < 0 || index >= STORE_SIZE || !m_cells[index]) {
        return nullptr;
    }

    m_unitToIndex.remove(m_cells[index].get());

    // 更新 UnitItem 显示状态
    if (StoreSlotItem* item = slotItem(index)) {
        item->setUnit(nullptr);
    }

    return std::move(m_cells[index]);  // 移出所有权，m_cells[index] 变为 nullptr
}

bool Store::hasUnitAt(int index) const {
    if (index < 0 || index >= STORE_SIZE) {
        return false;
    }
    return m_cells[index] != nullptr;
}

Unit* Store::getUnitAt(int index) const {
    if (index < 0 || index >= STORE_SIZE) {
        return nullptr;
    }
    return m_cells[index].get();
}

void Store::buildDisplay(QGraphicsScene* scene, double radius) {
    // 清除旧 GUI 指针（场景重建时旧 QGraphicsItems 已被 Qt 销毁）
    m_slotItems.clear();
    m_refreshButton = nullptr;

    const double storeX = -150.0;
    const double startY = 35.0;
    const double slotSpacing = 92.0;

    for (int i = 0; i < STORE_SIZE; ++i) {
        QPointF center(storeX, startY + i * slotSpacing);
        auto* item = new StoreSlotItem(i, center, radius);
        scene->addItem(item);
        m_slotItems.append(item);

        // 初始显示
        if (m_cells[i]) {
            item->setUnit(m_cells[i].get());
        }
    }

    // 刷新按钮
    const double btnW = 90.0, btnH = 32.0;
    const double btnCenterY = startY + (STORE_SIZE - 1) * slotSpacing
                              + radius + btnH / 2.0 + 8.0;
    m_refreshButton = new StoreRefreshButton(QPointF(storeX, btnCenterY), btnW, btnH);
    scene->addItem(m_refreshButton);
}

StoreSlotItem* Store::slotItem(int index) const {
    if (index < 0 || index >= STORE_SIZE) {
        return nullptr;
    }
    return m_slotItems[index];
}

void Store::refreshDisplay() {
    for (int i = 0; i < STORE_SIZE; ++i) {
        if (m_slotItems[i]) {
            m_slotItems[i]->setUnit(m_cells[i].get());
        }
    }
}