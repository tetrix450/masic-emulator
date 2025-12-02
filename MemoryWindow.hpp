#ifndef MEMORYWINDOW_H
#define MEMORYWINDOW_H

#include <QDialog>
#include "RegistersWidget.hpp"

class HexViewWidget;

class MemoryWindow : public QDialog
{
    Q_OBJECT

public:
    MemoryWindow(QWidget* parent, CPUThread* cpu);
    void updateView();

private:
    HexViewWidget* hexView;
    RegistersWidget* registers;
};

#endif
