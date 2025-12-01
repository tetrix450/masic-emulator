#include "MemoryWindow.hpp"
#include "HexViewWidget.hpp"
#include <QVBoxLayout>

MemoryWindow::MemoryWindow(QWidget* parent): QDialog(parent){
    setWindowTitle("Memoria (HEX)");

    hexView = new HexViewWidget(this);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(hexView);

    setLayout(layout);

    resize(hexView->getWidth() + 50, 400);
}

void MemoryWindow::highlightByte(size_t address){
    hexView->highlightByte(address);
}
