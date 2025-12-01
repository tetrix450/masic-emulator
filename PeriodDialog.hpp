#ifndef PERIODDIALOG_H
#define PERIODDIALOG_H

#include <QDialog>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>

class PeriodDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PeriodDialog(QWidget* parent = nullptr, double currentPeriodNs = 1000);

    double valorEnNanosegundos() const;

private:
    QDoubleSpinBox* spin;
    QComboBox* combo;
};

#endif // PERIODDIALOG_H
