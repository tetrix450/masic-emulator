#ifndef MEMORYWINDOW_H
#define MEMORYWINDOW_H

#include <QDialog>

class HexViewWidget;

class MemoryWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MemoryWindow(QWidget* parent = nullptr);
    void highlightByte(size_t address);

private:
    HexViewWidget* hexView;
};

#endif
