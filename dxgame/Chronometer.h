#ifndef CHRONOMETER_H_BECAUSE_TIMER_IS_A_RESERVED_CLASS_NAME_WTH
#define CHRONOMETER_H_BECAUSE_TIMER_IS_A_RESERVED_CLASS_NAME_WTH

#include "stdafx.h"
#include <lua.hpp>
#include "LunaShare.hpp"

class Chronometer
{
public:

    static const char className[];
    static const Luna::LunaShare<Chronometer>::PropertyType properties[];
    static const Luna::LunaShare<Chronometer>::FunctionType methods[];

    Chronometer(void);
    Chronometer(lua_State *);
    ~Chronometer(void);
    double Sample();
    double sinceInit();
    double sincePrev();

    int l_now(lua_State *L);
    int l_sincePrev(lua_State *L);
    int l_initTime(lua_State *L); // useful for seeding an RNG perhaps

    double m_secondsPerCount;
    double m_sincePrev;

    long long m_initTime;
    long long m_prevTime;
    long long m_now;
};

#endif