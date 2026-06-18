#ifndef GUI_SETTLEMENTDIALOG_H
#define GUI_SETTLEMENTDIALOG_H

#include <QDialog>
#include "core/game.h" // for SettlementInfo, BattleResult

class QLabel;

class SettlementDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettlementDialog(const SettlementInfo& info, QWidget* parent = nullptr);

private:
    QString resultText(BattleResult result) const;
    QString hpChangeLabel(const QString& side, int before, int after) const;
    QString goldChangeLabel(const QString& side, int before, int after, int interest, int streakBonus) const;
    QString streakStatusLabel(int winStreak, int loseStreak) const;
};

#endif // GUI_SETTLEMENTDIALOG_H
