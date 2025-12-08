#include "RegistersWidget.hpp"

RegistersWidget::RegistersWidget(QWidget* parent, CPUThread* cpu)
    : QWidget(parent), cpu(cpu)
{
    view = new QTableView(this);
    model = new QStandardItemModel(0, 2, this);

    model->setHeaderData(0, Qt::Horizontal, "Elemento");
    model->setHeaderData(1, Qt::Horizontal, "Valor");

    view->setModel(model);
    view->horizontalHeader()->setStretchLastSection(true);
    view->setEditTriggers(QAbstractItemView::DoubleClicked |
                          QAbstractItemView::EditKeyPressed);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(view);
    setLayout(layout);

    // Filas iniciales
    QStringList regs = { "AC", "AUX", "D", "PC", "SP", "RI", "RCF",
                         "S", "C", "V", "Z", "H" };

    model->setRowCount(regs.size());

    for (int i = 0; i < regs.size(); ++i) {
        auto *nameItem = new QStandardItem(regs[i]);
        nameItem->setEditable(false);

        auto *valueItem = new QStandardItem("00");
        valueItem->setEditable(true);

        model->setItem(i, 0, nameItem);
        model->setItem(i, 1, valueItem);
    }

    // Conectar cambios de la tabla con la CPU
    connect(model, &QStandardItemModel::itemChanged,
            this, &RegistersWidget::onItemChanged);
}

void RegistersWidget::updateRegisters()
{
    // Evita recursividad al actualizar desde CPU
    QSignalBlocker blocker(model);

    model->item(0,1)->setText(QString("%1").arg(cpu->getAC(), 2, 16, QLatin1Char('0')).toUpper());
    model->item(1,1)->setText(QString("%1").arg(cpu->getAUX(), 2, 16, QLatin1Char('0')).toUpper());
    model->item(2,1)->setText(QString("%1").arg(cpu->getD(), 4, 16, QLatin1Char('0')).toUpper());
    model->item(3,1)->setText(QString("%1").arg(cpu->getPC(), 4, 16, QLatin1Char('0')).toUpper());
    model->item(4,1)->setText(QString("%1").arg(cpu->getSP(), 4, 16, QLatin1Char('0')).toUpper());
    model->item(5,1)->setText(QString("%1").arg(cpu->getRI(), 2, 16, QLatin1Char('0')).toUpper());
    model->item(6,1)->setText(QString("%1").arg(cpu->getRCF(), 2, 16, QLatin1Char('0')).toUpper());
    model->item(7,1)->setText(QString("%1").arg(cpu->getS(), 2, 16, QLatin1Char('0')).toUpper());
    model->item(8,1)->setText(QString("%1").arg(cpu->getC(), 2, 16, QLatin1Char('0')).toUpper());
    model->item(9,1)->setText(QString("%1").arg(cpu->getV(), 2, 16, QLatin1Char('0')).toUpper());
    model->item(10,1)->setText(QString("%1").arg(cpu->getZ(), 2, 16, QLatin1Char('0')).toUpper());
    model->item(11,1)->setText(QString("%1").arg(cpu->getH(), 2, 16, QLatin1Char('0')).toUpper());

    view->viewport()->update();
}

void RegistersWidget::onItemChanged(QStandardItem *item)
{
    if (!item) return;
    if (item->column() != 1) return;

    int row = item->row();
    QString text = item->text().trimmed();

    bool ok = false;
    uint32_t value = text.toUInt(&ok, 16);
    if (!ok) return;

    // Aplicar a CPU según registro
    switch (row) {
        case 0:  cpu->setAC(value & 0xFF);   break;
        case 1:  cpu->setAUX(value & 0xFF);  break;
        case 2:  cpu->setD(value & 0xFFFF);  break;
        case 3:  cpu->setPC(value & 0xFFFF); break;
        case 4:  cpu->setSP(value & 0xFFFF); break;
        case 5:  cpu->setRI(value & 0xFF);   break;
        case 6:  cpu->setRCF(value & 0xFF);  break;
        case 7:  cpu->setS(value & 0x01);    break;
        case 8:  cpu->setC(value & 0x01);    break;
        case 9:  cpu->setV(value & 0x01);    break;
        case 10: cpu->setZ(value & 0x01);    break;
        case 11: cpu->setH(value & 0x01);    break;
        default: break;
    }

    // Normalizar formato (02 o 04 hex)
    QSignalBlocker blocker(model);
    int width = (row == 2 || row == 3 || row == 4) ? 4 : 2;
    item->setText(QString("%1").arg(value, width, 16, QLatin1Char('0')).toUpper());
}
