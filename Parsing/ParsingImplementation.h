#pragma once

#include "IParsing.h"
#include <string>

using namespace std;

class ParsingImplementation : public IParsing
{
private:
    const static int MAX_TX = 64;
    const static int MAX_RX = 64;
    const static int MAX_SAMPLES = 8192;
    const static int VALUE_SIZE = sizeof(int16_t);
    // assume file contains 2 cube chunks of MAX_TX * MAX_RX * MAX_SAMPLES of int16_t values
    constexpr static int CORRECT_FILE_SIZE = (2 * MAX_TX * MAX_RX * MAX_SAMPLES * VALUE_SIZE);

    inline const static char* INVALID_FILE_NAME = "Couldn't open file: ";
    inline const static string INVALID_FILE_SIZE = "Invalid file size. It should be: " + to_string(CORRECT_FILE_SIZE) + " bytes";

    static uint16_t transformation(const int16_t first, const int16_t second);

public:
    QVector<QVector<QVector<int16_t>>> parse(const char* name) override;

    ~ParsingImplementation() {}
};