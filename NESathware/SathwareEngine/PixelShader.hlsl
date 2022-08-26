Texture2D simpleTexture : register(t0);
SamplerState simpleSampler : register(s0);

float4 main(float4 pos : SV_POSITION, float2 texCoord : TEXCOORD) : SV_TARGET
{
	return simpleTexture.Sample(simpleSampler, texCoord);
}