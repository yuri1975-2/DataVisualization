GIVEN:

Data file (7-zipped data.7z) containded 2 chunks of 3D matrix (set of 2D matrix contained some abstract values).

NEEDS TO BE IMPLEMENTED:

Parse data file and form resulting data: 3D matrix contained set of 2D matrix with resulting values calculated from original chunks as:

            value[k, i, j] = sqrt(pow(value_chunk1[k, i, j], 2) + pow(value_chunk2[k, j, j], 2))
            
and represents as uint16_t.

After, get 2D matrix based upon index k and build color 2D-graphic of her values.

IMPLEMENTATION:

IParsing: data file parsing interface declaration;

ParsingImplementation.h, ParsingImplementation.cpp: implementation of above interface;

ApplicationComponents.h: class factory, returns IParsing implementation;

MainWindow.ui, MainWindow.h, MainWindow.cpp-window for data file load and 2D matrix choice;

GraphicWindow.ui, GraphicWindow.h, GraphicWindow.cpp-window to build color 2D-graphic.

SOME COMMENTS TO IMPLEMENTATION:

Since, in general, number of data points doesn't corresponds to number at graphic, we use mapping between data and graphic rects (possibly, with uninteger
size length).

The median value of points in data rect is treated as value of this one.

The color of graphic rect based upon data rect value is [RGB], where red component icreased from 0 to 255 for data value > (max value) / 2;
green component icreased from 0 to 255 for 0 <= data value < (max value) / 2 and decreased from 255 to 0 for (max value) / 2 <= data value <= max value;
blue component decreased from 255 to 0 for 0 <= data value < (max value) / 2.

To avoid repeating potentintially massive graphic build operation, I do it at buffer (QPixmap) which is depicted after at canvas at GraphicWindow::paintEvent() (which could repeatedly called, for instance, due windows switch).

Parsed data sent (by signals) by value not reference due "implicit sharing" Qt mechanism.
