#include "MainWindow.hpp"
#include "CPUThread.hpp"
#include "qicon.h"
#include "qmainwindow.h"
#include "qobject.h"
#include "VideoWidget.hpp"
#include "PeriodDialog.hpp"
#include "ScaleDialog.hpp"
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
#include <QProcess>
#include <QFile>
#include <QTemporaryFile>
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>
#include <QSettings>
#include <QString>

extern uint8_t mem[65536];
extern uint8_t io[65536];

MainWindow::MainWindow(CPUThread* cpu):cpu(cpu) {
    // Poner título a la ventana principal
    this->setWindowTitle("QTMASIC");
    // Escalar y centrar
    currentScale = 1.5;
    resize(640*currentScale, 480*currentScale);
    center();

    // Colocar las toolbars
    setupToolbar();
    setupDebugToolbar();
    sBar = this->statusBar();

    // Preparar el widget de vídeo
    video = new GLWidget(this);
    video->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->setCentralWidget(video);

    memWin = new MemoryWindow(this, cpu);
    memWin->updateView();

    // -------------- Apartar memWin a la derecha --------------
    // Obtener las dimensiones de la pantalla
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();

    // Calcular el centro
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;

    // Mover la ventana
    memWin->move(x + this->width(), y);
    // ---------------------------------------------------------
}

void MainWindow::autoResize(){
    layout()->invalidate();
    layout()->activate();
    adjustSize();
}

void MainWindow::setupToolbar() {
    QMenu* menuFile = menuBar()->addMenu("&Archivo");
    QAction* loadFileSourceAction = menuFile->addAction("&Cargar código fuente...");
    connect(loadFileSourceAction, &QAction::triggered, this, &MainWindow::onLoadFileSourceClicked);
    QAction* loadFileBinaryAction = menuFile->addAction("&Cargar código máquina...");
    connect(loadFileBinaryAction, &QAction::triggered, this, &MainWindow::onLoadFileBinaryClicked);

    QMenu* menuSettings = menuBar()->addMenu("&Configuración");
    QAction* periodAction = menuSettings->addAction("&Periodo de reloj...");
    connect(periodAction, &QAction::triggered, this, &MainWindow::onPeriodClicked);
    QAction* scaleAction = menuSettings->addAction("&Escalar ventana...");
    connect(scaleAction, &QAction::triggered, this, &MainWindow::onScaleClicked);

    QMenu* menuDebugger = menuBar()->addMenu("&Depurador");

    stepByStepCheckboxAction = new QAction("&Paso a paso", this);
    stepByStepCheckboxAction->setCheckable(true);
    stepByStepCheckboxAction->setChecked(false);

    connect(stepByStepCheckboxAction, &QAction::toggled, this, &MainWindow::onStepByStepToggled);

    menuDebugger->addAction(stepByStepCheckboxAction);

    QAction* memAction = menuDebugger->addAction("Mostrar &memoria y registros");
    connect(memAction, &QAction::triggered, this, &MainWindow::onShowMemory);

    QMenu* menuHelp = menuBar()->addMenu("&Ayuda");
    QAction* infoAction = menuHelp->addAction("&Información...");
    connect(infoAction, &QAction::triggered, this, &MainWindow::onInfoClicked);
    QAction* guideAction = menuHelp->addAction("&Guía de programación...");
    connect(guideAction, &QAction::triggered, this, &MainWindow::openManual);
}

void MainWindow::openManual(){
    QFile resourceFile(":/guia_emasic.pdf");
    if (!resourceFile.open(QIODevice::ReadOnly)) {
        warningMessage("No se pudo abrir la guía");
        return;
    }

    QTemporaryFile tempFile(QDir::tempPath() + "/guia_emasic.pdf");
    tempFile.setAutoRemove(false);
    if (!tempFile.open()) {
        warningMessage("No se pudo crear el archivo temporal");
        return;
    }

    tempFile.write(resourceFile.readAll());
    tempFile.close();
    resourceFile.close();

    QDesktopServices::openUrl(QUrl::fromLocalFile(tempFile.fileName()));
}

void MainWindow::onScaleClicked(){

    ScaleDialog dlg(this, currentScale);

    if (dlg.exec() == QDialog::Accepted) {
        currentScale = dlg.getScale();
        resize(640*currentScale, 480*currentScale);
        center();
    }

}

void MainWindow::onInfoClicked(){
    QMessageBox::information(this, "Acerca de...", "Emulador del sistema MASIC (v1.0), hecho por Diego Cerezo Rojas.");
}

void MainWindow::onShowMemory() {
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
    memWin->updateView();
    sBar->showMessage("El reloj avanzó 1 instrucción...", 1000);
}

void MainWindow::onReset(){
    cpu->randomRAM();
    if(loaded){
        autoLoad(loadedProgramFilename, true);
        sBar->showMessage("CPU reseteada y programa recargado. CPU en pausa.");
        memWin->updateView();
    }else{
        warningMessage("No se ha cargado ningún programa...");
    }
}

void MainWindow::autoLoad(std::string filename, bool debug){
    loadedProgramFilename = filename;
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
        stepByStepCheckboxAction->setChecked(true);
        pauseAction->setIcon(QIcon(":/icons/play.png"));

        memWin->show();
        memWin->raise();
        memWin->activateWindow();
    }else{
        cpu->start();
    }

    QString message = "Programa cargado correctamente (" + QString::number(file.size()) + " bytes)";
    sBar->showMessage(message, 5000);
}

void MainWindow::assembleAndLoad(QString filename){
    QProcess process;

    // Check if emasic is missing
    connect(&process, &QProcess::errorOccurred, this,
    [](QProcess::ProcessError error){
        if (error == QProcess::FailedToStart) {
            QMessageBox::critical(nullptr,"Error al cargar el ensamblador",
              "EMASIC no pudo encontrarse. "
              "Asegúrese de que qtmasic y emasic estén en la misma carpeta o en el PATH.");
        }
    });

    process.start("emasic", {filename});
    process.waitForFinished();

    //QString stdoutText = process.readAllStandardOutput();
    QString stderrText = process.readAllStandardError();
    int exitCode = process.exitCode();

    if(exitCode != 0){
        errorMessage(stderrText.toStdString());
        sBar->showMessage("El código fuente contiene errores", 5000);
    }else{
        autoLoad("out.mmc",false);
    }
}

void MainWindow::onLoadFileSourceClicked(){
    // Conseguir la última ruta a la que se accedió
    QSettings settings;
    QString lastDir = settings.value("lastDir", "./").toString(); // Leer última ruta ó usar ./ si no existe

    // Abrir el diálogo de selección de archivo
    QFileDialog dlg(this);
    dlg.setFileMode(QFileDialog::ExistingFile);
    dlg.setDirectory(lastDir);
    dlg.setOption(QFileDialog::DontUseNativeDialog);

    if(dlg.exec()){
        QString filename = dlg.selectedFiles().first();

        // Guardar la carpeta del archivo seleccionado
        QFileInfo info(filename);
        settings.setValue("lastDir", info.absolutePath());

        assembleAndLoad(filename);
    }
}

void MainWindow::onLoadFileBinaryClicked() {
    // Conseguir la última ruta a la que se accedió
    QSettings settings;
    QString lastDir = settings.value("lastDir", "./").toString(); // Leer última ruta ó usar ./ si no existe

    // Abrir el diálogo de selección de archivo
    QFileDialog dlg(this);
    dlg.setFileMode(QFileDialog::ExistingFile);
    dlg.setDirectory(lastDir);
    dlg.setOption(QFileDialog::DontUseNativeDialog);

    if(dlg.exec()){
        QString fileName = dlg.selectedFiles().first();

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

            memWin->updateView();
        } else {
            warningMessage("No se ha cargado ningún programa...");
        }
    }
}

void MainWindow::onClockStepClicked(){
    cpu->pause();
    pauseAction->setIcon(QIcon(":/icons/play.png"));
    cpu->step();
    memWin->updateView();
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
