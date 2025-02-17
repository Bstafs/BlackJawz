struct PSInput
{
    float4 Position : SV_POSITION;
    float3 WorldPos : POSITION; 
    float3 Normal : NORMAL;
    float2 TexC : TEXCOORD0;
    float3 Tangent : TANGENT; 
    float3x3 TBN_MATRIX : TBN_MATRIX; 
};

Texture2D DiffuseTexture : register(t0);
Texture2D NormalTexture : register(t1);
Texture2D MetalTexture : register(t2);
Texture2D RoughnessTexture : register(t3);
Texture2D AOTexture : register(t4);
Texture2D DisplacementTexture : register(t5);

SamplerState samLinear : register(s0);

#define MAX_LIGHTS 10

struct LightProperties
{
    float4 LightPosition;
    float4 DiffuseLight;
    float4 AmbientLight;
    float4 SpecularLight;

    float SpecularPower;
    float Range;
    float2 Padding01;

    float3 LightDirection;
    float Intensity;

    float3 Attenuation;
    float Padding02;

    float SpotInnerCone;
    float SpotOuterCone;
    int LightType;
    float Padding03;
};

cbuffer LightsBuffer : register(b0)
{
    LightProperties lights[MAX_LIGHTS];
    
    int numLights;
    float3 CameraPosition;
}

struct GBufferOutput
{
    float4 Albedo : SV_Target0; // Albedo (RGB) + AO (A)
    float4 Normal : SV_Target1; // Normal (RGB) + Padding (A)
    float3 MetalRoughAO : SV_Target2; // Metalness (R), Roughness (G), AO (B)
    float4 Position : SV_Target3; // Metalness (R), Roughness (G), AO (B)
};

static const float minLayers = 8.0f;
static const float maxLayers = 64.0f;
static const float heightScale = 0.02f; 

float2 ParallaxOcclusionMapping(float2 texCoords, float3 viewDirTangent)
{
    float numLayers = lerp(maxLayers, minLayers, abs(dot(float3(0, 0, 1), viewDirTangent)));
    float layerDepth = 1.0 / numLayers;
    float currentLayerDepth = 0.0;
    
    float2 deltaTexCoords = viewDirTangent.xy * heightScale / viewDirTangent.z / numLayers;
    
    float2 currentTexCoords = texCoords;
    float currentHeight = DisplacementTexture.Sample(samLinear, currentTexCoords).r;
    
    float2 prevTexCoords = currentTexCoords;
    float prevLayerDepth = 0.0;

    // Fixed maximum iterations for unrolling (e.g., 64 iterations)
    [unroll(64)]
    for (int i = 0; i < 64; ++i)
    {
        // Break if we've reached our computed layer count
        if (i >= (int) numLayers)
            break;
        
        prevTexCoords = currentTexCoords;
        prevLayerDepth = currentLayerDepth;
        currentTexCoords -= deltaTexCoords;
        currentLayerDepth += layerDepth;
        currentHeight = DisplacementTexture.Sample(samLinear, currentTexCoords).r;
        if (currentLayerDepth > currentHeight)
            break;
    }
    
    const int numRefinementSteps = 5;
    float2 refinedTexCoords = currentTexCoords;
    float refinedLayerDepth = currentLayerDepth;
    
    [unroll(5)]
    for (int j = 0; j < numRefinementSteps; j++)
    {
        float2 midTexCoords = (refinedTexCoords + prevTexCoords) * 0.5;
        float midHeight = DisplacementTexture.Sample(samLinear, midTexCoords).r;
        float midLayerDepth = (refinedLayerDepth + prevLayerDepth) * 0.5;
        if (midLayerDepth > midHeight)
        {
            refinedTexCoords = midTexCoords;
            refinedLayerDepth = midLayerDepth;
        }
        else
        {
            prevTexCoords = midTexCoords;
            prevLayerDepth = midLayerDepth;
        }
    }
    
    return refinedTexCoords;
}

float ParallaxSelfShadow(float2 texCoords, float3 lightDirTangent)
{
    const int numShadowSamples = 32; // Adjust for performance/quality
    float shadow = 1.0; // Start fully lit

    float height = DisplacementTexture.Sample(samLinear, texCoords).r;

    float2 deltaTexCoords = lightDirTangent.xy * heightScale / lightDirTangent.z / numShadowSamples;
    float stepHeight = height / numShadowSamples;

    float2 sampleCoords = texCoords;
    float currentDepth = 0.0;

    [unroll(numShadowSamples)]
    for (int i = 0; i < numShadowSamples; ++i)
    {
        sampleCoords -= deltaTexCoords;
        currentDepth += stepHeight;

        float sampleHeight = DisplacementTexture.Sample(samLinear, sampleCoords).r;
        
        if (sampleHeight > currentDepth) // Occlusion detected
        {
            shadow *= 0.85; // Reduce light intensity
        }
    }

    return shadow;
}

GBufferOutput PS(PSInput input)
{
    GBufferOutput output;

    float3 V = normalize(CameraPosition - input.WorldPos);
    float3 viewDirTangent = normalize(mul(V, input.TBN_MATRIX));

   // float3 L = normalize(lights[0].LightPosition.xyz - input.WorldPos);
   // float3 lightDirTangent = normalize(mul(L, input.TBN_MATRIX));
    
   float2 newTexCoords = ParallaxOcclusionMapping(input.TexC, viewDirTangent);
   // float2 newTexCoords = input.TexC;
    
   // float shadowFactor = ParallaxSelfShadow(newTexCoords, lightDirTangent);
    
    float3 albedo = DiffuseTexture.Sample(samLinear, newTexCoords).rgb;
    float ao = AOTexture.Sample(samLinear, newTexCoords).r;
    output.Albedo = float4(albedo, ao);
    
    float3 sampledNormal = NormalTexture.Sample(samLinear, newTexCoords).xyz * 2.0f - 1.0f;
    float3 worldNormal = normalize(mul(sampledNormal, input.TBN_MATRIX));
    output.Normal = float4(worldNormal, 1.0f);
    
    float metallic = MetalTexture.Sample(samLinear, newTexCoords).r;
    float roughness = RoughnessTexture.Sample(samLinear, newTexCoords).r;

    output.MetalRoughAO = float3(metallic, roughness, ao);
    
    output.Position = float4(input.WorldPos, 1.0f);
    
    return output;
}
