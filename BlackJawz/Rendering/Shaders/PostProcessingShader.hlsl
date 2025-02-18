SamplerState samLinear : register(s0);
Texture2D txDiffuse : register(t0);

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

float3 FXAntiAliasing(float2 TexC)
{
    float2 texelSize = 1.0 / ScreenSize; // Size of one pixel

    // Sample surrounding pixels
    float3 colorCenter = txDiffuse.Sample(samLinear, TexC).rgb;
    float3 colorUp = txDiffuse.Sample(samLinear, TexC + float2(0, texelSize.y)).rgb;
    float3 colorDown = txDiffuse.Sample(samLinear, TexC - float2(0, texelSize.y)).rgb;
    float3 colorLeft = txDiffuse.Sample(samLinear, TexC - float2(texelSize.x, 0)).rgb;
    float3 colorRight = txDiffuse.Sample(samLinear,TexC + float2(texelSize.x, 0)).rgb;

    // Compute edge detection (Luminance-based)
    float lumCenter = dot(colorCenter, float3(0.299, 0.587, 0.114));
    float lumUp = dot(colorUp, float3(0.299, 0.587, 0.114));
    float lumDown = dot(colorDown, float3(0.299, 0.587, 0.114));
    float lumLeft = dot(colorLeft, float3(0.299, 0.587, 0.114));
    float lumRight = dot(colorRight, float3(0.299, 0.587, 0.114));

    float edgeFactor = abs(lumLeft + lumRight - 2 * lumCenter) + abs(lumUp + lumDown - 2 * lumCenter);
    
    // Smooth blending
    float blendAmount = saturate(edgeFactor * 8.0); 
    float3 smoothColor = (colorLeft + colorRight + colorUp + colorDown) * 0.25;

    // Blend between original and smoothed color
    float3 finalColor = lerp(colorCenter, smoothColor, blendAmount);
    
    return finalColor;
}

float4 PS(PSInput input) : SV_TARGET
{
 // float3 color = txDiffuse.Sample(samLinear, input.TexC);
    float3 color = FXAntiAliasing(input.TexC);
    
    return float4(color, 1.0f);
}
