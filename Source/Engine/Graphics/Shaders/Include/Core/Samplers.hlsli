#ifndef Sampler_HLSLI
#define Sampler_HLSLI

SamplerState TrilinearWrap : register(s0);
SamplerComparisonState ShadowCmpSampler : register(s1);
SamplerState LUTSampler : register(s2);
SamplerState TrilinearClamp : register(s3);
SamplerState LinearClamp : register(s4);
SamplerState PointClamp : register(s0);

#endif