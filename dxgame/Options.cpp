#include "StdAfx.h"
#include "Options.h"

using namespace std;

namespace Options
{
    unordered_map<string, int> intOptions;
    unordered_map<string, string> stringOptions;

    void setDefaults()
    {
        intOptions["MSAACount"] = 8;
        intOptions["MSAAQuality"] = 32; // get the max value from the API
        intOptions["Width"] = 1024;
        intOptions["Height"] = 768;
        intOptions["FullScreen"] = 0; // because I'm too lazy to have a separate table of bool options
        intOptions["MouseSensitivy"] = 8; // sensitivity multiplier = this option / 32.0
    }
}