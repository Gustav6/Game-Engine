#ifndef Sampler_HLSLI
#define Sampler_HLSLI

SamplerState TrilinearWrap : register(s0);
SamplerComparisonState ShadowCmpSampler : register(s1);
SamplerComparisonState LutSampler : register(s2);

#endif