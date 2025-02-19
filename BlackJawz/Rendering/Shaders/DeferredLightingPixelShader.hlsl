SamplerState samLinear : register(s0);
SamplerState samCube : register(s1);

Texture2D gAlbedo : register(t0);
Texture2D gNormal : register(t1);
Texture2D gMetalRoughAO : register(t2);
Texture2D gPosition : register(t3);

TextureCube textureSkyBox : register(t4);
TextureCube textureIrradianceMap : register(t5);
TextureCube texturepreFilteredEnvMap : register(t6);
Texture2D textureBRDFLUT : register(t7);

#define MAX_LIGHTS 10
#define PI 3.14159265359

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
};

static const int LIGHT_TYPE_POINT = 0;
static const int LIGHT_TYPE_DIRECTIONAL = 1;
static const int LIGHT_TYPE_SPOT = 2;

struct VSInput
{
    float3 position : POSITION;
    float2 texcoord : TEXCOORD0;
};

struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexC : TEXCOORD0;
};

PSInput VS(VSInput input)
{
    PSInput output;
    output.Position = float4(input.position.xy, 0.0, 1.0);
    output.TexC = input.texcoord;
    return output;
}

float D_GGX(float dotNH, float roughness)
{
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float denom = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
    return alpha2 / (PI * denom * denom);
}

float G_SchlicksmithGGX(float dotNL, float dotNV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    float GL = dotNL / (dotNL * (1.0 - k) + k);
    float GV = dotNV / (dotNV * (1.0 - k) + k);
    return GL * GV;
}

float3 F_SchlickR(float cosTheta, float3 F0, float roughness)
{
    return F0 + (max((1.0 - roughness).xxx, F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

float3 prefilteredReflection(float3 R, float roughness)
{
    const float MAX_REFLECTION_LOD = 9.0;
    float lod = roughness * MAX_REFLECTION_LOD;
    return texturepreFilteredEnvMap.SampleLevel(samCube, R, lod).rgb;
}

float4 PS(PSInput input) : SV_TARGET
{
    // Sample G-buffer
    float3 albedo = gAlbedo.Sample(samLinear, input.TexC).rgb;
    float3 N = normalize(gNormal.Sample(samLinear, input.TexC).rgb);
    float3 pos = gPosition.Sample(samLinear, input.TexC).rgb;
    float3 metalRoughAO = gMetalRoughAO.Sample(samLinear, input.TexC).rgb;
    
    float metallic = metalRoughAO.r;
    float roughness = saturate(metalRoughAO.g);
    float ao = metalRoughAO.b;
    
    float3 V = normalize(CameraPosition - pos);
    float3 R = reflect(-V, N);
    float NdotV = max(dot(N, V), 0.0);
    
    // Base reflectivity
    float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo, metallic);
    float3 Lo = float3(0.0, 0.0, 0.0);

    // Direct lighting
    for (int i = 0; i < numLights; ++i)
    {
        LightProperties light = lights[i];
        float3 L = 0.0;
        float attenuation = 1.0;
        float3 radiance = light.DiffuseLight.rgb * light.Intensity;

        // Light direction calculation
        if (light.LightType == LIGHT_TYPE_POINT)
        {
            float3 fragToLight = light.LightPosition.xyz - pos;
            float dist = length(fragToLight);
            if (dist > light.Range)
                continue;
            
            L = normalize(fragToLight);
            float rangeFactor = saturate(1.0 - dist / light.Range);
            attenuation = rangeFactor / (1.0 + light.Attenuation.x * dist +
                        light.Attenuation.y * dist * dist);
        }
        else if (light.LightType == LIGHT_TYPE_DIRECTIONAL)
        {
            L = normalize(-light.LightDirection);
        }
        else if (light.LightType == LIGHT_TYPE_SPOT)
        {
            float3 fragToLight = light.LightPosition.xyz - pos;
            float dist = length(fragToLight);
            if (dist > light.Range)
                continue;
            
            L = normalize(fragToLight);
            float theta = dot(L, normalize(-light.LightDirection));
            float epsilon = light.SpotInnerCone - light.SpotOuterCone;
            float spotIntensity = clamp((theta - light.SpotOuterCone) / epsilon, 0.0, 1.0);
            float rangeFactor = saturate(1.0 - dist / light.Range);
            attenuation = spotIntensity * rangeFactor / (1.0 +
                        light.Attenuation.x * dist +
                        light.Attenuation.y * dist * dist);
        }

        float3 H = normalize(V + L);
        float NdotL = max(dot(N, L), 0.0);
        float NdotH = max(dot(N, H), 0.0);
        float HdotV = max(dot(H, V), 0.0);

        if (NdotL > 0.0)
        {
            // Cook-Torrance BRDF
            float D = D_GGX(NdotH, roughness);
            float G = G_SchlicksmithGGX(NdotL, NdotV, roughness);
            float3 F = F_SchlickR(HdotV, F0, roughness);

            float3 kD = (1.0 - F) * (1.0 - metallic);
            float3 diffuse = (albedo / PI) * kD * radiance * NdotL;

            float denominator = 4.0 * NdotV * NdotL + 0.0001;
            float3 specular = (D * G * F) / denominator * radiance * NdotL;

            Lo += (diffuse + specular) * attenuation;
        }
    }

    // Ambient IBL
    float3 F_ibl = F_SchlickR(NdotV, F0, roughness);
    float3 kD_ibl = (1.0 - F_ibl) * (1.0 - metallic);
    
    // Diffuse IBL
    float3 irradiance = textureIrradianceMap.Sample(samCube, N).rgb;
    float3 diffuseIBL = irradiance * albedo * kD_ibl * ao;
    
    // Specular IBL
    float2 brdf = textureBRDFLUT.Sample(samLinear, float2(NdotV, roughness)).rg;
    float3 prefiltered = prefilteredReflection(R, roughness);
    float3 specularIBL = prefiltered * (F_ibl * brdf.x + brdf.y) * ao;
    
    // Combine
    float3 ambient = (diffuseIBL + specularIBL);
    float3 color = ambient + Lo;

    return float4(color, 1.0);
}
