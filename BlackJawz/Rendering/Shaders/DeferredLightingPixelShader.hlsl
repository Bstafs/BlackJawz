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

float4 PS(PSInput input) : SV_TARGET
{
    // --- Sample the G-buffer ---
    // Albedo (from RT0)
    float3 albedo = gAlbedo.Sample(samLinear, input.TexC).rgb;
    
    // Use AO from the MetalRoughAO buffer (RT2) or from gAlbedo.a if you prefer.
    float ao = gMetalRoughAO.Sample(samLinear, input.TexC).b;
    
    // World-space normal (RT1)
    float3 N = normalize(gNormal.Sample(samLinear, input.TexC).rgb);
    
    // World-space position (RT3)
    float3 pos = gPosition.Sample(samLinear, input.TexC).rgb;
    
    // Material properties from RT2
    float3 metalRoughAO = gMetalRoughAO.Sample(samLinear, input.TexC).rgb;
    float metallic = metalRoughAO.r;
    float roughness = saturate(metalRoughAO.g);
    
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
        float3 L = 0;
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
            float rangeFactor = exp(-pow(dist / light.Range, 4));
            attenuation = rangeFactor / (1.0 + light.Attenuation.x * dist + light.Attenuation.y * (dist * dist));
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
            attenuation = rangeFactor * spotFactor / (light.Attenuation.x + light.Attenuation.y * dist + light.Attenuation.z * (dist * dist));
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
        float alpha = roughness * roughness;
        float alpha2 = alpha * alpha;
        float denom = (NdotH * NdotH * (alpha2 - 1.0) + 1.0);
        float D = alpha2 / (PI * denom * denom);
        
        // Geometry function (Schlick-GGX approximation)
        float k = (alpha + 1.0) * (alpha + 1.0) / 8.0;
        float G_V = NdotV / (NdotV * (1.0 - k) + k);
        float G_L = NdotL / (NdotL * (1.0 - k) + k);
        float G = G_V * G_L;
        
        // Fresnel using Schlick's approximation
        float3 F = F0 + (1.0 - F0) * pow(1.0 - VdotH, 5.0);
        
        // Final specular term
        float3 specular = (D * G * F) / max(4.0 * NdotV * NdotL, 0.001);
        
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
    // A simple ambient contribution; in production you might use IBL here.
    float3 ambient = float3(0.03, 0.03, 0.03) * albedo * ao;
    
    float3 color = ambient + Lo;
    
    // Optionally: tone mapping and gamma correction can be applied here.
    return float4(color, 1.0);
}
