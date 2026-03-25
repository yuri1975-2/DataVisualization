#include "MainWindow.h"
#include <QFileDialog>
#include "ApplicationComponents/ApplicationComponents.h"
#include <QtConcurrent\QtConcurrent>

Q_DECLARE_METATYPE(QVector<QVector<QVector<int16_t>>>)

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    qRegisterMetaType<QVector<QVector<QVector<int16_t>>>>();

    connect(ui.pushButton, SIGNAL(pressed()), this, SLOT(onButtonPress()));
    connect(this, SIGNAL(dataLoaded(QVector<QVector<QVector<int16_t>>>)), this, SLOT(onDataLoaded(QVector<QVector<QVector<int16_t>>>)));
    connect(ui.horizontalSlider, SIGNAL(valueChanged(int)), SIGNAL(indexChanged(int)));
}

MainWindow::~MainWindow()
{
    if (mFuture.isRunning())
    {
        mFuture.cancel();
        mFuture.waitForFinished();
    }
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    ui.horizontalSlider->setFixedWidth(width() - 2 * ui.horizontalSlider->pos().x());
    ui.label->setVisible(false);
}

void MainWindow::onButtonPress()
{
    const auto filename(QFileDialog::getOpenFileName());
    if (filename.isEmpty())
    {
        return;
    }

    setEnabled(false);
    ui.label->setVisible(true);

    if (!mFuture.isRunning())
    {
        mFuture = QtConcurrent::run([=]()
            {
                const auto parser(ApplicationComponents::getParser());
                const auto result(parser->parse(filename.toUtf8().constData()));
                if (mFuture.isCanceled())
                {
                    return;
                }
                emit dataLoaded(result);
            });
    }
}

void MainWindow::onDataLoaded(QVector<QVector<QVector<int16_t>>>)
{
    setEnabled(true);
    ui.label->setVisible(false);
}

void MainWindow::onImageBuildStarted()
{
    setEnabled(false);
}

void MainWindow::onImageBuildFinished()
{
    setEnabled(true);
}