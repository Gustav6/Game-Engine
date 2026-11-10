#include "Include/Core/Common.hlsli"

uint4 main(VStoPS aPixel) : SV_TARGET
{ 
    return uint4(OB_ObjectId, 0, 0, 0);
}