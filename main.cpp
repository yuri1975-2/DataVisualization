#include "MainWindow.h"
#include "GraphicWindow.h"
#include <QtWidgets/QApplication>
#include <QGuiApplication>
#include <QScreen>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow mainWindow;
    mainWindow.adjustSize();
    mainWindow.setFixedSize(QGuiApplication::primaryScreen()->geometry().width() / 4, mainWindow.height());
    mainWindow.move(0, 0);
    mainWindow.show();
    
    GraphicWindow graphicWindow;
    graphicWindow.setFixedSize(QGuiApplication::primaryScreen()->geometry().width() / 2,
        QGuiApplication::primaryScreen()->geometry().height() / 2);
    graphicWindow.move(mainWindow.width(), mainWindow.height());
    graphicWindow.show();

    QObject::connect(&mainWindow, SIGNAL(indexChanged(int)), &graphicWindow, SLOT(onIndexChanged(int)));
    QObject::connect(&mainWindow, SIGNAL(dataLoaded(QVector<QVector<QVector<int16_t>>>)),
        &graphicWindow, SLOT(onDataLoaded(QVector<QVector<QVector<int16_t>>>)));
    QObject::connect(&graphicWindow, SIGNAL(imageBuildStarted()), &mainWindow, SLOT(onImageBuildStarted()));
    QObject::connect(&graphicWindow, SIGNAL(imageBuildFinished()), &mainWindow, SLOT(onImageBuildFinished()));

    return app.exec();
}
