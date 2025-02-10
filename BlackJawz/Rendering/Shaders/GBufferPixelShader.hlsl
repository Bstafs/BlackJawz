struct PSInput
{
    float4 Position : SV_POSITION;
    float3 WorldPos : POSITION; 
    float3 Normal : NORMAL;
    float2 TexC : TEXCOORD0;
};

Texture2D DiffuseTexture : register(t0);
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
    output.Albedo = float4(1.0f, 1.0f, 1.0f, 1.0f) *  DiffuseTexture.Sample(samLinear, input.TexC);

    // Store Normal (normalized to make sure it's unit-length)
    output.Normal = float4(normalize(input.Normal), 1.0f);

    // Store World Position
    output.Position = float4(input.WorldPos, 1.0f);
    
    output.Specular = float4(1.0f, 1.0f, 1.0f, 1.0f);
       
    return output;
}