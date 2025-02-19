struct VSInput
{
    float2 Pos : POSITION;
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
    output.Pos = float4(input.Pos * 2.0 - 1.0, 0.0, 1.0); // NDC
    output.TexC = input.TexC;
    return output;
}

static const float PI = 3.14159265359;
static const uint NUM_SAMPLES = 1024;

// Proper Hammersley sequence for low-discrepancy sampling
float2 hammersley2d(uint i, uint N)
{
    uint bits = (i << 16u) | (i >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    float rdi = float(bits) * 2.3283064365386963e-10; // 1/2^32
    return float2(float(i) / float(N), rdi);
}

// GGX importance sampling without random jitter
float3 importanceSample_GGX(float2 Xi, float roughness, float3 N)
{
    float alpha = roughness * roughness;
    float phi = 2.0 * PI * Xi.x; // Remove random noise
    
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (alpha * alpha - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    
    // Spherical to Cartesian
    float3 H = float3(
        sinTheta * cos(phi),
        sinTheta * sin(phi),
        cosTheta
    );
    
    // Tangent-to-world space (N is always [0,0,1] for BRDF LUT)
    float3 up = abs(N.z) < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0);
    float3 T = normalize(cross(up, N));
    float3 B = cross(N, T);
    return normalize(T * H.x + B * H.y + N * H.z);
}

// Match geometry function with your main PBR shader
float G_SchlickSmithGGX(float dotNL, float dotNV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0; // Align with main shader's G calculation
    float GL = dotNL / (dotNL * (1.0 - k) + k);
    float GV = dotNV / (dotNV * (1.0 - k) + k);
    return GL * GV;
}

float2 BRDF(float NoV, float roughness)
{
    float3 N = float3(0, 0, 1); // Z-up for tangent space
    float3 V = float3(sqrt(1.0 - NoV * NoV), 0, NoV);
    
    float2 lut = float2(0, 0);
    for (uint i = 0; i < NUM_SAMPLES; i++)
    {
        float2 Xi = hammersley2d(i, NUM_SAMPLES);
        float3 H = importanceSample_GGX(Xi, roughness, N);
        float3 L = 2.0 * dot(V, H) * H - V;
        
        float dotNL = saturate(dot(N, L));
        float dotNV = saturate(dot(N, V));
        float dotVH = saturate(dot(V, H));
        float dotNH = saturate(dot(N, H));
        
        if (dotNL > 0)
        {
            float G = G_SchlickSmithGGX(dotNL, dotNV, roughness);
            float G_Vis = (G * dotVH) / (dotNH * dotNV + 1e-5);
            float Fc = pow(1.0 - dotVH, 5.0);
            lut += float2((1.0 - Fc) * G_Vis, Fc * G_Vis);
        }
    }
    return lut / float(NUM_SAMPLES);
}

float4 PS(PSInput input) : SV_TARGET
{
    float2 brdf = BRDF(input.TexC.x, input.TexC.y);
    return float4(brdf.x, brdf.y, 0.0, 1.0); // R = scale, G = bias
}
