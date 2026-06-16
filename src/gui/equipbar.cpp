#include "gui/equipbar.h"
#include "gui/equipslotitem.h"

#include <QDebug>

EquipBar::~EquipBar()
{
    clear();
}

void EquipBar::buildBar(QGraphicsScene* scene, const QPointF& topLeft,
                         int slotCount, double slotSize, double spacing)
{
    if (!scene) return;

    // 清理旧内容
    clear();
    m_scene = scene;

    for (int i = 0; i < slotCount; ++i) {
        const QPointF pos(topLeft.x() + i * (slotSize + spacing), topLeft.y());
        auto* slotItem = new EquipSlotItem(i, pos, slotSize);
        scene->addItem(slotItem);
        m_slots.push_back(slotItem);
    }
}

EquipSlotItem* EquipBar::slot(int index) const
{
    if (index < 0 || index >= static_cast<int>(m_slots.size()))
        return nullptr;
    return m_slots[index];
}

int EquipBar::findEmptySlot() const
{
    for (int i = 0; i < static_cast<int>(m_slots.size()); ++i) {
        if (m_slots[i] && m_slots[i]->isEmpty())
            return i;
    }
    return -1;
}

bool EquipBar::addEquipment(std::shared_ptr<Equipment> eq)
{
    int idx = findEmptySlot();
    if (idx < 0 || !eq) return false;

    m_slots[idx]->setEquipment(std::move(eq));
    return true;
}

bool EquipBar::setEquipment(int index, std::shared_ptr<Equipment> eq)
{
    auto* s = slot(index);
    if (!s) return false;

    s->setEquipment(std::move(eq));
    return true;
}

std::shared_ptr<Equipment> EquipBar::removeEquipment(int index)
{
    auto* s = slot(index);
    if (!s) return nullptr;

    auto eq = s->equipment();
    s->setEquipment(nullptr);
    return eq;
}

void EquipBar::refreshDisplay()
{
    for (auto* s : m_slots) {
        if (s) s->update();
    }
}

void EquipBar::clear()
{
    for (auto* s : m_slots) {
        if (!s) continue;
        if (m_scene) m_scene->removeItem(s);
        delete s;
    }
    m_slots.clear();
}

std::vector<std::shared_ptr<Equipment>> EquipBar::allEquipments() const
{
    std::vector<std::shared_ptr<Equipment>> result;
    result.reserve(m_slots.size());
    for (const auto* s : m_slots) {
        if (s) {
            result.push_back(s->equipment());
        } else {
            result.push_back(nullptr);
        }
    }
    return result;
}
