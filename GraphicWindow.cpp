#include "GraphicWindow.h"
#include <QPainter>
#include <QtConcurrent\QtConcurrent>
#include <QFontMetrics>

GraphicWindow::GraphicWindow(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    setWindowTitle(WINDOW_TITLE);
}

GraphicWindow::~GraphicWindow()
{
    if (mFuture.isRunning())
    {
        mFuture.cancel();
        mFuture.waitForFinished();
    }
}

void GraphicWindow::showWaitMessage(QPainter& painter)
{
    painter.begin(this);
    painter.fillRect(0, 0, width(), height(), palette().color(QWidget::backgroundRole()));
    QFontMetrics fm(font());
    painter.drawText(width() / 2 - fm.boundingRect(WAIT_MESSAGE).width() / 2, height() / 2 - fm.height() / 2, WAIT_MESSAGE);
    painter.end();
}

void GraphicWindow::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
    
    QPainter painter;
    if ((nullptr != mRenderBuffer) && (!mIsImageBuiding))
    {
        painter.begin(this);
        painter.drawPixmap(0, 0, *mRenderBuffer);
        painter.end();

        setWindowTitle(WINDOW_TITLE + ", TX = " + QString::number(mRequiredIndex));
    }

    if (mIsImageBuiding)
    {
        showWaitMessage(painter);
    }

    if (!needRepaint())
    {
        return;
    }

    showWaitMessage(painter);
    if (nullptr == mRenderBuffer)
    {
        mRenderBuffer = make_unique<QPixmap>(width(), height());
    }
    
    if (!mFuture.isRunning())
    {
        mFuture = QtConcurrent::run([=]()
            {
                mIsImageBuiding = true;
                QFontMetrics fm(font());
                showGraphic(GRAPHIC_LABELS_MARGIN, fm.height(),
                    width() - 2 * GRAPHIC_LABELS_MARGIN - GRAPHIC_LEGEND_WIDTH - GRAPHIC_LEGEND_GAP,
                    height() - 2.5 * fm.height());
                mIsImageBuiding = false;
            });

        mPaintedForIndex = mRequiredIndex;
    }
}

void GraphicWindow::showGraphic(const int x, const int y, const int width, const int height)
{
    emit imageBuildStarted();
    
    QPainter painter;
    painter.begin(mRenderBuffer.get());

    painter.fillRect(0, 0, this->width(), this->height(), palette().color(QWidget::backgroundRole()));

    // 1) found rects in data cell and in screen (graphic)
    // 2) based upon median value of dataRect found appropriate color
    // and fills by this one screenRect
    painter.setFont(font());
    const auto ratioX(1.0 * width / mData[mRequiredIndex].size());
    const auto ratioY(1.0 * height / mData[mRequiredIndex][0].size());
    QRect dataRect, screenRect;
    screenRect.setLeft(x);
    screenRect.setTop(y);
    auto currentYRemain(0.0);
    for (; screenRect.top() < (height + y);)
    {
        if (mFuture.isCanceled())
        {
            return;
        }
        const auto dataScreenPointsY(getDataScreenPoints(ratioY, currentYRemain));
        screenRect.setHeight(dataScreenPointsY.mNumberOfScreenPoints);
        dataRect.setHeight(dataScreenPointsY.mNumberOfDataPoints);
        currentYRemain = dataScreenPointsY.mRemain;

        auto currentXRemain(0.0);
        for (; screenRect.left() < (width + x);)
        {
            if (mFuture.isCanceled())
            {
                return;
            }
            const auto dataScreenPointsX(getDataScreenPoints(ratioX, currentXRemain));
            screenRect.setWidth(dataScreenPointsX.mNumberOfScreenPoints);
            dataRect.setWidth(dataScreenPointsX.mNumberOfDataPoints);
            currentXRemain = dataScreenPointsX.mRemain;

            painter.fillRect(screenRect.left(), screenRect.top(), screenRect.width(), screenRect.height(),
                getColor(getMedianDataValue(dataRect)));

            screenRect.setLeft(screenRect.left() + screenRect.width());
            dataRect.setLeft(dataRect.left() + dataRect.width());
        }

        screenRect.setLeft(x);
        dataRect.setLeft(0);
        screenRect.setTop(screenRect.top() + screenRect.height());
        dataRect.setTop(dataRect.top() + dataRect.height());
    }
    
    showLegend(painter, this->width() - GRAPHIC_LABELS_MARGIN - GRAPHIC_LEGEND_WIDTH, y, GRAPHIC_LEGEND_WIDTH, height);
    showGraphicMarks(painter, x, y, width, height);
    
    painter.end();

    emit imageBuildFinished();
    update();
}


uint16_t GraphicWindow::getMedianDataValue(const QRect& dataRect) const
{
    auto retValue(0.0);
    auto numberOfElements(0);

    for (auto i(dataRect.left()); i < (dataRect.left() + dataRect.width()); ++i)
    {
        for (auto j(dataRect.top()); j < (dataRect.top() + dataRect.height()); ++j)
        {
            retValue += mData[mRequiredIndex][i][j];
            ++numberOfElements;
        }
    }
    
    return static_cast<uint16_t>(retValue / numberOfElements);
}

void GraphicWindow::onIndexChanged(int index)
{
    mRequiredIndex = index;
    repaint();
}

void GraphicWindow::onDataLoaded(QVector<QVector<QVector<int16_t>>> data)
{
    mData = data;
    repaint();
}

pair<int, double> GraphicWindow::getPointsNumberInInterval(const double ratio, const double remain)
{
    const auto sum(ratio + remain);
    auto pointsNumber(static_cast<int>(round(sum)));
    if ((pointsNumber - sum) > ratio)
    {
        --pointsNumber;
    }
    return make_pair(pointsNumber, ratio - pointsNumber);
}

GraphicWindow::TDataScreen GraphicWindow::getDataScreenPoints(const double ratio, const double remain)
{
    auto numberOfDataPoints(0);
    auto numberOfScreenPoints(0);
    auto currentRemain(remain);

    do
    {
        const auto [_numberOfPoints, _remain] = getPointsNumberInInterval(ratio, currentRemain);
        numberOfScreenPoints += _numberOfPoints;
        ++numberOfDataPoints;
        currentRemain += _remain;
    } while (0 == numberOfScreenPoints);
    
    return {numberOfDataPoints, numberOfScreenPoints, currentRemain};
}

QColor GraphicWindow::getColor(uint16_t value)
{
    if (value > MAX_VALUE_FOR_COLOR)
    {
        value = MAX_VALUE_FOR_COLOR;
    }

    const auto red((value > MAX_VALUE_FOR_COLOR_HALF) ? 
        (static_cast<int>(255.0 * value / MAX_VALUE_FOR_COLOR)) : 0);
    
    const auto blue((value < MAX_VALUE_FOR_COLOR_HALF) ? 
        (255 - static_cast<int>(255.0 * value / MAX_VALUE_FOR_COLOR)) : 0);
    
    const auto green((value < MAX_VALUE_FOR_COLOR_HALF) ? 
        (static_cast<int>(255.0 * value / MAX_VALUE_FOR_COLOR_HALF)) :
        (255 - static_cast<int>(255.0 * (value - MAX_VALUE_FOR_COLOR_HALF) / MAX_VALUE_FOR_COLOR_HALF)));

    return QColor(red, green, blue);
}

void GraphicWindow::showLegend(QPainter& painter,
                               const int x, const int y,
                               const int width, const int height) const
{
    const auto numberOfSubregions(GRAPHIC_LEGEND_MARK_NUMBER * 20);
    const auto heightOfRegion(height / (1.0 * numberOfSubregions));
    auto valuesRatio(1.0 * MAX_VALUE_FOR_COLOR / numberOfSubregions);

    // draw color legend
    auto currentRemain(0.0);
    auto currentValue(1.0 * MAX_VALUE_FOR_COLOR);
    const auto lowY(y + height);
    for (auto currentY(y); currentY < lowY;)
    {
        const auto [numberOfPoint, _remain] = getPointsNumberInInterval(heightOfRegion, currentRemain);

        painter.fillRect(x, currentY, width, numberOfPoint, getColor(currentValue));
        
        currentY += numberOfPoint;
        currentValue -= valuesRatio;
        currentRemain += _remain;
    }

    // draw marks to legend
    QFontMetrics fm(painter.fontMetrics());
    const auto marksGapHeight(1.0 * height / (GRAPHIC_LEGEND_MARK_NUMBER - 1));
    const auto marksXStart(x + width);
    const auto marksXEnd(x + width + MARK_SIZE);
    valuesRatio = 1.0 * MAX_VALUE_FOR_COLOR / (GRAPHIC_LEGEND_MARK_NUMBER - 1);
    currentValue = MAX_VALUE_FOR_COLOR;
    currentRemain = 0.0;
    for (auto currentY(y); currentY <= lowY;)
    {
        const auto [numberOfPoint, _remain] = getPointsNumberInInterval(marksGapHeight, currentRemain);
        if (lowY == currentY)
        {
            painter.drawLine(marksXStart, currentY - 1, marksXEnd, currentY - 1);
            painter.drawText(marksXEnd + 1, currentY - 1 + fm.ascent() / 2, QString::number(currentValue));
        }
        else
        {
            painter.drawLine(marksXStart, currentY, marksXEnd, currentY);
            painter.drawText(marksXEnd + 1, currentY -1 + fm.ascent() / 2, QString::number(currentValue));
        }
        
        currentY += numberOfPoint;
        currentRemain += _remain;
        currentValue = currentValue - valuesRatio;
    }
}

void GraphicWindow::showGraphicMarks(QPainter& painter, const int graphicX, const int graphicY,
    const int graphicWidth, const int graphicHeight) const
{
    QFontMetrics fm(painter.fontMetrics());
    
    // vertical axe labels
    const auto marksYGapHeight(1.0 * graphicHeight * GRAPHIC_Y_MARKS_MAX_DEPICTED_VALUE / GRAPHIC_Y_MAX_VALUE / 
        (GRAPHIC_Y_MARKS_NUMBER - 1));
    const auto YmarksStart(graphicX - MARK_SIZE);
    const auto YmarksEnd(graphicX);
    auto currentY(graphicY);
    auto valuesRatio(1.0 * GRAPHIC_Y_MARKS_MAX_DEPICTED_VALUE / (GRAPHIC_Y_MARKS_NUMBER - 1));
    auto currentRemain(0.0);
    QString markText;
    for (auto currentValue(0); currentValue <= GRAPHIC_Y_MARKS_MAX_DEPICTED_VALUE;)
    {
        const auto [numberOfPoint, _remain] = getPointsNumberInInterval(marksYGapHeight, currentRemain);

        painter.drawLine(YmarksStart, currentY, YmarksEnd, currentY);
        markText.setNum(currentValue);
        painter.drawText(YmarksStart - 1 - fm.boundingRect(markText).width(),
            currentY - 1 + fm.ascent() / 2, markText);

        currentY += numberOfPoint;
        currentRemain += _remain;
        currentValue += valuesRatio;
    }


    // horizontal axe marks
    const auto marksXGapWidth(1.0 * graphicWidth * GRAPHIC_X_MARKS_MAX_DEPICTED_VALUE / GRAPHIC_X_MAX_VALUE /
        (GRAPHIC_X_MARKS_NUMBER - 1));
    const auto XmarksTop(graphicY + graphicHeight);
    const auto XmarksBottom(XmarksTop + MARK_SIZE);
    auto currentX(graphicX);
    valuesRatio = 1.0 * GRAPHIC_X_MARKS_MAX_DEPICTED_VALUE / (GRAPHIC_X_MARKS_NUMBER - 1);
    currentRemain = 0.0;
    for (auto currentValue(0); currentValue <= GRAPHIC_X_MARKS_MAX_DEPICTED_VALUE;)
    {
        const auto [numberOfPoint, _remain] = getPointsNumberInInterval(marksXGapWidth, currentRemain);

        painter.drawLine(currentX, XmarksTop, currentX, XmarksBottom);
        markText.setNum(currentValue);
        painter.drawText(currentX - fm.boundingRect(markText).width() / 2, XmarksBottom + 1 + fm.ascent(), markText);

        currentX += numberOfPoint;
        currentRemain += _remain;
        currentValue += valuesRatio;
    }
}