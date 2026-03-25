#pragma once

#include <QWidget>
#include <QVector>
#include "ui_MainWindow.h"
#include <QFuture>

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    virtual void resizeEvent(QResizeEvent* event);

private:
    Ui::MainWindowClass ui;
    QFuture<void> mFuture;

private slots:
    void onButtonPress();
    void onDataLoaded(QVector<QVector<QVector<int16_t>>>);
    void onImageBuildStarted();
    void onImageBuildFinished();

signals:
    void dataLoaded(QVector<QVector<QVector<int16_t>>>);
    void indexChanged(int);
};

