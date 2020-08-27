#define PXY_INNER_MAX_SHADER_CONSTS 256

#define EL_DCL32 \
	EL_DCL( 0) EL_DCL( 1) EL_DCL( 2) EL_DCL( 3) EL_DCL( 4) EL_DCL( 5) EL_DCL( 6) EL_DCL( 7) \
	EL_DCL( 8) EL_DCL( 9) EL_DCL(10) EL_DCL(11) EL_DCL(12) EL_DCL(13) EL_DCL(14) EL_DCL(15) \
	EL_DCL(16) EL_DCL(17) EL_DCL(18) EL_DCL(19) EL_DCL(20) EL_DCL(21) EL_DCL(22) EL_DCL(23) \
	EL_DCL(24) EL_DCL(25) EL_DCL(26) EL_DCL(27) EL_DCL(28) EL_DCL(29) EL_DCL(30) EL_DCL(31)

//per batch data record
	
struct batchDataType
{	
	#define EL_DCL(x) uint texture_s##x;		
	EL_DCL32	
	#undef EL_DCL
	
	#define EL_DCL(x) uint sampler_s##x;	
	EL_DCL32	
	#undef EL_DCL
		
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

//root signature 

Texture2DArray 					textureBinds[]  : register(t0, space0);
TextureCube 					textureBindsCubed[]  : register(t0, space1);
sampler      					samplerBinds[]  : register(s0);
ConstantBuffer<batchDataType>   batchData : register(b0);
SamplerComparisonState ShadowSampler : register(s0, space1);

//texState alias
#define texState batchData

//shader variables lookup 

float4 getPassedVSFv(uint idx)
{
	return batchData.valv[idx];
}

float4 getPassedPSFv(uint idx)
{
	return  batchData.valp[idx];
}

//simplified SRGB<->RGB converters

float4 color_s2lin(float4 cs) 
{
	float3 sRGB = cs.xyz;
	float3 lret = sRGB * (sRGB * (sRGB * 0.305306011 + 0.682171111) + 0.012522878);
	float4 ret = {lret.x, lret.y, lret.z, cs.w};
	
	return ret;
}

float4 color_lin2s_thru(float4 cs) 
{
	float3 RGB = cs.xyz;
	float3 S1 = sqrt(RGB);
	float3 S2 = sqrt(S1);
	float3 S3 = sqrt(S2);
	float3 sRGB = 0.662002687 * S1 + 0.684122060 * S2 - 0.323583601 * S3 - 0.0225411470 * RGB;
	
	float4 ret = {sRGB.x, sRGB.y, sRGB.z, cs.w};
	
	return ret;
}

float4 color_lin2s_cond(float4 cs, int enable) 
{
	if (enable)
		return color_lin2s_thru(cs);
	else
		return cs;
}

//various dx9 emulations

#define dx9_ps_write_emulation(color) 

#define dx9_ps_write_emulation_at(color) clip(dx9_alphatest_emulation_proc(texState.texture_s31, color.w))

#define dx9_ps_write_emulation_srgb(color) \
	{ \
		color = srgb_write_color_lin2s(color); \
	}

#define dx9_ps_write_emulation_at_srgb(color) \
	{ \
		clip(dx9_alphatest_emulation_proc(texState.texture_s31, color.w)); \
		color = srgb_write_color_lin2s(color); \
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

#define dx9_texture_srgb_read_proc(ret, mask) if (texState.texture_s30 & mask) { ret = color_s2lin(ret); }

#define vs_clip_plane0_def [clipplanes(texState.clipplane0)]

#define dx9_ps_nan_cull_emulation(a) if (isnan(a.w)) discard
#define dx9_vs_nan_cull_emulation(a) if (isnan(a.w)) a.w = 1;

float4 dx9_fix_halfpixel_offset(float4 inPos)
{
	float4 fixup = texState.halfpixelFix;
	
	float4 ret;
	ret.xy = inPos.w * fixup.xy + inPos.xy;
	ret.zw = inPos.zw;
		
	return ret;
}



//dx9 opcodes macro

#define dx9_log(a) log2(a)
#define dx9_expp(a) exp2(a)
#define dx9_exp(a) exp2(a)
#define dx9_lerp(a,b,c) lerp(a,b,c)
#define dx9_rcp(a) rcp(a)
#define dx9_rcp_guarded(a) a != 0 ? rcp(a) : 1e37
#define dx9_max(a,b) max(a,b) 
#define dx9_min(a,b) min(a,b)
#define dx9_rsqrt(a) rsqrt(abs(a))
#define dx9_rsqrt_guarded(a) a != 0 ? rsqrt(abs(a)) : 1e37
#define dx9_frac(a) frac(a)
#define dx9_pow(a,b) ((a == 0) && (b == 0)) ? 0 : pow(abs(a), b)
#define dx9_normalize(a) normalize(a)
#define dx9_normalize_guarded(a) any(a) ? normalize(a) : 0
#define dx9texldl(dt, srgb, to, so, uv, w) dx9texldl_ ## dt(to,so,uv,w, srgb)
#define dx9texld(dt, srgb, to, so, uv)  dx9texld_ ## dt(to,so,uv, srgb)

//texld for different texture types

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

float4 dx9texldl_texCube(TextureCube tex, sampler spl, float4 uv, float w, uint srgbMask)
{
	float3 auv = uv.xyz;
		
	float4 ret = tex.SampleLevel(spl, auv, w);
	
	dx9_texture_srgb_read(ret, srgbMask);
	
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

float4 dx9texld_texCube(TextureCube tex, sampler spl, float4 uv, uint srgbMask)
{
	float3 auv = uv.xyz;	
	
	float4 ret = tex.Sample(spl, auv);
	
	dx9_texture_srgb_read(ret, srgbMask);
	
	return ret;
}