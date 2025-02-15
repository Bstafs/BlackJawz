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
    float2 TexC : TEXCOORD0;
    float3 Tangent : TANGENT;
};

struct VSOutput
{
    float4 Position : SV_POSITION; // Clip-space position
    float3 WorldPos : POSITION; 
    float3 Normal : NORMAL;
    float2 TexC : TEXCOORD0;
    float3 Tangent : TANGENT;
    float3x3 TBN_MATRIX : TBN_MATRIX;
};

VSOutput VS(VSInput input)
{
    VSOutput output;

    // Transform to world space
    output.Position = mul(float4(input.Position, 1.0f), World);
    output.WorldPos = output.Position.xyz;

    // Transform to clip space
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);

    // Transform normal to world space (assumes no non-uniform scaling)
    float3 normalW = mul(float4(input.Normal, 0.0f), World).xyz;
    
    output.Normal = normalize(normalW);
    output.Tangent = mul(input.Tangent, (float3x3) World);
    
    float3 N = output.Normal;
    float3 T = normalize(output.Tangent - dot(output.Tangent, N) * N);
    float3 B = cross(N, T);
    
    float3x3 TBN_MATRIX = float3x3(T, B, N);

    output.TBN_MATRIX = TBN_MATRIX;
    
    output.TexC = input.TexC;

    return output;
}