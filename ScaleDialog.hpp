#ifndef SCALEDIALOG_HPP
#define SCALEDIALOG_HPP


#include <QDialog>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>

class ScaleDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ScaleDialog(QWidget* parent = nullptr, double currentScale = 1);
    double getScale() const;

private:
    QDoubleSpinBox* spin;
};

#endif // SCALEDIALOG_HPP
