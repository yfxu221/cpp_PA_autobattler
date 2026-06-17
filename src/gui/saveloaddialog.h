#ifndef GUI_SAVELOADDIALOG_H
#define GUI_SAVELOADDIALOG_H

#include <QDialog>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QEnterEvent>
#include <QVBoxLayout>
#include <QGridLayout>
#include <vector>
#include "core/savemanager.h"

class SaveSlotCard;

// 存档/读档对话框
class SaveLoadDialog : public QDialog
{
    Q_OBJECT

public:
    enum class Mode { Save, Load };

    explicit SaveLoadDialog(Mode mode, QWidget* parent = nullptr);

    int selectedSlot() const { return m_selectedSlot; } // 返回选中的槽位索引，-1 表示未选中
    // 仅保存模式：获取用户输入的备注文本，去除前后空白
    QString label() const { return m_labelInput ? m_labelInput->text().trimmed() : QString(); }

private slots:
    void onSlotClicked(int slot);
    void onConfirmClicked();
    void onCancelClicked();

private:
    void setupUI();
    void refreshSlots();

    Mode m_mode; // 当前模式：保存或读取
    int m_selectedSlot = -1; // 当前选中的槽位索引，-1 表示未选中
    std::vector<SaveSlotCard*> m_cards;

    // 顶部当前状态摘要（仅保存模式）
    QLabel* m_currentSummaryLabel = nullptr;

    // 备注输入（仅保存模式）
    QLineEdit* m_labelInput = nullptr;

    // 底部按钮
    QPushButton* m_confirmButton = nullptr;
    QPushButton* m_cancelButton = nullptr;
};

// 单个存档槽位卡片
class SaveSlotCard : public QFrame
{
    Q_OBJECT

public:
    explicit SaveSlotCard(int slot, QWidget* parent = nullptr);

    void setData(const SaveMeta& meta);
    void setSelected(bool selected);
    bool isSelected() const { return m_selected; }
    int slot() const { return m_slot; }
    bool isEmpty() const { return m_meta.isEmpty; }

signals:
    void clicked(int slot);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    void applyStyleSheet();

    int m_slot;
    SaveMeta m_meta;
    bool m_selected = false;
    bool m_hovered = false;

    QLabel* m_slotNumberLabel = nullptr;
    QLabel* m_statusLabel = nullptr;  // "空" 或 时间戳
    QLabel* m_roundLabel = nullptr;  // 轮次
    QLabel* m_summaryLabel = nullptr;  // Lv.X  HP:XX  金币:XX
    QLabel* m_labelPreview = nullptr;  // 用户备注预览
};

#endif // GUI_SAVELOADDIALOG_H
