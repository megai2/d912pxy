/*
    ps_3_0
    def c18, 1, 0, -0.5, 2
    dcl_texcoord v0.xy
    dcl_texcoord2 v1
    dcl vPos.xy
    dcl_2d s0
    dcl_2d s1
    dcl_2d s2
    dcl_2d s14
    texld_pp r0, v0, s0
    add_sat_pp r1.x, r0.w, r0.w
    add_pp r1, r1.x, c18.z
    texkill r1
    texld_pp r1, v0, s1
    dp4_pp r2.x, r1, c18.x
    add r2.y, -r2.x, c18.x
    rcp r2.x, r2.x
    mul_pp r1, r1, r2.x
    max_pp r3.x, r2.y, c18.y
    mad_pp r2, r0.xyzx, c18.xxxy, c18.yyyx
    dp4_pp r4.x, c5, r2
    dp4_pp r4.y, c6, r2
    dp4_pp r4.z, c7, r2
    dp4_pp r4.w, c8, r2
    dp4_pp r4.x, r4, r1
    dp4_pp r5.x, c9, r2
    dp4_pp r5.y, c10, r2
    dp4_pp r5.z, c11, r2
    dp4_pp r5.w, c12, r2
    dp4_pp r4.y, r5, r1
    dp4_pp r5.x, c13, r2
    dp4_pp r5.y, c14, r2
    dp4_pp r5.z, c15, r2
    dp4_pp r5.w, c16, r2
    dp4_pp r4.z, r5, r1
    lrp_pp r1.xyz, r3.x, r0, r4
    add_pp r0.x, r0.w, c18.z
    add_sat_pp r0.x, r0.x, r0.x
    mul_pp r0.x, r0.x, c17.w
    add r0.yz, c4.x, vPos.xxyw
    mul r0.yz, r0, c2.xxyw
    texld r2, r0.yzzw, s14
    mul_pp r2, r2, c0.y
    mul_pp r0.yzw, r1.xxyz, r2.xxyz
    mad r0.xyz, r0.x, -r0.yzww, r0.yzww
    texld_pp r1, v0, s2
    mul_pp r1.xyz, r1, c3
    mul r1.xyz, r2.w, r1
    mad_pp r0.xyz, r1, c18.w, r0
    add r0.w, c18.x, -v1.w
    mad_pp oC0.xyz, r0, r0.w, v1
    mov_pp oC0.w, c1.x

// approximately 43 instruction slots used (4 texture, 39 arithmetic)

*/
#define dx9_texture_srgb_read(a,b) 
#include "../../common.hlsli"
	
struct PS_INPUT
{
	float4 reg_misc0: SV_POSITION;
	float4 reg_inp0: TEXCOORD0;
	float4 unused_ireg_1_s1: TEXCOORD1;
	float4 reg_inp1: TEXCOORD2;
};
	
struct PS_OUTPUT
{
	float4 reg_clr_out0: SV_TARGET;
};
 
PS_OUTPUT main(PS_INPUT inp)
{ 
	PS_OUTPUT ret; 
	
	float4 reg_t0 = { 0, 0, 0, 0 };
	float4 reg_t1 = { 0, 0, 0, 0 };
	float4 reg_t2 = { 0, 0, 0, 0 };
	float4 reg_t3 = { 0, 0, 0, 0 };
	float4 reg_t4 = { 0, 0, 0, 0 };
	float4 reg_const5 = getPassedPSFv(5);
	float4 reg_const6 = getPassedPSFv(6);
	float4 reg_const7 = getPassedPSFv(7);
	float4 reg_const8 = getPassedPSFv(8);
	float4 reg_t5 = { 0, 0, 0, 0 };
	float4 reg_const9 = getPassedPSFv(9);
	float4 reg_const10 = getPassedPSFv(10);
	float4 reg_const11 = getPassedPSFv(11);
	float4 reg_const12 = getPassedPSFv(12);
	float4 reg_const13 = getPassedPSFv(13);
	float4 reg_const14 = getPassedPSFv(14);
	float4 reg_const15 = getPassedPSFv(15);
	float4 reg_const16 = getPassedPSFv(16);
	float4 reg_const17 = getPassedPSFv(17);
	float4 reg_const4 = getPassedPSFv(4);
	float4 reg_const2 = getPassedPSFv(2);
	float4 reg_const0 = getPassedPSFv(0);
	float4 reg_const3 = getPassedPSFv(3);
	#define dx9_ret_color_reg_ac ret.reg_clr_out0
	float4 reg_const1 = getPassedPSFv(1);
	 
	
    float4 reg_const18 = { 1.000000 , 0.000000 , -0.500000 , 2.000000 };
    inp.reg_misc0 = inp.reg_misc0 - 0.5f;
    #define ps_ros_reg_ac inp.reg_misc0
    Texture2DArray reg_sampler0t = textureBinds[texState.texture_s0];
    sampler reg_sampler0s = samplerBinds[texState.sampler_s0];
    #define reg_sampler0_deftype tex2d
    #define reg_sampler0_srgb_flag 1
     
    Texture2DArray reg_sampler1t = textureBinds[texState.texture_s1];
    sampler reg_sampler1s = samplerBinds[texState.sampler_s1];
    #define reg_sampler1_deftype tex2d
    #define reg_sampler1_srgb_flag 2
     
    Texture2DArray reg_sampler2t = textureBinds[texState.texture_s2];
    sampler reg_sampler2s = samplerBinds[texState.sampler_s2];
    #define reg_sampler2_deftype tex2d
    #define reg_sampler2_srgb_flag 4
     
    Texture2DArray reg_sampler14t = textureBinds[texState.texture_s14];
    sampler reg_sampler14s = samplerBinds[texState.sampler_s14];
    #define reg_sampler14_deftype tex2d
    #define reg_sampler14_srgb_flag 16384
     
    reg_t0 = dx9texld(reg_sampler0_deftype, reg_sampler0_srgb_flag, reg_sampler0t, reg_sampler0s, inp.reg_inp0);
    reg_t1.x = saturate(reg_t0.w + reg_t0.w);
    reg_t1 = reg_t1.xxxx + reg_const18.zzzz;
    clip(reg_t1);
    reg_t1 = dx9texld(reg_sampler1_deftype, reg_sampler1_srgb_flag, reg_sampler1t, reg_sampler1s, inp.reg_inp0);
    reg_t2.x = dot(reg_t1,reg_const18.xxxx);
    reg_t2.y = (-reg_t2.x) + reg_const18.x;
    reg_t2.x = dx9_rcp_guarded(reg_t2.x);
    reg_t1 = reg_t1 * reg_t2.xxxx;
    reg_t3.x = dx9_max(reg_t2.y, reg_const18.y);
    reg_t2 = (reg_t0.xyzx * reg_const18.xxxy ) + reg_const18.yyyx;
    reg_t4.x = dot(reg_const5,reg_t2);
    reg_t4.y = dot(reg_const6,reg_t2);
    reg_t4.z = dot(reg_const7,reg_t2);
    reg_t4.w = dot(reg_const8,reg_t2);
    reg_t4.x = dot(reg_t4,reg_t1);
    reg_t5.x = dot(reg_const9,reg_t2);
    reg_t5.y = dot(reg_const10,reg_t2);
    reg_t5.z = dot(reg_const11,reg_t2);
    reg_t5.w = dot(reg_const12,reg_t2);
    reg_t4.y = dot(reg_t5,reg_t1);
    reg_t5.x = dot(reg_const13,reg_t2);
    reg_t5.y = dot(reg_const14,reg_t2);
    reg_t5.z = dot(reg_const15,reg_t2);
    reg_t5.w = dot(reg_const16,reg_t2);
    reg_t4.z = dot(reg_t5,reg_t1);
    reg_t1.xyz = dx9_lerp(reg_t4.xyz, reg_t0.xyz, reg_t3.xxx);
    reg_t0.x = reg_t0.w + reg_const18.z;
    reg_t0.x = saturate(reg_t0.x + reg_t0.x);
    reg_t0.x = reg_t0.x * reg_const17.w;
    reg_t0.yz = reg_const4.xx + inp.reg_misc0.xy;
    reg_t0.yz = reg_t0.yz * reg_const2.xy;
    reg_t2 = dx9texld(reg_sampler14_deftype, reg_sampler14_srgb_flag, reg_sampler14t, reg_sampler14s, reg_t0.yzzw);
    reg_t2 = reg_t2 * reg_const0.yyyy;
    reg_t0.yzw = reg_t1.xyz * reg_t2.xyz;
    reg_t0.xyz = (reg_t0.xxx * (-reg_t0.yzw) ) + reg_t0.yzw;
    reg_t1 = dx9texld(reg_sampler2_deftype, reg_sampler2_srgb_flag, reg_sampler2t, reg_sampler2s, inp.reg_inp0);
    reg_t1.xyz = reg_t1.xyz * reg_const3.xyz;
    reg_t1.xyz = reg_t2.www * reg_t1.xyz;
    reg_t0.xyz = (reg_t1.xyz * reg_const18.www ) + reg_t0.xyz;
    reg_t0.w = reg_const18.x + (-inp.reg_inp1.w);
    ret.reg_clr_out0.xyz = (reg_t0.xyz * reg_t0.www ) + inp.reg_inp1.xyz;
    ret.reg_clr_out0.w = reg_const1.x;
    
    dx9_ps_write_emulation(dx9_ret_color_reg_ac);
    
    return ret;
    
}
