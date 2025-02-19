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

float4 PS(PSInput input) : SV_TARGET
{
    float3 N = normalize(input.Pos.xyz);
    float3 up = float3(0.0, 1.0, 0.0);
    float3 right = normalize(cross(up, N));
    up = cross(N, right);

    const float TWO_PI = PI * 2.0;
    const float HALF_PI = PI * 0.5;

    // Define sample counts and compute deltas
    static const uint NUM_PHI_SAMPLES = 360;
    static const uint NUM_THETA_SAMPLES = 180;
    float deltaPhi = TWO_PI / float(NUM_PHI_SAMPLES);
    float deltaTheta = HALF_PI / float(NUM_THETA_SAMPLES);

    float3 color = float3(0.0, 0.0, 0.0);

    // Loop over phi and theta with integer indices
    for (uint i = 0; i < NUM_PHI_SAMPLES; ++i)
    {
        float phi = i * deltaPhi;
        for (uint j = 0; j < NUM_THETA_SAMPLES; ++j)
        {
            float theta = j * deltaTheta;

            // Compute sample direction
            float3 tempVec = cos(phi) * right + sin(phi) * up;
            float3 sampleVector = cos(theta) * N + sin(theta) * tempVec;

            // Accumulate with correct integration weight
            color += textureEnv.Sample(samplerEnv, sampleVector).rgb 
                     * cos(theta) * sin(theta) * deltaPhi * deltaTheta;
        }
    }

    return float4(color, 1.0);
}