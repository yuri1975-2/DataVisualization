#pragma once
#include "..\Parsing\IParsing.h"
#include "..\Parsing\ParsingImplementation.h"
#include <memory>

using namespace std;

class ApplicationComponents
{
private:
    inline static shared_ptr<IParsing> mParserImplementation;

public:
    static shared_ptr<IParsing> getParser()
    {
        if (nullptr == mParserImplementation)
        {
            mParserImplementation = make_shared<ParsingImplementation>();
        }

        return mParserImplementation;
    }
};