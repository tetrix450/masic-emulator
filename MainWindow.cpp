#include "MainWindow.hpp"
#include "CPUThread.hpp"
#include "qicon.h"
#include "qmainwindow.h"
#include "qobject.h"
#include "VideoWidget.hpp"
#include "PeriodDialog.hpp"
#include <QMessageBox>
#include <QRegularExpression>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QScreen>
#include <QWidget>
#include <QVBoxLayout>
#include <QApplication>
#include <QStatusBar>
#include <QFileDialog>
#include <QString>
#include <QInputDialog>
#include <string>

extern uint8_t mem[65536];
extern uint8_t io[65536];

MainWindow::MainWindow(CPUThread* cpu):cpu(cpu) {
    // Poner título a la ventana principal
    this->setWindowTitle("EMUMASIC");

    // Escalar y centrar
    float windowScale = 1.5;
    resize(640*windowScale, 480*windowScale);
    center();

    // Colocar las toolbars
    setupToolbar();
    setupDebugToolbar();
    sBar = this->statusBar();

    // Preparar el widget de vídeo
    video = new GLWidget(this);
    video->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->setCentralWidget(video);

    memWin = new MemoryWindow(this);
    memWin->highlightByte(cpu->getPC());
}

void MainWindow::autoResize(){
    layout()->invalidate();
    layout()->activate();
    adjustSize();
}

void MainWindow::setupToolbar() {
    QMenu* menuFile = menuBar()->addMenu("&Archivo");
    QAction* loadFileAction = menuFile->addAction("&Cargar binario...");
    connect(loadFileAction, &QAction::triggered, this, &MainWindow::onLoadFileClicked);

    QMenu* menuView = menuBar()->addMenu("&Ver");
    menuView->addAction("&Escalar ventana...");

    QMenu* menuSettings = menuBar()->addMenu("&Configuración");
    QAction* periodAction = menuSettings->addAction("&Periodo de reloj...");
    connect(periodAction, &QAction::triggered, this, &MainWindow::onPeriodClicked);

    QMenu* menuDebugger = menuBar()->addMenu("&Depurador");

    stepByStepCheckboxAction = new QAction("&Paso a paso", this);
    stepByStepCheckboxAction->setCheckable(true);
    stepByStepCheckboxAction->setChecked(false);

    connect(stepByStepCheckboxAction, &QAction::toggled, this, &MainWindow::onStepByStepToggled);

    menuDebugger->addAction(stepByStepCheckboxAction);
    menuDebugger->addAction("Mostrar &registros");

    QAction* memAction = menuDebugger->addAction("Mostrar &memoria");
    connect(memAction, &QAction::triggered, this, &MainWindow::onShowMemory);

    QMenu* menuHelp = menuBar()->addMenu("&Ayuda");
    menuHelp->addAction("&Información...");
}

void MainWindow::onShowMemory() {
    cpu->pause();
    pauseAction->setIcon(QIcon(":/icons/play.png"));
    memWin->show();
    memWin->raise();
    memWin->activateWindow();
}


void MainWindow::onPeriodClicked(){
    PeriodDialog dlg(this, cpu->getPeriodNs());

    if (dlg.exec() == QDialog::Accepted) {
        double ns = dlg.valorEnNanosegundos();
        std::string message = "Periodo: " + std::to_string(ns) + " ns";
        sBar->showMessage(QString::fromStdString(message), 5000);
        cpu->setPeriodNs(ns);
    }
}

void MainWindow::center(){
    // Obtener pantalla principal
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();

    // Calcular posición para centrar
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;

    // Mover la ventana
    move(x, y);
}

void MainWindow::errorMessage(std::string message){
    QMessageBox::critical(this, "Error", message.c_str());
    std::exit(1);
}

void MainWindow::warningMessage(std::string message){
    QMessageBox::warning(this, "Advertencia", message.c_str());
}

void MainWindow::setupDebugToolbar() {
    debugToolbar = new QToolBar("Debugger", this);
    debugToolbar->setStyleSheet("QToolBar { background-color:rgb(215, 215, 215); }");
    debugToolbar->setMovable(false);
    debugToolbar->setVisible(false);
    debugToolbar->setIconSize(QSize(24, 24));

    pauseAction = debugToolbar->addAction(QIcon(":/icons/play.png"), "Ejecutar/Pausar");
    connect(pauseAction, &QAction::triggered, this, &MainWindow::onPauseClicked);

    stepAction = debugToolbar->addAction(QIcon(":/icons/next.png"), "Avanzar reloj");
    connect(stepAction, &QAction::triggered, this, &MainWindow::onClockStepClicked);

    stepInstruction = debugToolbar->addAction(QIcon(":/icons/jump.png"), "Avanzar instrucción");
    connect(stepInstruction, &QAction::triggered, this, &MainWindow::onInstructionStepClicked);

    resetAction = debugToolbar->addAction(QIcon(":/icons/reset.png"), "Reset");
    connect(resetAction, &QAction::triggered, this, &MainWindow::onReset);

    addToolBar(Qt::TopToolBarArea, debugToolbar);
}

void MainWindow::onInstructionStepClicked(){
    cpu->pause();
    pauseAction->setIcon(QIcon(":/icons/play.png"));
    cpu->stepInstruction();
    memWin->highlightByte(cpu->getPC());
    sBar->showMessage("El reloj avanzó 1 instrucción...", 1000);
}

void MainWindow::onReset(){
    if(loaded){
        autoLoad(loadedProgramFilename, true);
        sBar->showMessage("CPU reseteada y programa recargado. CPU en pausa.");
        memWin->highlightByte(cpu->getPC());
    }else{
        warningMessage("No se ha cargado ningún programa...");
    }
}

void MainWindow::autoLoad(std::string filename, bool debug){
    QFile file(QString::fromStdString(filename));
    if (!file.open(QIODevice::ReadOnly)) {
        errorMessage("Error al abrir el archivo: " + filename);
        return;
    }

    QByteArray data = file.readAll();
    int len = qMin(data.size(), 65536);
    memcpy(mem, data.constData(), len);

    loaded = true;
    cpu->reset();

    // Activate debug?
    if(debug){
        onStepByStepToggled(true);
    }else{
        cpu->start();
    }

    // Clear VRAM
    for(int i = 0; i < 8192; i++){
        io[i] = 0x20;
    }
}

void MainWindow::onLoadFileClicked() {
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Selecciona un programa",
        "./",
        "*.mmc"
        );

    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            errorMessage("Error al abrir el archivo: " + fileName.toStdString());
            return;
        }

        QByteArray data = file.readAll();
        int len = qMin(data.size(), 65536);
        memcpy(mem, data.constData(), len);

        loaded = true;
        cpu->reset();

        if(!stepByStepCheckboxAction->isChecked()){
            cpu->start();
        }

        // Clear VRAM
        for(int i = 0; i < 8192; i++){
            io[i] = 0x20;
        }

        sBar->showMessage("Programa cargado correctamente.", 5000);
        loadedProgramFilename = fileName.toStdString();

        memWin->highlightByte(cpu->getPC());
    } else {
        warningMessage("No se ha cargado ningún programa...");
    }
}

void MainWindow::onClockStepClicked(){
    cpu->pause();
    pauseAction->setIcon(QIcon(":/icons/play.png"));
    cpu->step();
    memWin->highlightByte(cpu->getPC());
    sBar->showMessage("El reloj avanzó 1 ciclo...", 1000);
}

void MainWindow::onStepByStepToggled(bool checked) {
    if(checked){
        cpu->pause();
        pauseAction->setIcon(QIcon(":/icons/play.png"));
    }
    debugToolbar->setVisible(checked);
}

void MainWindow::onPauseClicked() {
    bool running = cpu->isRunning();
    if (running) {
        pauseAction->setIcon(QIcon(":/icons/play.png"));
        cpu->pause();
        sBar->showMessage("Pausado.");
    } else if(loaded) {
        pauseAction->setIcon(QIcon(":/icons/pause.png"));
        cpu->start();
        sBar->showMessage("Ejecutando...");
    }else{
        sBar->showMessage("No se ha cargado ningún programa...");
    }
}
