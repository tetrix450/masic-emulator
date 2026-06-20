#ifndef HEXVIEWWIDGET_H
#define HEXVIEWWIDGET_H

#include <QAbstractScrollArea>
#include "CPUThread.hpp"
#include <QPoint>
#include <QLineEdit>

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
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;

private:
    CPUThread* cpu;
    int bytesPerRow;
    int lineHeight;
    int byteWidth;
    int totalBytes;

    QLineEdit* searchBar = nullptr;

    int firstRow;
    int totalRows;

    int selectionStart = -1;
    int selectionEnd   = -1;
    int editIndexInSelection = 0;
    bool selecting = false;

    int highlightedByte = -1;

    int editAddress;
    QString editBuffer;

    void updateScrollBar();
    int posToAddress(const QPoint& pos) const;
};

#endif
