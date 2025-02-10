cbuffer TransformBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
};

struct VSInput
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 TexC : TEXCOORD;
};

struct VSOutput
{
    float4 Position : SV_POSITION; // Clip-space position
    float3 WorldPos : POSITION; 
    float3 Normal : NORMAL;
    float2 TexC : TEXCOORD;
};

VSOutput VS(VSInput input)
{
    VSOutput output;

    // Transform to world space
    float4 worldPosition = mul(float4(input.Position, 1.0f), World);
    output.WorldPos = worldPosition.xyz;

    // Transform to clip space
    output.Position = mul(worldPosition, View);
    output.Position = mul(output.Position, Projection);

    // Transform normal to world space (assumes no non-uniform scaling)
    float3 normalW = mul(float4(input.Normal, 0.0f), World).xyz;
    output.Normal = normalize(normalW);

    output.TexC = input.TexC;

    return output;
}