#include "mainwindow.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("vpacman");
    a.setApplicationVersion(VPACMAN_VERSION);
    a.setDesktopFileName("vpacman");
    a.setWindowIcon(QIcon::fromTheme("vpacman", QIcon(":/icons/vpacman.svg")));
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(a);
    MainWindow w;
    w.show();
    return a.exec();
}
