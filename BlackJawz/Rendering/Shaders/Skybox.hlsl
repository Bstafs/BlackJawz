TextureCube SkyboxTexture : register(t0);
SamplerState Sampler : register(s0);

cbuffer MatrixBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
};

struct VS_INPUT
{
    float3 Position : POSITION;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float3 TexCoord : TEXCOORD0;
};

VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output;
    
    // Transform position to clip space (ignoring translation in the view matrix)
    float4x4 viewNoTranslation = View;
    viewNoTranslation._41 = 0;
    viewNoTranslation._42 = 0;
    viewNoTranslation._43 = 0;
    
    output.Position = mul(float4(input.Position, 1.0f), viewNoTranslation);
    output.Position = mul(output.Position, Projection);
    
    // Use the model-space position as the direction for cube mapping
    output.TexCoord = input.Position;
    
    return output;
}

float4 PS(VS_OUTPUT input) : SV_TARGET
{
    // Normalize the direction vector and sample the cube map
    float3 direction = normalize(input.TexCoord);
    return SkyboxTexture.Sample(Sampler, direction);
}