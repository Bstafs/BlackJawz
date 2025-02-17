SamplerState samLinear : register(s0);

Texture2D gAlbedo : register(t0); // RT0: RGB = Albedo, A = AO (optional)
Texture2D gNormal : register(t1); // RT1: World-space normal
Texture2D gMetalRoughAO : register(t2); // RT2: R = Metalness, G = Roughness, B = AO
Texture2D gPosition : register(t3); // RT3: World-space position

#define MAX_LIGHTS 10

static const float PI = 3.14159265;

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

// Normal Distribution function 
float D_GGX(float dotNH, float roughness)
{
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float denom = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
    return (alpha2) / (PI * denom * denom);
}

// Geometric Shadowing function 
float G_SchlicksmithGGX(float dotNL, float dotNV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    float GL = dotNL / (dotNL * (1.0 - k) + k);
    float GV = dotNV / (dotNV * (1.0 - k) + k);
    return GL * GV;
}

// Fresnel function 
float3 F_Schlick(float cosTheta, float3 albedo, float metallic)
{
    float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo, metallic); // * material.specular
    float3 F = F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
    return F;
}

float4 PS(PSInput input) : SV_TARGET
{
    // --- Sample the G-buffer ---
    // Albedo (from RT0)
    float3 albedo = gAlbedo.Sample(samLinear, input.TexC).rgb;
        
    // World-space normal (RT1)
    float3 N = normalize(gNormal.Sample(samLinear, input.TexC).rgb);
    
    // World-space position (RT3)
    float3 pos = gPosition.Sample(samLinear, input.TexC).rgb;
    
    // Material properties from RT2
    float3 metalRoughAO = gMetalRoughAO.Sample(samLinear, input.TexC).rgb;
    float metallic = metalRoughAO.r;
    float roughness = saturate(metalRoughAO.g);
    float ao = metalRoughAO.b;
    
    // --- PBR Base Reflectivity ---
    // For dielectrics, F0 is typically around 0.04.
    // For metals, F0 comes from the albedo.
    float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo, metallic);
    
    // Camera (view) vector
    float3 V = normalize(CameraPosition - pos);
    
    // Accumulated outgoing radiance
    float3 Lo = float3(0, 0, 0);
     
    // --- Lighting Loop ---
    for (int i = 0; i < numLights; ++i)
    {
        LightProperties light = lights[i];
        float3 L = float3(0,0,0);
        float attenuation = 1.0;
        float3 fragToLight = 0;
        float dist = 0.0;
        
        // Determine light direction and attenuation based on light type
        if (light.LightType == LIGHT_TYPE_POINT)
        {
            fragToLight = light.LightPosition.xyz - pos;
            dist = length(fragToLight);
            if (dist > light.Range)
                continue;
            L = fragToLight / dist;
            float rangeFactor = saturate(1.0 - (dist / light.Range));
            attenuation = rangeFactor / max(1.0 + light.Attenuation.x * dist + light.Attenuation.y * (dist * dist), 0.001);
        }
        else if (light.LightType == LIGHT_TYPE_DIRECTIONAL)
        {
            L = normalize(-light.LightDirection);
            attenuation = 1.0;
        }
        else if (light.LightType == LIGHT_TYPE_SPOT)
        {
            fragToLight = light.LightPosition.xyz - pos;
            dist = length(fragToLight);
            if (dist > light.Range)
                continue;
            L = fragToLight / dist;
            float theta = dot(normalize(-light.LightDirection), -L);
            float epsilon = light.SpotInnerCone - light.SpotOuterCone;
            float spotFactor = clamp((theta - light.SpotOuterCone) / epsilon, 0.0, 1.0);
            float rangeFactor = saturate(1.0 - (dist / light.Range));
            attenuation = rangeFactor / max(1.0 + light.Attenuation.x * dist + light.Attenuation.y * (dist * dist), 0.001);
        }
        
        // Half vector
        float3 H = normalize(V + L);
        
        // Dot products
        float NdotL = max(dot(N, L), 0.0);
        float NdotV = max(dot(N, V), 0.0);
        float NdotH = max(dot(N, H), 0.0);
        float VdotH = max(dot(V, H), 0.0);
        
        // --- Microfacet BRDF (Specular) ---

        // GGX Normal Distribution Function
        float D = D_GGX(NdotH, roughness);
        
        // Geometry function (Schlick-GGX approximation)
        float G = G_SchlicksmithGGX(NdotL, NdotV, roughness);
        
        // Fresnel using Schlick's approximation
        float3 F = F_Schlick(VdotH, albedo, metallic);
        
        // Final specular term
        float3 specular = D * F * G / max(4.0 * NdotL * NdotV, 0.001);
        
        // --- Diffuse term ---
        // Metals have little to no diffuse reflection.
        float3 kD = (1.0 - F) * (1.0 - metallic);
        float3 diffuse = kD * albedo / PI;
        
        // Light radiance (includes light color and intensity)
        float3 radiance = light.DiffuseLight.rgb * attenuation * light.Intensity;
        
        // Accumulate contribution (scaled by the cosine term)
        Lo += (diffuse + specular) * radiance * NdotL;
                
    }
    
    // --- Ambient Term ---
    // A simple ambient contribution;  IBL here.
    float3 ambient = albedo * ao;
    
    float3 color = ambient + Lo;   
    
    // Optionally: tone mapping and gamma correction can be applied here.
    return float4(color, 1.0);
}
