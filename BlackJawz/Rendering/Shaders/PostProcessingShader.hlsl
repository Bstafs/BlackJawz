SamplerState samLinear : register(s0);
Texture2D lightingTexture : register(t0); // Lighting pass result
Texture2D skyBoxTexture : register(t1); // Sky Box Pass Result
Texture2D depthTexture : register(t2); // Depth Pass Result

cbuffer PostProcessingBuffer : register(b0)
{
    float2 ScreenSize; 
    float2 Padding01;
}

struct VSInput
{
    float3 Position : POSITION;
    float2 TexC : TEXCOORD0;
};

struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexC : TEXCOORD0;
};

PSInput VS(VSInput input)
{
    PSInput output;
    output.Position = float4(input.Position, 1.0f);
    output.TexC = input.TexC;
    return output;
}

float4 PS(PSInput input) : SV_TARGET
{
    // Get the scene depth at this pixel
    float sceneDepth = depthTexture.Sample(samLinear, input.TexC).r;

    // Sample the deferred lighting result
    float3 sceneColor = lightingTexture.Sample(samLinear, input.TexC).rgb;

    // Sample the skybox.
    float3 skyColor = skyBoxTexture.Sample(samLinear, input.TexC).rgb;
    
    // Use skybox color where there is no geometry (depth is 1.0) 
    float3 finalColor = (sceneDepth >= 1.0f) ? skyColor : sceneColor;
    return float4(finalColor, 1.0);
}
