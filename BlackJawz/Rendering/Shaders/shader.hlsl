SamplerState samLinear : register(s0);

Texture2D textureDiffuse : register(t0);

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

cbuffer TransformBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
}


cbuffer LightsBuffer : register(b1)
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
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 TexC : TEXCOORD0;
};

struct PSInput
{
    float4 Position : SV_POSITION;
    float3 WorldPos : POSITION;
    float3 Normal : NORMAL;
    float2 TexC : TEXCOORD0;
};

PSInput MainVS(VSInput input)
{
    PSInput output;
      
    output.Position = mul(float4(input.Position, 1.0f), World);
    
    output.WorldPos = output.Position.xyz;
    
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);
    
    float3 normalW = mul(float4(input.Normal, 0.0f), World).xyz;
    output.Normal = normalize(normalW);
    
    output.TexC = input.TexC;
    
    return output;
}

float4 PS(PSInput input) : SV_TARGET
{
    // Sample the texture
    float4 textureColor = textureDiffuse.Sample(samLinear, input.TexC);
    float3 normal = normalize(input.Normal);
    
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
            fragToLight = light.LightPosition.xyz - input.WorldPos;
            dist = length(fragToLight);
            
            // Skip if the fragment is out of range
            if (dist > light.Range)
                continue;
                
            lightDir = fragToLight / dist; // Normalize
            
            float fadeStart = light.Range * 0.3; // Start fading out at 70% of the range
            float fadeEnd = light.Range; // Fully fade out at max range
            float rangeFactor = exp(-pow(dist / light.Range, 4));
            attenuation = rangeFactor / (1.0 + light.Attenuation.x * dist + light.Attenuation.y * (dist * dist));
        }
        else if (light.LightType == LIGHT_TYPE_DIRECTIONAL)
        {
            // Directional light
            lightDir = normalize(-light.LightDirection);
            attenuation = 1.0f;
        }
        else if (light.LightType == LIGHT_TYPE_SPOT)
        {
            // Spot Light
            fragToLight = light.LightPosition.xyz - input.WorldPos;
            dist = length(fragToLight);
            
            if (dist > light.Range)
                continue;
                
            lightDir = fragToLight / dist;
            
            // Compute the angle between the light direction and the vector to the fragment
            float theta = dot(normalize(-light.LightDirection), -lightDir);
            float epsilon = light.SpotInnerCone - light.SpotOuterCone;
            float spotFactor = clamp((theta - light.SpotOuterCone) / epsilon, 0.0f, 1.0f);
            
            float rangeFactor = saturate(1.0f - (dist / light.Range));
            attenuation = rangeFactor * spotFactor / (light.Attenuation.x + light.Attenuation.y * dist + light.Attenuation.z * (dist * dist));
        }
        
        // Diffuse 
        float diff = max(dot(normal, lightDir), 0.0f);
        float3 diffuse = light.DiffuseLight.rgb * diff * textureColor.rgb;
        
        // Specular (Blinn-Phong)
        float3 viewDir = normalize(CameraPosition - input.WorldPos);
        float3 halfwayDir = normalize(viewDir + lightDir);
        float spec = pow(max(dot(normal, halfwayDir), 0.0f), light.SpecularPower);
        float3 specular = light.SpecularLight.rgb * spec;
        
        // Apply attenuation and the lights intensity
        finalColor += (diffuse + specular) * attenuation * light.Intensity;
        
        // Ambient Light
        finalColor += light.AmbientLight.rgb * textureColor.rgb;
    }

    return float4(finalColor, 1.0f);
}