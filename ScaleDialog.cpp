#include "ScaleDialog.hpp"

ScaleDialog::ScaleDialog(QWidget* parent, double currentScale):QDialog(parent){
    setWindowTitle("Escalar ventana");

    auto layout = new QFormLayout(this);

    // Campo numérico
    spin = new QDoubleSpinBox(this);
    spin->setRange(0.1, 10);
    spin->setDecimals(2);
    spin->setValue(currentScale);
    spin->setSingleStep(0.1);

    // Layout horizontal
    auto hbox = new QHBoxLayout;
    hbox->addWidget(spin);

    layout->addRow("Escala:", hbox);

    // Botones OK/Cancel
    auto botones = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel
        );
    layout->addWidget(botones);

    connect(botones, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(botones, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

double ScaleDialog::getScale() const{
    return spin->value();
}
