struct PSInput
{
    float4 Position : SV_POSITION;
    float3 WorldPos : POSITION; 
    float3 Normal : NORMAL;
    float2 TexC : TEXCOORD0;
    float3 Tangent : TANGENT; 
    float3x3 TBN_MATRIX : TBN_MATRIX; 
};

Texture2D DiffuseTexture : register(t0);
Texture2D NormalTexture : register(t1);
SamplerState samLinear : register(s0);

struct GBufferOutput
{
    float4 Albedo : SV_Target0; // Albedo (Diffuse Color)
    float4 Normal : SV_Target1; // Normal
    float4 Position : SV_Target2; // World Position
    float4 Specular : SV_Target3; // Specular
};

GBufferOutput PS(PSInput input)
{
    GBufferOutput output;

    // Store Albedo (sample from texture)
    output.Albedo = DiffuseTexture.Sample(samLinear, input.TexC);

    // Store Normal (normalized to make sure it's unit-length)
    float3 sampledNormal = NormalTexture.Sample(samLinear, input.TexC).xyz;
    sampledNormal = normalize(sampledNormal * 2.0f - 1.0f);
    float3 worldNormal = normalize(mul(sampledNormal, input.TBN_MATRIX));
    
    output.Normal = float4(worldNormal, 1.0f);

    // Store World Position
    output.Position = float4(input.WorldPos, 1.0f);
    
    output.Specular = float4(1.0f, 1.0f, 1.0f, 32.0f);
       
    return output;
}