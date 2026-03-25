#include "ParsingImplementation.h"
#include <fstream>
#include <filesystem>
#include <exception>
#include <cmath>
#include <QDebug>

using namespace std;

uint16_t ParsingImplementation::transformation(const int16_t first, const int16_t second)
{
    return static_cast<uint16_t>(sqrt(pow(first, 2) + pow(second, 2)));
}

QVector<QVector<QVector<int16_t>>> ParsingImplementation::parse(const char* name)
{
    if (CORRECT_FILE_SIZE != filesystem::file_size(name))
    {
        throw length_error(INVALID_FILE_SIZE);
    }

    ifstream firstChunk(name, ios::binary);
    ifstream secondChunk(name, ios::binary);
    if ((!firstChunk.is_open()) || (!secondChunk.is_open()))
    {
        throw invalid_argument(INVALID_FILE_NAME + string(name));
    }
    secondChunk.seekg(CORRECT_FILE_SIZE / 2);

    QVector<QVector<QVector<int16_t>>> retValue(MAX_TX, QVector<QVector<int16_t>>(MAX_RX, QVector<int16_t>(MAX_SAMPLES, 0)));

    for (auto i(0); i < MAX_TX; ++i)
    {
        for (auto j(0); j < MAX_RX; ++j)
        {
            for (auto k(0); k < MAX_SAMPLES; ++k)
            {
                int16_t value1(0), value2(0);
                firstChunk.read(reinterpret_cast<char*>(&value1), VALUE_SIZE);
                secondChunk.read(reinterpret_cast<char*>(&value2), VALUE_SIZE);
                retValue[i][j][k] = transformation(value1, value2);
            }
        }
    }

    return retValue;
}
