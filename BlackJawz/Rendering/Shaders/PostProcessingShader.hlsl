SamplerState samLinear : register(s0);
Texture2D lightingTexture : register(t0); // Lighting pass result
Texture2D skyBoxTexture : register(t1); // Sky Box Pass Result
Texture2D depthTexture : register(t2); // Depth Pass Result

cbuffer PostProcessingBuffer : register(b0)
{
    float2 ScreenSize; 
    float2 RcpScreenSize; 
    float ContrastThreshold;
    float RelativeThreshold;
    float SubpixelBlending; 
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

float4 GaussianBlur(Texture2D tex, float2 uv, float2 direction)
{
    static const float weights[5] = { 0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216 };
    static const float offsets[5] = { 0.0, 1.0, 2.0, 3.0, 4.0 };

    float4 color = tex.Sample(samLinear, uv) * weights[0];

    for (int i = 1; i < 5; i++)
    {
        float2 offset = direction * (offsets[i] / ScreenSize);
        color += tex.Sample(samLinear, uv + offset) * weights[i];
        color += tex.Sample(samLinear, uv - offset) * weights[i];
    }

    return color;
}

float GetLuminance(float3 color)
{
    return dot(color, float3(0.2126, 0.7152, 0.0722));
}

float3 FXAA(float2 TexC)
{
    // Sample the current pixel and its neighbors
    float3 colorCenter = lightingTexture.Sample(samLinear, TexC).rgb;
    float3 colorN = lightingTexture.Sample(samLinear, TexC + float2(0, RcpScreenSize.y)).rgb;
    float3 colorS = lightingTexture.Sample(samLinear, TexC - float2(0, RcpScreenSize.y)).rgb;
    float3 colorE = lightingTexture.Sample(samLinear, TexC + float2(RcpScreenSize.x, 0)).rgb;
    float3 colorW = lightingTexture.Sample(samLinear, TexC - float2(RcpScreenSize.x, 0)).rgb;

    // Calculate luminance for each sample
    float lumaCenter = GetLuminance(colorCenter);
    float lumaN = GetLuminance(colorN);
    float lumaS = GetLuminance(colorS);
    float lumaE = GetLuminance(colorE);
    float lumaW = GetLuminance(colorW);

    // Find max and min luminance around the current pixel
    float lumaMin = min(lumaCenter, min(min(lumaN, lumaS), min(lumaE, lumaW)));
    float lumaMax = max(lumaCenter, max(max(lumaN, lumaS), max(lumaE, lumaW)));

    // Edge detection threshold
    float lumaRange = lumaMax - lumaMin;
    if (lumaRange < max(ContrastThreshold, lumaMax * RelativeThreshold))
    {
        return colorCenter; // Not an edge; return original color
    }

    // --- Edge Direction ---
    float edgeHorizontal = abs(lumaN + lumaS - 2.0 * lumaCenter);
    float edgeVertical = abs(lumaE + lumaW - 2.0 * lumaCenter);
    bool isHorizontal = edgeHorizontal >= edgeVertical;

    // --- Edge Blending ---
    float2 uvOffset = isHorizontal ? float2(0.0, RcpScreenSize.y) : float2(RcpScreenSize.x, 0.0);
    float3 color1 = isHorizontal ? colorN : colorE;
    float3 color2 = isHorizontal ? colorS : colorW;

    float luma1 = GetLuminance(color1);
    float luma2 = GetLuminance(color2);

    // Blend along the edge direction
    float blendStrength = 1.0 / (abs(luma1 + luma2 - 2.0 * lumaCenter) + 1e-4);
    float blendFactor = clamp(blendStrength * SubpixelBlending, 0.0, 1.0);

    // Final blended color
    return lerp(colorCenter, 0.5 * (color1 + color2), blendFactor);
}

float4 PS(PSInput input) : SV_TARGET
{
    // Get the scene depth at this pixel
    float sceneDepth = depthTexture.Sample(samLinear, input.TexC).r;
  
    // Sample the deferred lighting result
    float3 sceneColor = lightingTexture.Sample(samLinear, input.TexC).rgb;

    // Apply Gaussian blur (two passes)
    float4 bloomH = GaussianBlur(lightingTexture, input.TexC, float2(1, 0)); // Horizontal
    float4 bloomV = GaussianBlur(lightingTexture, input.TexC, float2(0, 1)); // Vertical
    
    // Sample the skybox.
    float3 skyColor = skyBoxTexture.Sample(samLinear, input.TexC).rgb;
    
    float3 fxaa = FXAA(input.TexC);
    
    // Use skybox color where there is no geometry (depth is 1.0) 
    float3 finalColor = (sceneDepth >= 1.0f) ? skyColor : sceneColor;
    return float4(finalColor, 1.0);
}
