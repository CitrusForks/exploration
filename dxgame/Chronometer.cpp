#include "StdAfx.h"
#include "Chronometer.h"


Chronometer::Chronometer(void) : m_prevTime(0), m_initTime(0), m_now(0)
{
    long long countsPerSec;

    QueryPerformanceFrequency( (LARGE_INTEGER*) &countsPerSec);

    m_secondsPerCount = 1 / (double)countsPerSec;

    QueryPerformanceCounter( (LARGE_INTEGER*) &m_now);
    m_initTime = m_prevTime = m_now;
    m_prevTime -= countsPerSec / 60; // fudge time for first frame
}

Chronometer::Chronometer(lua_State *)
{
    Chronometer c;
    *this = c;
}


Chronometer::~Chronometer(void)
{
}

double Chronometer::Sample()
{
    m_prevTime = m_now;
    QueryPerformanceCounter( (LARGE_INTEGER*) &m_now);

    return m_sincePrev = (m_now - m_prevTime) * m_secondsPerCount;
}

double Chronometer::sinceInit()
{
    return (m_now - m_initTime) * m_secondsPerCount;
}

double Chronometer::sincePrev()
{
    return m_sincePrev;
}


// Lua hooks and stuff follow
const char Chronometer::className[] = "Chronometer";

const Luna::LunaShare<Chronometer>::PropertyType Chronometer::properties[] = {{0,0}};

const Luna::LunaShare<Chronometer>::FunctionType Chronometer::methods[] =
{
    {
        "now", &Chronometer::l_now
    },
    {
        "sinceLast", &Chronometer::l_sincePrev
    },
    {
        "initTime", &Chronometer::l_initTime
    },
    {0,0}
};


int Chronometer::l_now( lua_State *L )
{
    lua_pushnumber(L, sinceInit());
    return 1;
}

int Chronometer::l_sincePrev( lua_State *L )
{
    lua_pushnumber(L, sincePrev());
    return 1;
}

int Chronometer::l_initTime( lua_State *L )
{
    lua_pushnumber(L, (lua_Number)m_initTime);
    return 1;
}

