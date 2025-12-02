#include "HexViewWidget.hpp"
#include <QPainter>
#include <QFontDatabase>
#include <QScrollBar>
#include <QString>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QColor>

extern uint8_t mem[65536];

HexViewWidget::HexViewWidget(QWidget* parent, CPUThread* cpu):
    QAbstractScrollArea(parent),
    totalBytes(65536),
    firstRow(0),
    editAddress(-1),
    cpu(cpu)
{
    bytesPerRow = 16;
    QFont mono = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    viewport()->setFont(mono);

    QFontMetrics fm(mono);
    lineHeight = fm.height();
    byteWidth  = fm.horizontalAdvance("FF ");

    totalRows = (totalBytes + bytesPerRow - 1) / bytesPerRow;

    updateScrollBar();

    connect(verticalScrollBar(), &QScrollBar::valueChanged,
            this, [this](int v){
                firstRow = v;
                viewport()->update();
            });
}

void HexViewWidget::updateScrollBar(){
    verticalScrollBar()->setRange(0, totalRows - 1);
    verticalScrollBar()->setSingleStep(1);
    verticalScrollBar()->setPageStep(viewport()->height() / lineHeight);
}

void HexViewWidget::highlightPC(){
    highlightedByte = cpu->getPC();
    viewport()->update();
}

void HexViewWidget::paintEvent(QPaintEvent*){
    QPainter p(viewport());

    const int vpHeight = viewport()->height();
    const int visibleRows = vpHeight / lineHeight + 1;

    QFontMetrics fm(viewport()->font());

    int y = 0;

    for (int row = firstRow; row < firstRow + visibleRows; ++row){
        int baseAddr = row * bytesPerRow;
        if (baseAddr >= totalBytes)
            break;

        // Dirección
        p.drawText(0, y + fm.ascent(), QString("%1:").arg(baseAddr, 4, 16, QChar('0')));

        // Bytes
        for (int col = 0; col < bytesPerRow; ++col){
            int addr = baseAddr + col;
            if (addr >= totalBytes) break;

            int x = 60 + col * byteWidth;

            int boxWidth = fm.horizontalAdvance("FF");

            // resaltado o edición
            if (addr == editAddress){
                QRect r(x, y, boxWidth, lineHeight);
                p.fillRect(r, QColor(0, 120, 255, 100));

                QString text;
                if (editBuffer.isEmpty())
                    text = QString("%1 ").arg(mem[addr], 2, 16, QChar('0'));
                else
                    text = editBuffer + QString(" ").repeated(2 - editBuffer.size());

                p.drawText(x, y + fm.ascent(), text);
            }else if (addr == highlightedByte){
                QRect r(x, y, boxWidth, lineHeight);
                p.fillRect(r, QColor(128, 255, 128));
                p.drawText(x, y + fm.ascent(), QString("%1 ").arg(mem[addr], 2, 16, QChar('0')));
            }else{
                p.drawText(x, y + fm.ascent(),
                           QString("%1 ").arg(mem[addr], 2, 16, QChar('0')));
            }
        }

        y += lineHeight;
    }
}

void HexViewWidget::mousePressEvent(QMouseEvent* e)
{
    QFontMetrics fm(viewport()->font());

    int y = e->pos().y();
    int row = y / lineHeight + firstRow;
    if (row < 0 || row >= totalRows)
        return;

    int x = e->pos().x();

    // zona de bytes empieza en x >= 60
    if (x < 60)
        return;

    int col = (x - 60) / byteWidth;
    if (col < 0 || col >= bytesPerRow)
        return;

    int addr = row * bytesPerRow + col;
    if (addr >= totalBytes)
        return;

    editAddress = addr;
    editBuffer.clear();

    viewport()->setFocus();
    viewport()->update();
}

void HexViewWidget::keyPressEvent(QKeyEvent* e)
{
    if (editAddress < 0)
        return;

    int key = e->key();

    // cancelar con ESC
    if (key == Qt::Key_Escape)
    {
        editAddress = -1;
        editBuffer.clear();
        viewport()->update();
        return;
    }

    QString text = e->text().toUpper();
    if (text.isEmpty())
        return;

    QChar c = text.at(0);

    bool hex = (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F');
    if (!hex)
        return;

    editBuffer.append(c);

    if (editBuffer.size() == 2){
        bool ok = false;
        uint8_t value = editBuffer.toUInt(&ok, 16);
        if (ok)
            mem[editAddress] = value;

        if (editBuffer.size() == 2){
            bool ok = false;
            uint8_t value = editBuffer.toUInt(&ok, 16);
            if (ok)
                mem[editAddress] = value;

            // Avanzar automáticamente al siguiente byte
            int next = editAddress + 1;
            if (next < totalBytes){
                editAddress = next;
                editBuffer.clear();
            }else{
                // Final de memoria: cancelar edición
                editAddress = -1;
                editBuffer.clear();
            }
        }

        editBuffer.clear();
    }

    viewport()->update();
}

int HexViewWidget::getWidth(){
    return byteWidth*(3 + bytesPerRow);
}
