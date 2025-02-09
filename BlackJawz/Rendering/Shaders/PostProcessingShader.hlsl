SamplerState samLinear : register(s0);
Texture2D txDiffuse : register(t0);

struct VSInput
{
    float3 Position : POSITION;
    float2 TexC : TEXCOORD0;
};

struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexC : TEXCOORD0;
};

PSInput VS(VSInput input)
{
    PSInput output;
    output.Position = output.Position;
    output.TexC = output.TexC;
    return output;
}

float4 PS(PSInput input) : SV_TARGET
{
    float4 color = txDiffuse.Sample(samLinear, input.TexC);
    
	return color;
}