// STL
#include <memory>

// Qt
#include <QDebug>
#include <QScopedPointer>
#include <QUrl>
#include <QTemporaryDir>
#include <QApplication>
#include <QStringList>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>

// Local
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QTranslator* translator = new QTranslator();

#ifdef Q_OS_WIN
    setlocale(LC_ALL, "C");
#else
    std::setlocale(LC_ALL, "C");
#endif
    QString strLocaleName = QLocale::system().name();		// language_country (예:ko_KR)
    strLocaleName = strLocaleName.left(2);			// language 부분만 남기기.
    translator->load("qroiapp_ko.qm");
    QApplication::installTranslator(translator);

    MainWindow* window = new MainWindow();
    window->show();
    return app.exec();
}
