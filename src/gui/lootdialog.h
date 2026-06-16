#ifndef GUI_LOOTDIALOG_H
#define GUI_LOOTDIALOG_H

#include <QDialog>
#include <QString>
#include <QWidget>
#include <set>
#include <vector>
#include <memory>
#include <functional>
#include "entity/equipment.h"
#include <QLabel>

// 可点击的装备图标
class EquipIconWidget : public QWidget
{
public:
    using ClickCallback = std::function<void(int id)>;

    EquipIconWidget(std::shared_ptr<Equipment> eq, int id, double size,
                    ClickCallback cb, QWidget* parent = nullptr);

    void setSelected(bool sel);
    bool isSelected() const { return m_selected; }
    int id() const { return m_id; }
    bool isEmpty() const { return m_equipment == nullptr; }

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;

private:
    void ensureSpriteLoaded() const;

    std::shared_ptr<Equipment> m_equipment;
    int m_id;
    double m_size;
    double m_radius;
    bool m_selected = false;
    ClickCallback m_callback;

    mutable QPixmap m_sprite;
    mutable bool m_spriteTried = false;

    // 颜色
    static constexpr QColor kEmptyColor   = QColor(50, 50, 55);
    static constexpr QColor kFilledColor  = QColor(65, 65, 75);
    static constexpr QColor kBorderColor  = QColor(120, 120, 140);
    static constexpr QColor kSelectBorder = QColor(255, 60, 60);
    static constexpr QColor kNewItemBorder = QColor(255, 170, 0);
};

// 战利品弹窗：战斗结算 / 卸装备时装备栏满后选择丢弃
class LootDialog : public QDialog
{
    Q_OBJECT

public:
    // 弹窗场景
    enum class LootContext {
        Settlement, // 战斗结算 → 掉落的装备
        Unequip // 卸装备/出售 → 卸下的装备
    };

    // 用户选择结果
    struct LootResult {
        std::set<int> discardBarIndices; // 要丢弃的装备栏槽位索引
        std::set<int> discardNewIndices; // 要丢弃的新物品索引
        bool cancelled = false; // 用户点了 ×
    };

    // barEquipments: 当前装备栏所有槽位快照（nullptr 表示空槽位）
    // context: 场景（决定标题文案）
    explicit LootDialog(const std::vector<std::shared_ptr<Equipment>>& newItems,
                        const std::vector<std::shared_ptr<Equipment>>& barEquipments,
                        LootContext context,
                        QWidget* parent = nullptr);

    LootResult result() const { return m_result; }

protected:
    void reject() override; // 处理 × 关闭

private slots:
    void onIconClicked(int id);
    void onConfirm();

private:
    void buildUI();
    void updateStatusLabel();

    std::vector<std::shared_ptr<Equipment>> m_newItems; // 新装备
    std::vector<std::shared_ptr<Equipment>> m_barEquipments; // 装备栏快照
    LootContext m_context;
    LootResult m_result;

    // 图标控件
    std::vector<EquipIconWidget*> m_newIcons; // 上方：新装备
    std::vector<EquipIconWidget*> m_barIcons; // 下方：装备栏现有

    QPushButton* m_confirmButton = nullptr;
    QLabel* m_statusLabel = nullptr;

    // id 编码：0..N-1 = 新装备, N..N+M-1 = 装备栏槽位
    int m_newCount = 0;
    int m_barCount = 0;
    int idToBarIndex(int id) const { return id - m_newCount; } // 装备栏槽位 id 转索引
    bool isNewItemId(int id) const { return id < m_newCount; } // 是否新装备 id
};

#endif // GUI_LOOTDIALOG_H
