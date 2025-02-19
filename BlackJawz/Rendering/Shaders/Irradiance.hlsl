struct VSInput
{
    float3 Pos : POSITION;
    float2 TexC : TEXCOORD;
};

struct PSInput
{
    float4 Pos : SV_POSITION;
    float2 TexC : TEXCOORD;
};

PSInput VS(VSInput input)
{
    PSInput output;
    output.Pos = float4(input.Pos, 1.0); // NDC
    output.TexC = input.TexC;
    return output;
}

static const float PI = 3.14159265359;

float4 PS(PSInput input) : SV_TARGET
{
    return float4(1, 1, 1, 1);

}
