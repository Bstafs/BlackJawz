SamplerState samLinear : register(s0);

Texture2D textureDiffuse : register(t0);

cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
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
};

PSInput MainVS(VSInput input)
{
    PSInput output;
      
    output.Position = mul(float4(input.Position, 1.0f), World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);
    
    float3 normalW = mul(float4(input.Normal, 0.0f), World).xyz;
    output.Normal = normalize(normalW);
    
    output.TexC = input.TexC;
    
    return output;
}

float4 PS(PSInput input) : SV_TARGET
{ 
    float4 diffuseMap = textureDiffuse.Sample(samLinear, input.TexC);
    
    return float4(diffuseMap.xyz, 1.0f); 
}