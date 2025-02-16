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
    float4 ClipPos : SV_POSITION; // Clip-space position
    float3 WorldPos : POSITION; // World-space position
    float3 Normal : NORMAL; // World-space normal
    float2 TexC : TEXCOORD0; // UV coordinates
    float3 Tangent : TANGENT; // World-space tangent
    float3x3 TBN : TBN_MATRIX; // Tangent-to-world matrix
};

VSOutput VS(VSInput input)
{
    VSOutput output;

    // Transform to world space
    float4 worldPos = mul(float4(input.Position, 1.0f), World);
    output.WorldPos = worldPos.xyz;

    // Transform to clip space
    output.ClipPos = mul(worldPos, View);
    output.ClipPos = mul(output.ClipPos, Projection);

    // Compute normal in world space using inverse transpose (for correct scaling)
    float3 normalW = normalize(mul((float3x3) World, input.Normal));
    output.Normal = normalW;

    // Transform tangent using the same method (assuming uniform scaling)
    float3 tangentW = normalize(mul((float3x3) World, input.Tangent));
    output.Tangent = tangentW;

    // Compute bitangent and TBN matrix
    float3 bitangent = cross(normalW, tangentW);
    output.TBN = float3x3(tangentW, bitangent, normalW);

    // Pass UV coordinates
    output.TexC = input.TexC;

    return output;
}