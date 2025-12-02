#ifndef REGISTERSWIDGET_HPP
#define REGISTERSWIDGET_HPP

#include <QStandardItemModel>
#include <QTableView>
#include <QWidget>
#include <QHeaderView>
#include <QVBoxLayout>
#include "CPUThread.hpp"

class RegistersWidget : public QWidget {
    Q_OBJECT
public:
    RegistersWidget(QWidget *parent, CPUThread* cpu);
    void updateRegisters();

private:
    QTableView *view;
    QStandardItemModel *model;
    CPUThread* cpu;
};

#endif // REGISTERSWIDGET_HPP
