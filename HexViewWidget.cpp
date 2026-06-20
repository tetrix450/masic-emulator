#include "HexViewWidget.hpp"
#include <QPainter>
#include <QFontDatabase>
#include <QScrollBar>
#include <QString>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QColor>
#include <QApplication>
#include <QClipboard>

extern uint8_t mem[65536];

HexViewWidget::HexViewWidget(QWidget* parent, CPUThread* cpu):
    QAbstractScrollArea(parent),
    totalBytes(65536),
    firstRow(0),
    editBuffer(),
    cpu(cpu)
{
    setFocusPolicy(Qt::StrongFocus);
    viewport()->setFocusPolicy(Qt::StrongFocus);

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

    searchBar = new QLineEdit(this);
    searchBar->setPlaceholderText("Go to address (hex)");
    searchBar->setFixedHeight(22);
    searchBar->move(0, 0);
    searchBar->raise();

    connect(searchBar, &QLineEdit::returnPressed, this, [this]() {
        QString txt = searchBar->text().trimmed();

        if (txt.startsWith("0x") || txt.startsWith("0X"))
            txt = txt.mid(2);

        bool ok = false;
        int addr = txt.toInt(&ok, 16);
        if (!ok) return;

        addr = qBound(0, addr, totalBytes - 1);

        int row = addr / bytesPerRow;

        verticalScrollBar()->setValue(row);
        firstRow = row;
        viewport()->update();
    });
}

void HexViewWidget::resizeEvent(QResizeEvent* e){
    QAbstractScrollArea::resizeEvent(e);

    if (searchBar){
        int h = searchBar->sizeHint().height();

        searchBar->setFixedWidth(viewport()->width());
        searchBar->move(0, 0);

        viewport()->setGeometry(
            0,
            h,
            width(),
            height() - h
            );
    }

    updateScrollBar();
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

int HexViewWidget::posToAddress(const QPoint& pos) const {
    int row = pos.y() / lineHeight + firstRow;
    if (row < 0 || row >= totalRows) return -1;

    int x = pos.x();
    if (x < 60) return -1;

    int col = (x - 60) / byteWidth;
    if (col < 0 || col >= bytesPerRow) return -1;

    int addr = row * bytesPerRow + col;
    if (addr >= totalBytes) return -1;

    return addr;
}

void HexViewWidget::paintEvent(QPaintEvent*){
    QPainter p(viewport());
    const int vpHeight = viewport()->height();
    const int visibleRows = vpHeight / lineHeight + 1;
    QFontMetrics fm(viewport()->font());

    int y = 0;

    int selStart = qMin(selectionStart, selectionEnd);
    int selEnd   = qMax(selectionStart, selectionEnd);

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

            // Resaltado de edición múltiple
            if (addr >= selStart && addr <= selEnd){
                QColor color = (addr == selStart + editIndexInSelection) ? QColor(0, 192, 255, 192)
                                                                         : QColor(192, 192, 255, 128);
                p.fillRect(QRect(x, y, boxWidth, lineHeight), color);
            } else if (addr == highlightedByte){
                p.fillRect(QRect(x, y, boxWidth, lineHeight), QColor(192, 0, 0));
            }

            p.drawText(x, y + fm.ascent(), QString("%1 ").arg(mem[addr], 2, 16, QChar('0')));
        }

        y += lineHeight;
    }
}

void HexViewWidget::mousePressEvent(QMouseEvent* e){
    int addr = posToAddress(e->pos());
    if (addr < 0) return;

    if (e->button() == Qt::LeftButton){
        selectionStart = addr;
        selectionEnd   = addr;
        editIndexInSelection = 0;
        editBuffer.clear();
        selecting = true;
        viewport()->setFocus();
        viewport()->update();
    }
}

void HexViewWidget::mouseMoveEvent(QMouseEvent* e){
    if (!selecting) return;
    int addr = posToAddress(e->pos());
    if (addr < 0) return;
    selectionEnd = addr;
    viewport()->update();
}

void HexViewWidget::mouseReleaseEvent(QMouseEvent*){
    selecting = false;
    viewport()->update();
}

void HexViewWidget::keyPressEvent(QKeyEvent* e){

    if (e->modifiers() & Qt::ControlModifier &&
        e->key() == Qt::Key_C){

        if (selectionStart >= 0 && selectionEnd >= 0){

            int selStart = qMin(selectionStart, selectionEnd);
            int selEnd   = qMax(selectionStart, selectionEnd);

            QString text;

            for (int addr = selStart; addr <= selEnd; ++addr){
                text += QString("%1").arg(mem[addr], 2, 16, QChar('0')).toUpper();

                if (addr != selEnd)
                    text += ' ';
            }

            QApplication::clipboard()->setText(text);
        }

        return;
    }

    if (selectionStart < 0 || selectionEnd < 0)
        return;

    int selStart = qMin(selectionStart, selectionEnd);
    int selEnd   = qMax(selectionStart, selectionEnd);

    QString text = e->text().toUpper();
    if (text.isEmpty()) return;

    QChar c = text.at(0);
    if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'))) return;

    editBuffer.append(c);

    if (editBuffer.size() == 2){
        bool ok;
        uint8_t value = editBuffer.toUInt(&ok, 16);
        if (ok){
            int addr = selStart + editIndexInSelection;
            if (addr <= selEnd)
                mem[addr] = value;

            // Avanzar al siguiente byte dentro de la selección
            editIndexInSelection++;
            if (selStart + editIndexInSelection > selEnd){
                // Fin de la selección
                selectionStart = selectionEnd = -1;
                editIndexInSelection = 0;
            }
        }
        editBuffer.clear();
    }

    viewport()->update();
}

int HexViewWidget::getWidth(){
    return byteWidth*(3 + bytesPerRow);
}
