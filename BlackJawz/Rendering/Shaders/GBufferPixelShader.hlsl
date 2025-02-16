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
static const float maxLayers = 128.0f;
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
    
    // Binary search refinement: refine between prevTexCoords and currentTexCoords
    const int numRefinementSteps = 5;
    float2 refinedTexCoords = currentTexCoords;
    float refinedLayerDepth = currentLayerDepth;
    for (int i = 0; i < numRefinementSteps; i++)
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

GBufferOutput PS(PSInput input)
{
    GBufferOutput output;

    // Convert view vector to tangent space
    // Assume CameraPosition is provided via a cbuffer
    float3 V = normalize(CameraPosition - input.WorldPos);
    float3 viewDirTangent = normalize(mul(V, input.TBN_MATRIX));

    // Adjust texture coordinates using POM
    float2 newTexCoords = ParallaxOcclusionMapping(input.TexC, viewDirTangent);
    
    // Sample Albedo and store AO in alpha channel
    float3 albedo = DiffuseTexture.Sample(samLinear, newTexCoords).rgb;
    float ao = AOTexture.Sample(samLinear, newTexCoords).r;
    output.Albedo = float4(albedo, ao);
    
    // Sample and transform Normal (recompute if desired, or use displaced normals if provided)
    float3 sampledNormal = NormalTexture.Sample(samLinear, newTexCoords).xyz * 2.0f - 1.0f;
    float3 worldNormal = normalize(mul(sampledNormal, input.TBN_MATRIX));
    output.Normal = float4(worldNormal, 1.0f);
    
    // Sample material properties from their respective textures (or pack them together)
    float metallic = MetalTexture.Sample(samLinear, newTexCoords).r;
    float roughness = RoughnessTexture.Sample(samLinear, newTexCoords).r;
    // Optionally, you can sample AO here as well if not stored in albedo.alpha
    output.MetalRoughAO = float3(metallic, roughness, ao);
    
    // Store world-space position (unchanged)
    output.Position = float4(input.WorldPos, 1.0f);
    
    return output;
}
