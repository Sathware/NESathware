struct Vout
{
	float4 pos : SV_POSITION;
	float2 texCoord : TEXCOORD;
};

Vout main(float3 pos : POSITION, float2 texCoord : TEXCOORD)
{
	Vout output;
	output.pos = float4(pos, 1.0f);
	output.texCoord = texCoord;
	return output;
}