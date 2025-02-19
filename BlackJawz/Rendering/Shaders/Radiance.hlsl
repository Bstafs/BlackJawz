TextureCube textureEnv : register(t0);
SamplerState samplerEnv : register(s0);

struct VSInput
{
    float3 Pos : POSITION;
    float2 TexC : TEXCOORD;
};

struct PSInput
{
    float4 Pos : SV_POSITION;
    float2 TexC : TEXCOORD;
};

PSInput VS(VSInput input)
{
    PSInput output;
    output.Pos = float4(input.Pos, 1.0);
    output.TexC = input.TexC;
    return output;
}

static const float PI = 3.14159265359;
static const float numSamples = 1024;

float2 hammersley2d(uint i, uint N)
{
    uint bits = (i << 16u) | (i >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    float rdi = float(bits) * 2.3283064365386963e-10;
    return float2(float(i) / float(N), rdi);
}

float3 importanceSample_GGX(float2 Xi, float roughness, float3 normal)
{
    float alpha = roughness * roughness;
    float phi = 2.0 * PI * Xi.x; // No random offset
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (alpha * alpha - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    float3 H = float3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);

    // Robust tangent basis
    float3 up = abs(normal.y) > 0.999 ? float3(0, 0, 1) : float3(0, 1, 0);
    float3 tangentX = normalize(cross(up, normal));
    float3 tangentY = cross(normal, tangentX);

    return normalize(tangentX * H.x + tangentY * H.y + normal * H.z);
}

// Normal Distribution function
float D_GGX(float dotNH, float roughness)
{
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float denom = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
    return (alpha2) / (PI * denom * denom);
}

float3 prefilterEnvMap(float3 R, float roughness)
{
    float3 N = R;
    float3 V = R;
    float3 color = 0;
    float totalWeight = 0;
    int2 envMapDims;
    textureEnv.GetDimensions(envMapDims.x, envMapDims.y);
    float envMapDim = float(envMapDims.x);

    for (uint i = 0; i < numSamples; i++)
    {
        float2 Xi = hammersley2d(i, numSamples);
        float3 H = importanceSample_GGX(Xi, roughness, N);
        float3 L = 2.0 * dot(V, H) * H - V;
        float dotNL = saturate(dot(N, L));

        if (dotNL > 0)
        {
            float dotNH = saturate(dot(N, H));
            float dotVH = saturate(dot(V, H));

            // PDF and mip calculation
            float pdf = D_GGX(dotNH, roughness) * dotNH / (4.0 * dotVH) + 0.0001;
            float omegaS = 1.0 / (numSamples * pdf);
            float omegaP = 4.0 * PI / (6.0 * envMapDim * envMapDim);
            float mipLevel = (roughness == 0.0) ? 0.0 : max(0.5 * log2(omegaS / omegaP), 0.0);

            color += textureEnv.SampleLevel(samplerEnv, L, mipLevel).rgb * dotNL;
            totalWeight += dotNL;
        }
    }
    return color / max(totalWeight, 0.001);
}

float4 PS(PSInput input) : SV_TARGET
{   
    float roughness = 6.0f;
    
    float3 N = normalize(input.Pos.xyz);
    return float4(prefilterEnvMap(N, roughness), 1.0);
}