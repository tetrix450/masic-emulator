#include "MainWindow.hpp"

#include <QApplication>
#include <QCommandLineParser>
#include <QStyleHints>

uint8_t mem[65536];
uint8_t io[65536];
MainWindow* w;

void errorMessage(std::string message){
    w->errorMessage(message);
}

int main(int argc, char *argv[]){
    QApplication a(argc, argv);
    QApplication::setStyle("Fusion");


    CPUThread* cpu = new CPUThread();

    // Inicializar memoria de vídeo
    for(int i = 0; i < 8192; i++){
        io[i] = 0x20;
    }

    w = new MainWindow(cpu);

    {// Leer y procesar parámetros de la línea de comandos
        QCommandLineParser parser;
        parser.setApplicationDescription("Emulador MASIC, hecho por Diego Cerezo Rojas");
        parser.addHelpOption();
        parser.addVersionOption();

        // Opción debug: -d / --debug
        QCommandLineOption debugOption({"d", "debug"}, "Activar modo paso a paso al arrancar el programa.");
        parser.addOption(debugOption);

        // Argumento posicional opcional: filename
        parser.addPositionalArgument("archivo", "Archivo a cargar al iniciar el programa.");

        // Detectar error y mostrar ayuda
        if (!parser.parse(QCoreApplication::arguments())){
            QTextStream(stderr) << parser.errorText() << "\n";
            parser.showHelp(1);
            a.quit();
        }

        parser.process(a);

        const QStringList args = parser.positionalArguments();
        bool debugMode = parser.isSet(debugOption);

        if (!args.isEmpty()){
            QString filename = args.first();
            w->autoLoad(filename.toStdString(), debugMode);
        }
    }

    w->show();
    return a.exec();
}
