// Pick one of these at a time. Keep in mind that ACES and Lottes need to be
// corrected after output but no the Unreal one!

// Source : https://github.com/TheRealMJP/BakingLab/blob/master/BakingLab/ACES.hlsl
// Source : https://github.com/dmnsgn/shaders-tone-map/tree/main

float3 LinearToGamma(float3 aColor)
{
    return pow(abs(aColor), 1.0 / 2.2);
}

static const float3x3 ACESInputMat =
{
    { 0.59719, 0.35458, 0.04823 },
    { 0.07600, 0.90834, 0.01566 },
    { 0.02840, 0.13383, 0.83777 }
};

// ODT_SAT => XYZ => D60_2_D65 => sRGB
static const float3x3 ACESOutputMat =
{
    { 1.60475, -0.53108, -0.07367 },
    { -0.10208, 1.10813, -0.00605 },
    { -0.00327, -0.07276, 1.07602 }
};

float3 RRTAndODTFit(float3 v)
{
    float3 a = v * (v + 0.0245786f) - 0.000090537f;
    float3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
    return a / b;
}

float3 Tonemap_ACES(float3 aColor)
{
    aColor = mul(ACESInputMat, aColor);

    // Apply RRT and ODT
    aColor = RRTAndODTFit(aColor);

    aColor = mul(ACESOutputMat, aColor);

    // Clamp to [0, 1]
    aColor = saturate(aColor);

    return aColor;
}

float3 Tonemap_Lottes(float3 aColor)
{
	// Lottes 2016, "Advanced Techniques and Optimization of HDR Color Pipelines"
	const float a = 1.6;
	const float d = 0.977;
	const float hdrMax = 8.0;
	const float midIn = 0.18;
	const float midOut = 0.267;

	// Can be precomputed
	const float b =
		(-pow(midIn, a) + pow(hdrMax, a) * midOut) /
		((pow(hdrMax, a * d) - pow(midIn, a * d)) * midOut);
	const float c =
		(pow(hdrMax, a * d) * pow(midIn, a) - pow(hdrMax, a) * pow(midIn, a * d) * midOut) /
		((pow(hdrMax, a * d) - pow(midIn, a * d)) * midOut);

	return pow(aColor, a) / (pow(aColor, a * d) * b + c);
}

float3 Tonemap_UnrealEngine(float3 aColor)
{
    // Unreal 3, Documentation: "Color Grading"
	// Adapted to be close to Tonemap_ACES, with similar range
	// Gamma 2.2 correction is baked in, don't use with sRGB conversion!
    return aColor / (aColor + 0.155) * 1.019;
}

float3 Tonemap_Neutral(float3 color)
{
    const float startCompression = 0.8 - 0.04;
    const float desaturation = 0.15;

    float x = min(color.r, min(color.g, color.b));
    float offset = x < 0.08 ? x - 6.25 * x * x : 0.04;
    color -= offset;

    float peak = max(color.r, max(color.g, color.b));
    if (peak < startCompression)
        return color;

    const float d = 1.0 - startCompression;
    float newPeak = 1.0 - d * d / (peak + d - startCompression);
    color *= newPeak / peak;

    float g = 1.0 - 1.0 / (desaturation * (peak - newPeak) + 1.0);
    return lerp(color, float3(newPeak, newPeak, newPeak), g);
}

float3 Tonemap_filmic(float3 x)
{
    float3 X = max(float3(0, 0, 0), x - 0.004);
    float3 result = (X * (6.2 * X + 0.5)) / (X * (6.2 * X + 1.7) + 0.06);
    return pow(result, 2.2);
}

float filmic(float x)
{
    float X = max(0.0, x - 0.004);
    float result = (X * (6.2 * X + 0.5)) / (X * (6.2 * X + 1.7) + 0.06);
    return pow(result, 2.2);
}

float3 uchimura(float3 x, float P, float a, float m, float l, float c, float b)
{
    x = max(x, 0.0);

    float l0 = ((P - m) * l) / a;
    float S0 = m + l0;
    float S1 = m + a * l0;
    float C2 = (a * P) / (P - S1);
    float CP = -C2 / P;

    float3 w0 = 1.0 - smoothstep(float3(0.0, 0.0, 0.0), float3(m, m, m), x);
    float3 w2 = step(float3(S0, S0, S0), x);
    float3 w1 = 1.0 - w0 - w2;

    float3 T = m * pow(x / m, float3(c, c, c)) + b;
    float3 S = P - (P - S1) * exp(CP * (x - S0));
    float3 L = m + a * (x - m);

    return T * w0 + L * w1 + S * w2;
}

float3 Tonemap_Uchimura(float3 x)
{
    const float P = 1.0; // max display brightness
    const float a = 1.0; // contrast
    const float m = 0.22; // linear section start
    const float l = 0.4; // linear section length
    const float c = 1.33; // black
    const float b = 0.0; // pedestal

    return uchimura(x, P, a, m, l, c, b);
}

float uchimura(float x, float P, float a, float m, float l, float c, float b)
{
    float l0 = ((P - m) * l) / a;
    float L0 = m - m / a;
    float L1 = m + (1.0 - m) / a;
    float S0 = m + l0;
    float S1 = m + a * l0;
    float C2 = (a * P) / (P - S1);
    float CP = -C2 / P;

    float w0 = 1.0 - smoothstep(0.0, m, x);
    float w2 = step(m + l0, x);
    float w1 = 1.0 - w0 - w2;

    float T = m * pow(x / m, c) + b;
    float S = P - (P - S1) * exp(CP * (x - S0));
    float L = m + a * (x - m);

    return T * w0 + L * w1 + S * w2;
}

float uchimura(float x)
{
    const float P = 1.0; // max display brightness
    const float a = 1.0; // contrast
    const float m = 0.22; // linear section start
    const float l = 0.4; // linear section length
    const float c = 1.33; // black
    const float b = 0.0; // pedestal

    return uchimura(x, P, a, m, l, c, b);
}

float3 uncharted2Tonemap(float3 x)
{
    float A = 0.15;
    float B = 0.50;
    float C = 0.10;
    float D = 0.20;
    float E = 0.02;
    float F = 0.30;
    float W = 11.2;
    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

float3 Tonemap_Uncharted2(float3 color)
{
    const float W = 11.2;
    float exposureBias = 2.0;
    float3 curr = uncharted2Tonemap(exposureBias * color);
    float3 whiteScale = 1.0 / uncharted2Tonemap(float3(W, W, W));
    return curr * whiteScale;
}

float uncharted2Tonemap(float x)
{
    float A = 0.15;
    float B = 0.50;
    float C = 0.10;
    float D = 0.20;
    float E = 0.02;
    float F = 0.30;
    float W = 11.2;
    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

float uncharted2(float color)
{
    const float W = 11.2;
    const float exposureBias = 2.0;
    float curr = uncharted2Tonemap(exposureBias * color);
    float whiteScale = 1.0 / uncharted2Tonemap(W);
    return curr * whiteScale;
}

static const float3x3 LINEAR_REC2020_TO_LINEAR_SRGB = float3x3(
     1.6605, -0.5876, -0.0728,
    -0.1246, 1.1329, -0.0083,
    -0.0182, -0.1006, 1.1187
);

static const float3x3 LINEAR_SRGB_TO_LINEAR_REC2020 = float3x3(
    0.6274, 0.3293, 0.0433,
    0.0691, 0.9195, 0.0113,
    0.0164, 0.0880, 0.8956
);

static const float3x3 AgXInsetMatrix = float3x3(
    0.856627153315983, 0.0951212405381588, 0.0482516061458583,
    0.137318972929847, 0.761241990602591, 0.101439036467562,
    0.11189821299995, 0.0767994186031903, 0.811302368396859
);

static const float3x3 AgXOutsetMatrix = float3x3(
     1.1271005818144368, -0.11060664309660323, -0.016493938717834573,
    -0.1413297634984383, 1.157823702216272, -0.016493938717834257,
    -0.14132976349843826, -0.11060664309660294, 1.2519364065950405
);

static const float AgxMinEv = -12.47393;
static const float AgxMaxEv = 4.026069;

float3 agxCdl(float3 color, float3 slope, float3 offset, float3 power, float saturation)
{
    color = mul(LINEAR_SRGB_TO_LINEAR_REC2020, color);
    
    color = mul(AgXInsetMatrix, color);

    color = max(color, 1e-10);
    
    color = clamp(log2(color), AgxMinEv, AgxMaxEv);
    color = (color - AgxMinEv) / (AgxMaxEv - AgxMinEv);

    color = clamp(color, 0.0, 1.0);
    
    float3 x2 = color * color;
    float3 x4 = x2 * x2;
    color = +15.5 * x4 * x2
          - 40.14 * x4 * color
          + 31.96 * x4
          - 6.868 * x2 * color
          + 0.4298 * x2
          + 0.1191 * color
          - 0.00232;
    
    color = pow(max(color * slope + offset, 0.0), power);
    const float3 lw = float3(0.2126, 0.7152, 0.0722);
    float luma = dot(color, lw);
    color = luma + saturation * (color - luma);
    
    color = mul(AgXOutsetMatrix, color);
    
    color = pow(max(0.0, color), 2.2);
    
    color = mul(LINEAR_REC2020_TO_LINEAR_SRGB, color);

    color = clamp(color, 0.0, 1.0);

    return color;
}

float3 Tonemap_AGX(float3 color)
{
    return agxCdl(
        color,
        float3(1.0, 1.0, 1.0),
        float3(0.0, 0.0, 0.0),
        float3(1.0, 1.0, 1.0),
        1.0
    );
}

float3 Tonemap_AGX_Golden(float3 color)
{
    return agxCdl(
        color,
        float3(1.0, 0.9, 0.5),
        float3(0.0, 0.0, 0.0),
        float3(0.8, 0.8, 0.8),
        1.3
    );
}

float3 Tonemap_AGX_Punchy(float3 color)
{
    return agxCdl(
        color,
        float3(1.0, 1.0, 1.0),
        float3(0.0, 0.0, 0.0),
        float3(1.35, 1.35, 1.35),
        1.4
    );
}

float3 Tonemap_FilmicHejl(float3 hdr, float whitePt)
{
    float4 vh = float4(hdr, whitePt);
    float4 va = (1.425 * vh) + 0.05; // eval filmic curve
    float4 vf = ((vh * va + 0.004) / ((vh * (va + 0.55) + 0.0491))) - 0.0821;
    return saturate(vf.rgb / vf.www); // white point correction
}