#ifndef HEXVIEWWIDGET_H
#define HEXVIEWWIDGET_H

#include <QAbstractScrollArea>
#include "CPUThread.hpp"

class HexViewWidget : public QAbstractScrollArea{
    Q_OBJECT

public:
    HexViewWidget(QWidget* parent, CPUThread* cpu);

    void highlightPC();
    int getWidth();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* e) override;
    void keyPressEvent(QKeyEvent* e) override;

private:
    CPUThread* cpu;
    int bytesPerRow;
    int lineHeight;
    int byteWidth;
    int totalBytes;

    int firstRow;
    int totalRows;

    int highlightedByte = -1;

    int editAddress;
    QString editBuffer;

    void updateScrollBar();
};

#endif
