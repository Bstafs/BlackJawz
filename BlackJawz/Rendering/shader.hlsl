SamplerState samLinear : register(s0);

cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
}

struct VSInput
{
    float3 Position : POSITION;
    float4 Color : COLOR;
};

struct PSInput
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
};

PSInput MainVS(VSInput input)
{
    PSInput output;
      
    output.Position = mul(input.Position, World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);
    
    
    //output.Position = float4(input.Position, 1.0f); // Transform position
    output.Color = input.Color; // Pass color to the pixel shader
    
    return output;
}

float4 PS(PSInput input) : SV_TARGET
{
    return input.Color; // Use color passed from vertex shader
}