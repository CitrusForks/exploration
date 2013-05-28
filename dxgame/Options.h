#pragma once

#include <unordered_map>

namespace Options
{
    extern unordered_map<string, int> intOptions;
    extern unordered_map<string, string> stringOptions;

    void setDefaults();
}
