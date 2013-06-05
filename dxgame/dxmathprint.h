#pragma once

#include <iostream>
#include <DirectXMath.h>

namespace DirectX
{
    // a couple of operators to print out DirectXMath matrix and vector variables

    std::ostream& operator << ( std::ostream& os, CXMMATRIX m);

    std::ostream& operator << ( std::ostream& os, FXMVECTOR v);

}
