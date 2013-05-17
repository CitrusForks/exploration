#ifndef CHRONOMETER_H_BECAUSE_TIMER_IS_A_RESERVED_CLASS_NAME_WTH
#define CHRONOMETER_H_BECAUSE_TIMER_IS_A_RESERVED_CLASS_NAME_WTH

#include "stdafx.h"

class Chronometer
{
public:
    Chronometer(void);
    ~Chronometer(void);
    double Sample();
    double sinceInit();
    double sincePrev();
    double m_secondsPerCount;
    double m_sincePrev;

    long long m_initTime;
    long long m_prevTime;
    long long m_now;
};

#endif