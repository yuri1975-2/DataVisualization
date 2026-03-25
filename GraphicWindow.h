#pragma once

#include <QWidget>
#include <QVector>
#include "ui_GraphicWindow.h"
#include <utility>
#include <QString>
#include <atomic>
#include <memory>
#include <QFuture>

using namespace std;

class GraphicWindow : public QWidget
{
    Q_OBJECT

public:
    GraphicWindow(QWidget *parent = nullptr);
    ~GraphicWindow();

public slots:
    void onIndexChanged(int);
    void onDataLoaded(QVector<QVector<QVector<int16_t>>>);

private:
    struct TDataScreen
    {
        TDataScreen(const int numberOfDataPoints, const int numberOfScreenPoints, const double remain)
            : mNumberOfDataPoints(numberOfDataPoints), mNumberOfScreenPoints(numberOfScreenPoints), mRemain(remain) {};

        int mNumberOfDataPoints{ 0 };
        int mNumberOfScreenPoints{ 0 };
        double mRemain{ 0.0 };
    };

    static const uint16_t MAX_VALUE_FOR_COLOR = 7000;
    static constexpr uint16_t MAX_VALUE_FOR_COLOR_HALF = MAX_VALUE_FOR_COLOR / 2;
    const QString WAIT_MESSAGE = tr("Build image... Please, wait");
    const QString WINDOW_TITLE = tr("B-scan");
    static const auto GRAPHIC_LABELS_MARGIN = 60;
    static const auto GRAPHIC_LEGEND_WIDTH = 20;
    static const auto GRAPHIC_LEGEND_GAP = 20;
    static const auto MARK_SIZE = 5;

    static const auto GRAPHIC_LEGEND_MARK_NUMBER = 8;
    static const auto GRAPHIC_Y_MARKS_NUMBER = 9;
    static const auto GRAPHIC_Y_MARKS_MAX_DEPICTED_VALUE = 8000;
    static const auto GRAPHIC_Y_MAX_VALUE = 8192;
    static const auto GRAPHIC_X_MARKS_NUMBER = 7;
    static const auto GRAPHIC_X_MARKS_MAX_DEPICTED_VALUE = 60;
    static const auto GRAPHIC_X_MAX_VALUE = 64;

    Ui::GraphicWindowClass ui;
    QVector<QVector<QVector<int16_t>>> mData;
    int mPaintedForIndex{ -1 };
    int mRequiredIndex{ 0 };
    QFuture<void> mFuture;
    
    unique_ptr<QPixmap> mRenderBuffer{nullptr};
    atomic<bool> mIsImageBuiding{ false };

    /*
    * based upon @ratio and previously unused @remain
    * returns integer length that fit to @ratio + @remain and new unused @remain
    * for instance, for @ratio = 3.33 (i.e 3 elements from 10 points)
    * lengths will be returned are (3; 4; 3)
    * remains will be returned are (0.33; -0.33; 0)
    */
    static pair<int, double> getPointsNumberInInterval(const double ratio, const double remain);
    
    /*
    * based upon @ratio (between lengths of graphic and data) and unused @remain
    * returns number of data points which corresponds screen (graphic points) or vice versa
    * and new unused remain
    */
    static TDataScreen getDataScreenPoints(const double ratio, const double remain);


    /*
    * with given @dataRect found median (arithmetic)
    * for all containing values
    */
    uint16_t getMedianDataValue(const QRect& dataRect) const;

    auto needRepaint() const
    {
        return ((0 != mData.size()) && (mPaintedForIndex != mRequiredIndex));
    }
    static QColor getColor(uint16_t);
    void showWaitMessage(QPainter&);
    void showLegend(QPainter&, const int, const int, const int, const int) const;
    void showGraphic(const int, const int, const int, const int);
    void showGraphicMarks(QPainter&, const int, const int, const int, const int) const;

protected:
    void paintEvent(QPaintEvent* event) override;

signals:
    void imageBuildStarted();
    void imageBuildFinished();
};

