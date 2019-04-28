#define PXY_INNER_MAX_SHADER_CONSTS 256

struct batchDataType
{
	uint texture_s0;
	uint texture_s1;
	uint texture_s2;
	uint texture_s3;
	uint texture_s4;
	uint texture_s5;
	uint texture_s6;
	uint texture_s7;
	uint texture_s8;
	uint texture_s9;
	uint texture_s10;
	uint texture_s11;
	uint texture_s12;
	uint texture_s13;
	uint texture_s14;
	uint texture_s15;
	uint texture_s16;
	uint texture_s17;
	uint texture_s18;
	uint texture_s19;
	uint texture_s20;
	uint texture_s21;
	uint texture_s22;
	uint texture_s23;
	uint texture_s24;
	uint texture_s25;
	uint texture_s26;
	uint texture_s27;
	uint texture_s28;
	uint texture_s29;
	uint texture_s30;
	uint texture_s31;	
	uint sampler_s0;
	uint sampler_s1;
	uint sampler_s2;
	uint sampler_s3;
	uint sampler_s4;
	uint sampler_s5;
	uint sampler_s6;
	uint sampler_s7;
	uint sampler_s8;
	uint sampler_s9;
	uint sampler_s10;
	uint sampler_s11;
	uint sampler_s12;
	uint sampler_s13;
	uint sampler_s14;
	uint sampler_s15;
	uint sampler_s16;
	uint sampler_s17;
	uint sampler_s18;
	uint sampler_s19;
	uint sampler_s20;
	uint sampler_s21;
	uint sampler_s22;
	uint sampler_s23;
	uint sampler_s24;
	uint sampler_s25;
	uint sampler_s26;
	uint sampler_s27;
	uint sampler_s28;
	uint sampler_s29;
	uint sampler_s30;
	uint sampler_s31;	
	float4 valv[PXY_INNER_MAX_SHADER_CONSTS];
	float4 valp[PXY_INNER_MAX_SHADER_CONSTS];	
	float4 clipplane0;
	float4 halfpixelFix;
	uint extraTextureBind0;
	uint extraTextureBind1;
	uint extraTextureBind2;
	uint extraTextureBind3;
	uint extraTextureSampler0;
	uint extraTextureSampler1;
	uint extraTextureSampler2;
	uint extraTextureSampler3;	
	float4 extraVars[12];
};

#define texState batchData

Texture2DArray 					textureBinds[]  : register(t0, space0);
TextureCube 					textureBindsCubed[]  : register(t0, space1);
sampler      					samplerBinds[]  : register(s0);
ConstantBuffer<batchDataType> batchData : register(b0);

SamplerComparisonState ShadowSampler : register(s0, space1);

float4 getPassedVSFv(uint idx)
{
	return batchData.valv[idx];
}

float4 getPassedPSFv(uint idx)
{
	return  batchData.valp[idx];
}

float dx9_alphatest_emulation_proc(uint data, float avf)
{
	if (data & 1)
	{	
		uint func = (data >> 1) & 0xF;
		uint ref = ((data >> 5) & 0xFF);
		uint av = avf * 255;
		bool cmpRet = true;
		
		if (func == 1)
		{
			cmpRet = false;
		} else if (func == 2) {
			cmpRet = (av < ref);
		} else if (func == 3) {
			cmpRet = (av == ref);
		} else if (func == 4) {
			cmpRet = (av <= ref);
		} else if (func == 5) {
			cmpRet = (av > ref);
		} else if (func == 6) {
			cmpRet = (av != ref);
		} else if (func == 7) {
			cmpRet = (av >= ref);
		} 
		
		return cmpRet ? 1 : -1;
		
	} else 
		return 1;
}

float4 color_s2lin(float4 cs) 
{
	float3 sRGB = cs.xyz;
	float3 lret = sRGB * (sRGB * (sRGB * 0.305306011 + 0.682171111) + 0.012522878);
	float4 ret = {lret.x, lret.y, lret.z, cs.w};
	
	return ret;
}

float4 color_lin2s(float4 cs) 
{
	float3 RGB = cs.xyz;
	float3 S1 = sqrt(RGB);
	float3 S2 = sqrt(S1);
	float3 S3 = sqrt(S2);
	float3 sRGB = 0.662002687 * S1 + 0.684122060 * S2 - 0.323583601 * S3 - 0.0225411470 * RGB;
	
	float4 ret = {sRGB.x, sRGB.y, sRGB.z, cs.w};
	
	return ret;
}

#define dx9_texture_srgb_read_proc(ret, mask) if (texState.texture_s30 & mask) { ret = color_s2lin(ret); }

#define vs_clip_plane0_def [clipplanes(texState.clipplane0)]

#define dx9_ps_write_emulation(color) 

#define dx9_ps_write_emulation_at(color) clip(dx9_alphatest_emulation_proc(texState.texture_s31, color.w))

#define dx9_ps_write_emulation_srgb(color) \
	{ \
		color = color_lin2s(color); \
	}

#define dx9_ps_write_emulation_at_srgb(color) \
	{ \
		clip(dx9_alphatest_emulation_proc(texState.texture_s31, color.w)); \
		color = color_lin2s(color); \
	}

float4 dx9texldl_tex2d(Texture2DArray tex, sampler spl, float4 uv, float w, uint srgbMask)
{
	float3 auv = 0;
	auv.xy = uv.xy;
			
	float4 ret = tex.SampleLevel(spl, auv, w);
	
	dx9_texture_srgb_read(ret, srgbMask);
	
	return ret;
}

float4 dx9texldl_depth(Texture2DArray tex, sampler spl, float4 uv, float w, uint srgbMask)
{
	float3 auv = 0;
	auv.xy = uv.xy;
				
	float4 ret = 0;
	ret.x = tex.SampleCmpLevelZero(ShadowSampler, auv, uv.z);
		
	return ret;
}

float4 dx9texld_depth(Texture2DArray tex, sampler spl, float4 uv, uint srgbMask)
{
	float3 auv = 0;
	auv.xy = uv.xy;
				
	float4 ret = 0;
	ret.x = tex.SampleCmpLevelZero(ShadowSampler, auv, uv.z);
		
	return ret;
}


float4 dx9texld_tex2d(Texture2DArray tex, sampler spl, float4 uv, uint srgbMask)
{
	float3 auv = 0;
	auv.xy = uv.xy;
		
	float4 ret = tex.Sample(spl, auv);
	
	dx9_texture_srgb_read(ret, srgbMask);
	
	return ret;
}

float4 dx9texldl_texCube(TextureCube tex, sampler spl, float4 uv, float w, uint srgbMask)
{
	float3 auv = uv.xyz;
		
	float4 ret = tex.SampleLevel(spl, auv, w);
	
	dx9_texture_srgb_read(ret, srgbMask);
	
	return ret;
}

float4 dx9texld_texCube(TextureCube tex, sampler spl, float4 uv, uint srgbMask)
{
	float3 auv = uv.xyz;	
	
	float4 ret = tex.Sample(spl, auv);
	
	dx9_texture_srgb_read(ret, srgbMask);
	
	return ret;
}

#define dx9texldl(dt, srgb, to, so, uv, w) dx9texldl_ ## dt(to,so,uv,w, srgb)
#define dx9texld(dt, srgb, to, so, uv)  dx9texld_ ## dt(to,so,uv, srgb)

float4 dx9_fix_halfpixel_offset(float4 inPos)
{
	float4 fixup = texState.halfpixelFix;
	
	float4 ret;
	ret.xy = inPos.w * fixup.xy + inPos.xy;
	ret.zw = inPos.zw;
		
	return ret;
}

#define dx9_log(a) log2(a)
#define dx9_expp(a) exp2(a)
#define dx9_exp(a) exp2(a)
#define dx9_lerp(a,b,c) lerp(a,b,c)
#define dx9_rcp(a) rcp(a)
#define dx9_max(a,b) max(a,b) 
#define dx9_min(a,b) min(a,b)
#define dx9_rsqrt(a) rsqrt(abs(a))
#define dx9_frac(a) frac(a)
#define dx9_pow(a,b) pow(abs(a), b)
#define dx9_normalize(a) normalize(a)
