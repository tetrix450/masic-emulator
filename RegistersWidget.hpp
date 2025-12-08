#ifndef REGISTERSWIDGET_HPP
#define REGISTERSWIDGET_HPP

#include <QStandardItemModel>
#include <QTableView>
#include <QWidget>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QSignalBlocker>
#include "CPUThread.hpp"

class RegistersWidget : public QWidget {
    Q_OBJECT
public:
    explicit RegistersWidget(QWidget *parent, CPUThread* cpu);
    void updateRegisters();

private slots:
    void onItemChanged(QStandardItem *item);

private:
    QTableView *view;
    QStandardItemModel *model;
    CPUThread* cpu;
};

#endif // REGISTERSWIDGET_HPP
