SamplerState samLinear : register(s0);

Texture2D textureDiffuse : register(t0);

cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;

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
    
    float3 CameraPosition;
    float Padding04;
}


struct VSInput
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 TexC : TEXCOORD0;
};

struct PSInput
{
    float4 Position : SV_POSITION;
    float3 Normal : NORMAL;
    float2 TexC : TEXCOORD0;
    
    float3 WorldPos : POSITION;
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
    float4 textureColor = textureDiffuse.Sample(samLinear, input.TexC);

    float3 normal = normalize(input.Normal);
    
    // Ambient Lighting
    float3 ambient = AmbientLight.rgb * textureColor.rgb;
    
    // Compute light direction, distance, and attenuation
    float3 lightDir;
    float attenuation = 1.0f;
    
    if (LightType == 0) // Point Light
    {
        lightDir = LightPosition.xyz - input.WorldPos;
        float dist = length(lightDir);
        lightDir /= dist; // Normalize direction

        // Apply Light Range
        if (dist > Range)
            return float4(ambient, 1.0f); // No effect if outside range

        // Attenuation (inverse square law)
        float rangeFactor = saturate(1.0f - (dist / Range));
        attenuation = rangeFactor * (1.0f / (Attenuation.x + Attenuation.y * dist + Attenuation.z * (dist * dist)));
    }
    else if (LightType == 1) // Directional Light
    {
        lightDir = normalize(-LightDirection); // Directional lights have no attenuation
    }
    else if (LightType == 2) // Spot Light
    {
        lightDir = LightPosition.xyz - input.WorldPos;
        float dist = length(lightDir);
        lightDir /= dist; // Normalize direction

        // Apply Light Range
        if (dist > Range)
            return float4(ambient, 1.0f); // No effect if outside range

        // Spot Light Falloff
        float theta = dot(normalize(-LightDirection), -lightDir);
        float epsilon = SpotInnerCone - SpotOuterCone;
        float spotFactor = clamp((theta - SpotOuterCone) / epsilon, 0.0f, 1.0f);

        // Attenuation
        float rangeFactor = saturate(1.0f - (dist / Range));
        attenuation = rangeFactor * spotFactor * (1.0f / (Attenuation.x + Attenuation.y * dist + Attenuation.z * (dist * dist)));
    }

    // Diffuse Lighting
    float diff = max(dot(normal, lightDir), 0.0f);
    float3 diffuse = DiffuseLight.rgb * diff * textureColor.rgb;
    
    // Specular Lighting (Blinn-Phong)
    float3 viewDir = normalize(CameraPosition - input.WorldPos); // Use actual camera position
    float3 halfwayDir = normalize(viewDir + lightDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0f), SpecularPower);
    float3 specular = SpecularLight.rgb * spec * attenuation;
    
    // Final Color
    float3 finalColor = ambient + (diffuse + specular) * attenuation * Intensity;
    
    return float4(finalColor, 1.0f);
}