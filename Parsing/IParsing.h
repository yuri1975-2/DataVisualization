#pragma once

#include <QVector>

class IParsing
{
public:
    virtual QVector<QVector<QVector<int16_t>>> parse(const char* name) = 0;
    
    virtual ~IParsing() {};
};