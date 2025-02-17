cbuffer TransformBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
};

// Input from vertex buffer
struct VSInput
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 TexC : TEXCOORD0;
    float3 Tangent : TANGENT;
};

// Output to G-Buffer pixel shader
struct VSOutput
{
    float4 Position : SV_POSITION; // Clip-space position
    float3 WorldPos : POSITION; // World-space position
    float3 Normal : NORMAL; // World-space normal
    float2 TexC : TEXCOORD0; // UV coordinates
    float3 Tangent : TANGENT; // World-space tangent
    float3x3 TBN_MATRIX : TBN_MATRIX; // Tangent-to-world matrix
};

VSOutput VS(VSInput input)
{
    VSOutput output;

    float4 worldPos = mul(float4(input.Position, 1.0f), World);
    output.WorldPos = worldPos.xyz;

    // Transform to clip space
    output.Position = mul(worldPos, View);
    output.Position = mul(output.Position, Projection);

    // Transform normal to world space 
    output.Normal = normalize(mul(input.Normal, (float3x3) World));
    
    // Transform Tangent to world space 
    output.Tangent = mul(input.Tangent.xyz, (float3x3) World);
    
    float3 N = normalize(mul(input.Normal, (float3x3) World));
    float3 T = normalize(mul(input.Tangent, (float3x3) World));
    float3 B = cross(T, N);
    
    float3x3 TBN_MATRIX = float3x3(T, B, N);

    output.TBN_MATRIX = TBN_MATRIX;

    // Pass UV coordinates
    output.TexC = input.TexC;

    return output;
}