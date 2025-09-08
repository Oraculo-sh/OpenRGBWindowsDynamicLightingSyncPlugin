#include <QCoreApplication>
#include <QCommandLineParser>
#include <QTextStream>
#include <QTimer>

#include "WDLDriverServer.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("WindowsDynamicLightingDriver");
    QCoreApplication::setApplicationVersion("0.1");

    QCommandLineParser parser;
    parser.setApplicationDescription("Windows Dynamic Lighting Driver (Named Pipe/QLocalServer)");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption nameOpt({"n", "name"}, "Nome do servidor QLocalServer", "name", "OpenRGB_WDL_Driver");
    parser.addOption(nameOpt);
    parser.process(app);

    const QString serverName = parser.value(nameOpt);

    WDLDriverServer server(serverName);
    if (!server.start()) {
        QTextStream(stderr) << "Falha ao iniciar o servidor em '" << serverName << "'\n";
        return 1;
    }

    return app.exec();
}
