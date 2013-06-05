#include "stdafx.h"
#include <iostream>
#include <DirectXMath.h>

namespace DirectX
{
    // a couple of operators to print out DirectXMath matrix and vector variables

    std::ostream& operator << ( std::ostream& os, CXMMATRIX m) 
    { 
        XMFLOAT4X4 mm;
        XMStoreFloat4x4(&mm, m);

        for( int i = 0; i < 4; ++ i) 
        { 
            for( int j = 0; j < 4; ++ j) os << mm.m[i][j] << "\t"; 
            os << std::endl; 
        } 
        return os; 
    }

    std::ostream& operator << ( std::ostream& os, FXMVECTOR v)
    {
        XMFLOAT4 vv;
        XMStoreFloat4(&vv, v);
        return os << vv.x << "\t" << vv.y << "\t" << vv.z << "\t" << vv.w << endl;
    }

}
