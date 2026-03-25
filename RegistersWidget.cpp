#include "RegistersWidget.hpp"

RegistersWidget::RegistersWidget(QWidget* parent, CPUThread* cpu)
    : QWidget(parent), cpu(cpu)
{
    view = new QTableView(this);
    model = new QStandardItemModel(0, 4, this);

    model->setHeaderData(0, Qt::Horizontal, "Elemento");
    model->setHeaderData(1, Qt::Horizontal, "Hex");
    model->setHeaderData(2, Qt::Horizontal, "Decimal");
    model->setHeaderData(3, Qt::Horizontal, "Binario");

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

        auto *hexItem = new QStandardItem("00");
        hexItem->setEditable(true);

        auto *decItem = new QStandardItem("000");
        decItem->setEditable(true);

        auto *binItem = new QStandardItem("0000000000000000");
        binItem->setEditable(true);

        model->setItem(i, 0, nameItem);
        model->setItem(i, 1, hexItem);
        model->setItem(i, 2, decItem);
        model->setItem(i, 3, binItem);
    }

    // Conectar cambios de la tabla con la CPU
    connect(model, &QStandardItemModel::itemChanged,
            this, &RegistersWidget::onItemChanged);
}

void RegistersWidget::updateRegisters()
{
    // Evita recursividad al actualizar desde CPU
    QSignalBlocker blocker(model);

    // Hexadecimal
    model->item(0,1)->setText(QString("%1").arg(cpu->getAC(), 2, 16, QLatin1Char('0')).toUpper());
    model->item(1,1)->setText(QString("%1").arg(cpu->getAUX(), 2, 16, QLatin1Char('0')).toUpper());
    model->item(2,1)->setText(QString("%1").arg(cpu->getD(), 4, 16, QLatin1Char('0')).toUpper());
    model->item(3,1)->setText(QString("%1").arg(cpu->getPC(), 4, 16, QLatin1Char('0')).toUpper());
    model->item(4,1)->setText(QString("%1").arg(cpu->getSP(), 4, 16, QLatin1Char('0')).toUpper());
    model->item(5,1)->setText(QString("%1").arg(cpu->getRI(), 2, 16, QLatin1Char('0')).toUpper());
    model->item(6,1)->setText(QString("%1").arg(cpu->getRCF(), 2, 16, QLatin1Char('0')).toUpper());
    model->item(7,1)->setText(QString("%1").arg(cpu->getS(), 1, 16, QLatin1Char('0')).toUpper());
    model->item(8,1)->setText(QString("%1").arg(cpu->getC(), 1, 16, QLatin1Char('0')).toUpper());
    model->item(9,1)->setText(QString("%1").arg(cpu->getV(), 1, 16, QLatin1Char('0')).toUpper());
    model->item(10,1)->setText(QString("%1").arg(cpu->getZ(), 1, 16, QLatin1Char('0')).toUpper());
    model->item(11,1)->setText(QString("%1").arg(cpu->getH(), 1, 16, QLatin1Char('0')).toUpper());

    // Decimal
    model->item(0,2)->setText(QString("%1").arg(cpu->getAC(), 3, 10, QLatin1Char('0')).toUpper());
    model->item(1,2)->setText(QString("%1").arg(cpu->getAUX(), 3, 10, QLatin1Char('0')).toUpper());
    model->item(2,2)->setText(QString("%1").arg(cpu->getD(), 5, 10, QLatin1Char('0')).toUpper());
    model->item(3,2)->setText(QString("%1").arg(cpu->getPC(), 5, 10, QLatin1Char('0')).toUpper());
    model->item(4,2)->setText(QString("%1").arg(cpu->getSP(), 5, 10, QLatin1Char('0')).toUpper());
    model->item(5,2)->setText(QString("%1").arg(cpu->getRI(), 3, 10, QLatin1Char('0')).toUpper());
    model->item(6,2)->setText(QString("%1").arg(cpu->getRCF(), 2, 10, QLatin1Char('0')).toUpper());
    model->item(7,2)->setText(QString("%1").arg(cpu->getS(), 1, 10, QLatin1Char('0')).toUpper());
    model->item(8,2)->setText(QString("%1").arg(cpu->getC(), 1, 10, QLatin1Char('0')).toUpper());
    model->item(9,2)->setText(QString("%1").arg(cpu->getV(), 1, 10, QLatin1Char('0')).toUpper());
    model->item(10,2)->setText(QString("%1").arg(cpu->getZ(), 1, 10, QLatin1Char('0')).toUpper());
    model->item(11,2)->setText(QString("%1").arg(cpu->getH(), 1, 10, QLatin1Char('0')).toUpper());

    // Binario
    model->item(0,3)->setText(QString("%1").arg(cpu->getAC(), 8, 2, QLatin1Char('0')).toUpper());
    model->item(1,3)->setText(QString("%1").arg(cpu->getAUX(), 8, 2, QLatin1Char('0')).toUpper());
    model->item(2,3)->setText(QString("%1").arg(cpu->getD(), 16, 2, QLatin1Char('0')).toUpper());
    model->item(3,3)->setText(QString("%1").arg(cpu->getPC(), 16, 2, QLatin1Char('0')).toUpper());
    model->item(4,3)->setText(QString("%1").arg(cpu->getSP(), 16, 2, QLatin1Char('0')).toUpper());
    model->item(5,3)->setText(QString("%1").arg(cpu->getRI(), 8, 2, QLatin1Char('0')).toUpper());
    model->item(6,3)->setText(QString("%1").arg(cpu->getRCF(), 4, 2, QLatin1Char('0')).toUpper());
    model->item(7,3)->setText(QString("%1").arg(cpu->getS(), 1, 2, QLatin1Char('0')).toUpper());
    model->item(8,3)->setText(QString("%1").arg(cpu->getC(), 1, 2, QLatin1Char('0')).toUpper());
    model->item(9,3)->setText(QString("%1").arg(cpu->getV(), 1, 2, QLatin1Char('0')).toUpper());
    model->item(10,3)->setText(QString("%1").arg(cpu->getZ(), 1, 2, QLatin1Char('0')).toUpper());
    model->item(11,3)->setText(QString("%1").arg(cpu->getH(), 1, 2, QLatin1Char('0')).toUpper());

    view->viewport()->update();
}

void RegistersWidget::onItemChanged(QStandardItem *item)
{
    if (!item) return;
    if (item->column() == 0) return;

    int row = item->row();
    QString text = item->text().trimmed();

    if(item->column() == 1){ // Hexadecimal
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

        // Normalizar formato (02 o 04 dígitos hex)
        QSignalBlocker blocker(model);
        int width = (row == 2 || row == 3 || row == 4) ? 4 : 2;
        item->setText(QString("%1").arg(value, width, 16, QLatin1Char('0')).toUpper());
    }else if(item->column() == 2){ // Decimal
        bool ok = false;
        uint32_t value = text.toUInt(&ok, 10);
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

        // Normalizar formato (3 o 5 dígitos decimales)
        QSignalBlocker blocker(model);
        int width = (row == 2 || row == 3 || row == 4) ? 5 : 3;
        item->setText(QString("%1").arg(value, width, 10, QLatin1Char('0')).toUpper());
    }else if(item->column() == 3){ // Binario
        bool ok = false;
        uint32_t value = text.toUInt(&ok, 2);
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

        // Normalizar formato (8 o 16 dígitos bin)
        QSignalBlocker blocker(model);
        int width = (row == 2 || row == 3 || row == 4) ? 16 : 8;
        item->setText(QString("%1").arg(value, width, 2, QLatin1Char('0')).toUpper());
    }


}
