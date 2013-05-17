#include "StdAfx.h"
#include "Chronometer.h"


Chronometer::Chronometer(void) : m_prevTime(0), m_initTime(0), m_now(0)
{
    long long countsPerSec;

    QueryPerformanceFrequency( (LARGE_INTEGER*) &countsPerSec);

    m_secondsPerCount = 1 / (double)countsPerSec;

    QueryPerformanceCounter( (LARGE_INTEGER*) &m_now);
    m_initTime = m_prevTime = m_now;
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

