#pragma once
#include "CPUThread.hpp"
#include "MemoryWindow.hpp"
#include <QAction>
#include <QMainWindow>
#include <QToolBar>
#include <QAction>
#include <QTextEdit>
class GLWidget;

class MainWindow : public QMainWindow{
public:
    MainWindow(CPUThread* cpu);
    void center();
    void errorMessage(std::string message);
    void warningMessage(std::string message);
    void autoLoad(std::string filename, bool debug);
    void assembleAndLoad(QString filename);

private:
    void setupToolbar();
    void setupDebugToolbar();
    void onStepByStepToggled(bool checked);
    void onPauseClicked();
    void onLoadFileBinaryClicked();
    void onLoadFileSourceClicked();
    void onClockStepClicked();
    void onPeriodClicked();
    void onInstructionStepClicked();
    void autoResize();
    void onShowMemory();
    void onReset();
    void onInfoClicked();
    void onScaleClicked();
    void openManual();
    void updateTitle();

    bool loaded = false;
    CPUThread* cpu;
    GLWidget* video;
    QStatusBar* sBar = nullptr;
    QToolBar *debugToolbar = nullptr;
    QAction *pauseAction = nullptr;
    QAction *stepInstruction = nullptr;
    QAction *stepAction = nullptr;
    QAction *resetAction = nullptr;
    QAction *stepByStepCheckboxAction = nullptr;
    QWidget *openglWidget = nullptr;
    QTextEdit *memoryWidget = nullptr;
    MemoryWindow* memWin = nullptr;
    float currentScale = 1.0;

    std::string loadedProgramFilename;
};

