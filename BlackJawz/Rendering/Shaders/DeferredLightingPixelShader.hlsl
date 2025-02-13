SamplerState samLinear : register(s0);

Texture2D gAlbedo : register(t0);
Texture2D gNormal : register(t1);
Texture2D gPosition : register(t2);
Texture2D gSpecular : register(t3);

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
    // Sample from G-buffer
    float3 normal = gNormal.Sample(samLinear, input.TexC);
    float3 worldPos = gPosition.Sample(samLinear, input.TexC);
    float3 textureColor = gAlbedo.Sample(samLinear, input.TexC);
    float3 specular = gSpecular.Sample(samLinear, input.TexC);
    
    float3 finalColor = float3(0.0f, 0.0f, 0.0f);

    // Loop over each light
    for (int i = 0; i < numLights; ++i)
    {
        LightProperties light = lights[i];
        float3 lightDir = float3(0.0f, 0.0f, 0.0f);
        float attenuation = 1.0f;
        float3 fragToLight = float3(0.0f, 0.0f, 0.0f);
        float dist = 0.0f;
        
        // Determine light direction and attenuation based on the light type
        if (light.LightType == LIGHT_TYPE_POINT)
        {
            fragToLight = light.LightPosition.xyz - worldPos;
            dist = length(fragToLight);
            
            // Skip if the fragment is out of range
            if (dist > light.Range)
                continue;
                
            lightDir = fragToLight / dist; // Normalize

            // Attenuation and fade based on distance
            float rangeFactor = exp(-pow(dist / light.Range, 4));
            attenuation = rangeFactor / (1.0 + light.Attenuation.x * dist + light.Attenuation.y * (dist * dist));
        }
        else if (light.LightType == LIGHT_TYPE_DIRECTIONAL)
        {
            lightDir = normalize(-light.LightDirection); // Directional light
            attenuation = 1.0f;
        }
        else if (light.LightType == LIGHT_TYPE_SPOT)
        {
            fragToLight = light.LightPosition.xyz - worldPos;
            dist = length(fragToLight);
            
            if (dist > light.Range)
                continue;
                
            lightDir = fragToLight / dist;

            // Compute spot light factor
            float theta = dot(normalize(-light.LightDirection), -lightDir);
            float epsilon = light.SpotInnerCone - light.SpotOuterCone;
            float spotFactor = clamp((theta - light.SpotOuterCone) / epsilon, 0.0f, 1.0f);
            
            // Range attenuation and spot attenuation
            float rangeFactor = saturate(1.0f - (dist / light.Range));
            attenuation = rangeFactor * spotFactor / (light.Attenuation.x + light.Attenuation.y * dist + light.Attenuation.z * (dist * dist));
        }
        
        // Diffuse (Lambertian)
        float diff = max(dot(normal, lightDir), 0.0f);
        float3 diffuse = light.DiffuseLight.rgb * diff * textureColor.rgb;
        
        // Specular (Blinn-Phong)
        float3 viewDir = normalize(CameraPosition - worldPos);
        float3 halfwayDir = normalize(viewDir + lightDir);
        float spec = pow(max(dot(normal, halfwayDir), 0.0f), light.SpecularPower);
        float3 specularColor = light.SpecularLight.rgb * spec * specular.rgb;
        
        // Add the diffuse and specular components, applying attenuation
        finalColor += (diffuse + specularColor) * attenuation * light.Intensity;
        
        // Ambient Light
        finalColor += light.AmbientLight.rgb * textureColor.rgb;
    }

    return float4(finalColor, 1.0f);
}
