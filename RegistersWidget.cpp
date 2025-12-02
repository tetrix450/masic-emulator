#include "RegistersWidget.hpp"

RegistersWidget::RegistersWidget(QWidget* parent, CPUThread* cpu) : QWidget(parent), cpu(cpu) {
    view = new QTableView(this);
    model = new QStandardItemModel(0, 2, this); // columna 0 -> nombre, columna 1 -> valor
    model->setHeaderData(0, Qt::Horizontal, "Elemento");
    model->setHeaderData(1, Qt::Horizontal, "Valor");

    view->setModel(model);
    view->horizontalHeader()->setStretchLastSection(true);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(view);
    setLayout(layout);

    // Inicialización de filas
    QStringList regs = { "AC", "AUX", "D", "PC", "SP", "RI", "RCF", "S", "C", "V", "Z", "H" };
    model->setRowCount(regs.size());
    for (int i = 0; i < regs.size(); i++) {
        model->setItem(i, 0, new QStandardItem(regs[i]));
        model->setItem(i, 1, new QStandardItem("00"));
    }
}

void RegistersWidget::updateRegisters(){
    model->item(0,1)->setText(QString("%1").arg(cpu->getAC(), 2, 16, QLatin1Char('0')).toUpper());
    model->item(1,1)->setText(QString("%1").arg(cpu->getAUX(), 2, 16, QLatin1Char('0')).toUpper());
    model->item(2,1)->setText(QString("%1").arg(cpu->getD(), 4, 16, QLatin1Char('0')).toUpper());
    model->item(3,1)->setText(QString("%1").arg(cpu->getPC(), 4, 16, QLatin1Char('0')).toUpper());
    model->item(4,1)->setText(QString("%1").arg(cpu->getSP(), 4, 16, QLatin1Char('0')).toUpper());
    model->item(5,1)->setText(QString("%1").arg(cpu->getRI(), 2, 16, QLatin1Char('0')).toUpper());
    model->item(6,1)->setText(QString("%1").arg(cpu->getRCF(), 2, 16, QLatin1Char('0')).toUpper());
    model->item(7,1)->setText(QString("%1").arg(cpu->getS(), 2, 16, QLatin1Char('0')).toUpper());
    model->item(7,1)->setText(QString("%1").arg(cpu->getC(), 2, 16, QLatin1Char('0')).toUpper());
    model->item(7,1)->setText(QString("%1").arg(cpu->getV(), 2, 16, QLatin1Char('0')).toUpper());
    model->item(7,1)->setText(QString("%1").arg(cpu->getZ(), 2, 16, QLatin1Char('0')).toUpper());
    model->item(7,1)->setText(QString("%1").arg(cpu->getH(), 2, 16, QLatin1Char('0')).toUpper());

}
