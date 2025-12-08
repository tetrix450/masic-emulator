#include "MemoryWindow.hpp"
#include "HexViewWidget.hpp"
#include <QScreen>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QTimer>
#include <QApplication>

MemoryWindow::MemoryWindow(QWidget* parent, CPUThread* cpu): QDialog(parent){
    setWindowTitle("Memoria y registros (HEX)");

    hexView = new HexViewWidget(this, cpu);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(hexView, 1);
    registers = new RegistersWidget(this, cpu);
    layout->addWidget(registers, 1);

    setLayout(layout);

    // Actualización automática
    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        this->updateView();
    });
    timer->start(20);

    resize(hexView->getWidth() + 50, 600);
}

void MemoryWindow::updateView(){
    hexView->highlightPC();
    registers->updateRegisters();
}
