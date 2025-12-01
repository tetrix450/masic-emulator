#include "PeriodDialog.hpp"

PeriodDialog::PeriodDialog(QWidget* parent, double currentPeriodNs)
    : QDialog(parent)
{
    setWindowTitle("Periodo de reloj");

    auto layout = new QFormLayout(this);

    // Campo numérico
    spin = new QDoubleSpinBox(this);
    spin->setRange(1, 1e12);
    spin->setDecimals(0);
    spin->setValue(currentPeriodNs);

    // Combo de unidades
    combo = new QComboBox(this);
    combo->addItems({"ns", "us", "ms", "s"});

    // Layout horizontal
    auto hbox = new QHBoxLayout;
    hbox->addWidget(spin);
    hbox->addWidget(combo);

    layout->addRow("Periodo:", hbox);

    // Botones OK/Cancel
    auto botones = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel
        );
    layout->addWidget(botones);

    connect(botones, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(botones, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

double PeriodDialog::valorEnNanosegundos() const
{
    double v = spin->value();
    QString u = combo->currentText().toLower();

    if (u == "ns") return v;
    if (u == "us") return v * 1000.0;
    if (u == "ms") return v * 1'000'000.0;
    if (u == "s")  return v * 1'000'000'000.0;

    return v;
}
