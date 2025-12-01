#ifndef HEXVIEWWIDGET_H
#define HEXVIEWWIDGET_H

#include <QAbstractScrollArea>
#include <cstddef>

class HexViewWidget : public QAbstractScrollArea
{
    Q_OBJECT

public:
    explicit HexViewWidget(QWidget* parent = nullptr);

    void highlightByte(std::size_t index);
    int getWidth();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* e) override;
    void keyPressEvent(QKeyEvent* e) override;

private:
    int bytesPerRow;
    int lineHeight;
    int byteWidth;
    int totalBytes;

    int firstRow;
    int totalRows;

    int highlightedByte = -1;

    // Edición
    int editAddress;
    QString editBuffer;

    void updateScrollBar();
};

#endif
