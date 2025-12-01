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

private:
    void setupToolbar();
    void setupDebugToolbar();
    void onStepByStepToggled(bool checked);
    void onPauseClicked();
    void onLoadFileClicked();
    void onClockStepClicked();
    void onPeriodClicked();
    void onInstructionStepClicked();
    void autoResize();
    void onShowMemory();
    void onReset();
    void onInfoClicked();

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

    std::string loadedProgramFilename;
};

