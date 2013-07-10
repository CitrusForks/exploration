#pragma once
#include <stdlib.h>
#include "Errors.h"

// this is a mix-in containing aligning overloads for operator new and operator delete

__declspec (align(16)) class SSE2Aligned
{
public:
    inline SSE2Aligned(void)
    {
    }

    inline ~SSE2Aligned(void)
    {

    }

    void *operator new(size_t size)
    {
        return _aligned_malloc(size, 16);
    }

    void operator delete(void *p)
    {
        _aligned_free(p);
    }
};

