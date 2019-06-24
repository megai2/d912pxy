/*
    ps_3_0
    def c21, 1, 0, 2, -1
    def c22, -0.5, 128, -2000, -0.000500000024
    def c23, -2, 3, 4, -6
    def c24, 0.800000012, 0.349999994, 0.649999976, 0.25
    def c25, -9.99999975e-005, -13.2877121, 0, 0
    dcl_texcoord v0.xy
    dcl_texcoord2_pp v1
    dcl_texcoord3_pp v2
    dcl_texcoord4_pp v3
    dcl_texcoord5_pp v4.x
    dcl_texcoord6 v5
    dcl vPos.xy
    dcl_2d s0
    dcl_2d s1
    dcl_2d s2
    dcl_2d s3
    dcl_2d s4
    dcl_2d s13
    dcl_cube s15
    texld_pp r0, v0, s0
    add_sat_pp r1.x, r0.w, r0.w
    add_pp r1, r1.x, c22.x
    texkill r1
    texld_pp r1, v0, s1
    dp4_pp r2.x, r1, c21.x
    add r2.y, -r2.x, c21.x
    rcp r2.x, r2.x
    mul_pp r1, r1, r2.x
    max_pp r3.x, r2.y, c21.y
    mad_pp r2, r0.xyzx, c21.xxxy, c21.yyyx
    dp4_pp r4.x, c7, r2
    dp4_pp r4.y, c8, r2
    dp4_pp r4.z, c9, r2
    dp4_pp r4.w, c10, r2
    dp4_pp r4.x, r4, r1
    dp4_pp r5.x, c11, r2
    dp4_pp r5.y, c12, r2
    dp4_pp r5.z, c13, r2
    dp4_pp r5.w, c14, r2
    dp4_pp r4.y, r5, r1
    dp4_pp r5.x, c15, r2
    dp4_pp r5.y, c16, r2
    dp4_pp r5.z, c17, r2
    dp4_pp r5.w, c18, r2
    dp4_pp r4.z, r5, r1
    lrp_pp r1.xyz, r3.x, r0, r4
    add_pp r0.x, r0.w, c22.x
    add_sat_pp r0.x, r0.x, r0.x
    add_pp r0.y, c22.z, v4.x
    mul_sat_pp r0.y, r0.y, c22.w
    mad_pp r0.z, r0.y, c23.x, c23.y
    mul_pp r0.y, r0.y, r0.y
    mul_pp r0.y, r0.y, r0.z
    mad_pp r2.w, r0.y, -c23.y, -c23.w
    mad_pp r0.y, r0.y, -r0.x, c21.x
    texld_pp r3, v0, s2
    mad_pp r0.zw, r3.xyxy, c21.z, c21.w
    mul_pp r3.xyz, r0.w, v1
    mad_pp r3.xyz, r0.z, v2, r3
    dp2add_pp r0.z, r0.zwzw, -r0.zwzw, c21.x
    max_pp r1.w, r0.z, c21.y
    rsq_pp r0.z, r1.w
    rcp_pp r0.z, r0.z
    mad_pp r3.xyz, r0.z, v3, r3
    nrm_pp r2.xyz, r3
    texldl_pp r3, r2, s15
    mul_pp r3.xyz, r3, c24.x
    mad_pp r0.z, r0.x, c24.y, c24.z
    mul_pp r0.x, r0.x, c19.w
    mul_pp r4.xyz, r0.z, r3
    mul_pp r4.xyz, r1, r4
    mad r3.xyz, r3, r0.z, -r4
    mad_pp r3.xyz, r3, c24.w, r4
    mul_pp r3.xyz, r3, c20.x
    texld_pp r4, v0, s4
    mul_pp r3.xyz, r3, r4.y
    add r0.zw, c6.x, vPos.xyxy
    mul r0.zw, r0, c2.xyxy
    texld r5, r0.zwzw, s13
    mul_pp r5, r5, c0.y
    mul_pp r1.xyz, r1, r5
    mul_pp r0.z, r0.y, c23.z
    cmp_pp r6.w, r0.y, r0.z, c21.y
    mov_pp r5.x, v2.w
    mov_pp r5.y, v1.w
    mov_pp r5.z, v3.w
    dp3_pp r0.y, r5, r5
    rsq_pp r0.y, r0.y
    mul_pp r4.xzw, r0.y, r5.xyyz
    mad_pp r0.yzw, r5.xxyz, r0.y, c5.xxyz
    nrm_pp r5.xyz, r0.yzww
    dp3_pp r0.y, r2, r5
    dp3_pp r0.z, -r4.xzww, r2
    add_pp r0.z, r0.z, r0.z
    mad_pp r6.xyz, r2, -r0.z, -r4.xzww
    dp3_sat r0.z, r2, c5
    mul r2.xyz, r0.z, c4
    texldl_pp r6, r6, s15
    rcp r0.z, r6.w
    mul_pp r4.xzw, r0.z, r6.xyyz
    mul_pp r4.xzw, r4, r4
    mad_pp r4.xzw, r4, c19.xyyz, -r1.xyyz
    mad_pp r0.xzw, r0.x, r4, r1.xyyz
    texld_pp r1, v0, s3
    mul_pp r1.xyz, r1, c3
    mul_pp r1.w, r1.w, c22.y
    max_pp r2.w, c21.x, r1.w
    mul r4.xzw, r5.w, r1.xyyz
    mul_pp r1.xyz, r1, c20.x
    mul_pp r1.xyz, r4.y, r1
    mad_pp r0.xzw, r4, c21.z, r0
    mad_pp r0.xzw, r2.xyyz, r3.xyyz, r0
    add r1.w, r0.y, c25.x
    log_pp r0.y, r0.y
    cmp_pp r0.y, r1.w, r0.y, c25.y
    mul_pp r0.y, r0.y, r2.w
    exp_pp r0.y, r0.y
    mul_pp r0.y, r0.y, -c22.x
    mul r2.xyz, r0.y, c4
    mad_pp r0.xyz, r2, r1, r0.xzww
    add r0.w, c21.x, -v5.w
    mad_pp oC0.xyz, r0, r0.w, v5
    mov_pp oC0.w, c1.x

// approximately 111 instruction slots used (10 texture, 101 arithmetic)

*/
#define dx9_texture_srgb_read(a,b) 
#include "../../common.hlsli"
	
struct PS_INPUT
{
	float4 reg_misc0: SV_POSITION;
	float4 reg_inp0: TEXCOORD0;
	float4 unused_ireg_1_s1: TEXCOORD1;
	float4 reg_inp1: TEXCOORD2;
	float4 reg_inp2: TEXCOORD3;
	float4 reg_inp3: TEXCOORD4;
	float4 reg_inp4: TEXCOORD5;
	float4 reg_inp5: TEXCOORD6;
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
	float4 reg_const7 = getPassedPSFv(7);
	float4 reg_const8 = getPassedPSFv(8);
	float4 reg_const9 = getPassedPSFv(9);
	float4 reg_const10 = getPassedPSFv(10);
	float4 reg_t5 = { 0, 0, 0, 0 };
	float4 reg_const11 = getPassedPSFv(11);
	float4 reg_const12 = getPassedPSFv(12);
	float4 reg_const13 = getPassedPSFv(13);
	float4 reg_const14 = getPassedPSFv(14);
	float4 reg_const15 = getPassedPSFv(15);
	float4 reg_const16 = getPassedPSFv(16);
	float4 reg_const17 = getPassedPSFv(17);
	float4 reg_const18 = getPassedPSFv(18);
	float4 reg_const19 = getPassedPSFv(19);
	float4 reg_const20 = getPassedPSFv(20);
	float4 reg_const6 = getPassedPSFv(6);
	float4 reg_const2 = getPassedPSFv(2);
	float4 reg_const0 = getPassedPSFv(0);
	float4 reg_t6 = { 0, 0, 0, 0 };
	float4 reg_const5 = getPassedPSFv(5);
	float4 reg_const4 = getPassedPSFv(4);
	float4 reg_const3 = getPassedPSFv(3);
	#define dx9_ret_color_reg_ac ret.reg_clr_out0
	float4 reg_const1 = getPassedPSFv(1);
	 
	
    float4 reg_const21 = { 1.000000 , 0.000000 , 2.000000 , -1.000000 };
    float4 reg_const22 = { -0.500000 , 128.000000 , -2000.000000 , -0.000500 };
    float4 reg_const23 = { -2.000000 , 3.000000 , 4.000000 , -6.000000 };
    float4 reg_const24 = { 0.800000 , 0.350000 , 0.650000 , 0.250000 };
    float4 reg_const25 = { -0.000100 , -13.287712 , 0.000000 , 0.000000 };
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
     
    Texture2DArray reg_sampler3t = textureBinds[texState.texture_s3];
    sampler reg_sampler3s = samplerBinds[texState.sampler_s3];
    #define reg_sampler3_deftype tex2d
    #define reg_sampler3_srgb_flag 8
     
    Texture2DArray reg_sampler4t = textureBinds[texState.texture_s4];
    sampler reg_sampler4s = samplerBinds[texState.sampler_s4];
    #define reg_sampler4_deftype tex2d
    #define reg_sampler4_srgb_flag 16
     
    Texture2DArray reg_sampler13t = textureBinds[texState.texture_s13];
    sampler reg_sampler13s = samplerBinds[texState.sampler_s13];
    #define reg_sampler13_deftype tex2d
    #define reg_sampler13_srgb_flag 8192
     
    TextureCube reg_sampler15t = textureBindsCubed[texState.texture_s15];
    sampler reg_sampler15s = samplerBinds[texState.sampler_s15];
    #define reg_sampler15_deftype texCube
    #define reg_sampler15_srgb_flag 32768
     
    reg_t0 = dx9texld(reg_sampler0_deftype, reg_sampler0_srgb_flag, reg_sampler0t, reg_sampler0s, inp.reg_inp0);
    reg_t1.x = saturate(reg_t0.w + reg_t0.w);
    reg_t1 = reg_t1.xxxx + reg_const22.xxxx;
    clip(reg_t1);
    reg_t1 = dx9texld(reg_sampler1_deftype, reg_sampler1_srgb_flag, reg_sampler1t, reg_sampler1s, inp.reg_inp0);
    reg_t2.x = dot(reg_t1,reg_const21.xxxx);
    reg_t2.y = (-reg_t2.x) + reg_const21.x;
    reg_t2.x = dx9_rcp_guarded(reg_t2.x);
    reg_t1 = reg_t1 * reg_t2.xxxx;
    reg_t3.x = dx9_max(reg_t2.y, reg_const21.y);
    reg_t2 = (reg_t0.xyzx * reg_const21.xxxy ) + reg_const21.yyyx;
    reg_t4.x = dot(reg_const7,reg_t2);
    reg_t4.y = dot(reg_const8,reg_t2);
    reg_t4.z = dot(reg_const9,reg_t2);
    reg_t4.w = dot(reg_const10,reg_t2);
    reg_t4.x = dot(reg_t4,reg_t1);
    reg_t5.x = dot(reg_const11,reg_t2);
    reg_t5.y = dot(reg_const12,reg_t2);
    reg_t5.z = dot(reg_const13,reg_t2);
    reg_t5.w = dot(reg_const14,reg_t2);
    reg_t4.y = dot(reg_t5,reg_t1);
    reg_t5.x = dot(reg_const15,reg_t2);
    reg_t5.y = dot(reg_const16,reg_t2);
    reg_t5.z = dot(reg_const17,reg_t2);
    reg_t5.w = dot(reg_const18,reg_t2);
    reg_t4.z = dot(reg_t5,reg_t1);
    reg_t1.xyz = dx9_lerp(reg_t4.xyz, reg_t0.xyz, reg_t3.xxx);
    reg_t0.x = reg_t0.w + reg_const22.x;
    reg_t0.x = saturate(reg_t0.x + reg_t0.x);
    reg_t0.y = reg_const22.z + inp.reg_inp4.x;
    reg_t0.y = saturate(reg_t0.y * reg_const22.w);
    reg_t0.z = (reg_t0.y * reg_const23.x ) + reg_const23.y;
    reg_t0.y = reg_t0.y * reg_t0.y;
    reg_t0.y = reg_t0.y * reg_t0.z;
    reg_t2.w = (reg_t0.y * (-reg_const23.y) ) + (-reg_const23.w);
    reg_t0.y = (reg_t0.y * (-reg_t0.x) ) + reg_const21.x;
    reg_t3 = dx9texld(reg_sampler2_deftype, reg_sampler2_srgb_flag, reg_sampler2t, reg_sampler2s, inp.reg_inp0);
    reg_t0.zw = (reg_t3.xy * reg_const21.zz ) + reg_const21.ww;
    reg_t3.xyz = reg_t0.www * inp.reg_inp1.xyz;
    reg_t3.xyz = (reg_t0.zzz * inp.reg_inp2.xyz ) + reg_t3.xyz;
    reg_t0.z = dot(reg_t0.zw, (-reg_t0.zw)) + reg_const21.x;
    reg_t1.w = dx9_max(reg_t0.z, reg_const21.y);
    reg_t0.z = dx9_rsqrt(reg_t1.w);
    reg_t0.z = dx9_rcp(reg_t0.z);
    reg_t3.xyz = (reg_t0.zzz * inp.reg_inp3.xyz ) + reg_t3.xyz;
    reg_t2.xyz = dx9_normalize(reg_t3.xyz);
    reg_t3 = dx9texldl(reg_sampler15_deftype, reg_sampler15_srgb_flag, reg_sampler15t, reg_sampler15s, reg_t2, reg_t2.w);
    reg_t3.xyz = reg_t3.xyz * reg_const24.xxx;
    reg_t0.z = (reg_t0.x * reg_const24.y ) + reg_const24.z;
    reg_t0.x = reg_t0.x * reg_const19.w;
    reg_t4.xyz = reg_t0.zzz * reg_t3.xyz;
    reg_t4.xyz = reg_t1.xyz * reg_t4.xyz;
    reg_t3.xyz = (reg_t3.xyz * reg_t0.zzz ) + (-reg_t4.xyz);
    reg_t3.xyz = (reg_t3.xyz * reg_const24.www ) + reg_t4.xyz;
    reg_t3.xyz = reg_t3.xyz * reg_const20.xxx;
    reg_t4 = dx9texld(reg_sampler4_deftype, reg_sampler4_srgb_flag, reg_sampler4t, reg_sampler4s, inp.reg_inp0);
    reg_t3.xyz = reg_t3.xyz * reg_t4.yyy;
    reg_t0.zw = reg_const6.xx + inp.reg_misc0.xy;
    reg_t0.zw = reg_t0.zw * reg_const2.xy;
    reg_t5 = dx9texld(reg_sampler13_deftype, reg_sampler13_srgb_flag, reg_sampler13t, reg_sampler13s, reg_t0.zwzw);
    reg_t5 = reg_t5 * reg_const0.yyyy;
    reg_t1.xyz = reg_t1.xyz * reg_t5.xyz;
    reg_t0.z = reg_t0.y * reg_const23.z;
    {
        float tmpCmpVec = 0;
        reg_t6.w = (reg_t0.y >= tmpCmpVec) ? reg_t0.z : reg_const21.y;
    }
    reg_t5.x = inp.reg_inp2.w;
    reg_t5.y = inp.reg_inp1.w;
    reg_t5.z = inp.reg_inp3.w;
    reg_t0.y = dot(reg_t5.xyz,reg_t5.xyz);
    reg_t0.y = dx9_rsqrt(reg_t0.y);
    reg_t4.xzw = reg_t0.yyy * reg_t5.xyz;
    reg_t0.yzw = (reg_t5.xyz * reg_t0.yyy ) + reg_const5.xyz;
    reg_t5.xyz = dx9_normalize(reg_t0.yzw);
    reg_t0.y = dot(reg_t2.xyz,reg_t5.xyz);
    reg_t0.z = dot((-reg_t4.xzw),reg_t2.xyz);
    reg_t0.z = reg_t0.z + reg_t0.z;
    reg_t6.xyz = (reg_t2.xyz * (-reg_t0.zzz) ) + (-reg_t4.xzw);
    reg_t0.z = saturate(dot(reg_t2.xyz,reg_const5.xyz));
    reg_t2.xyz = reg_t0.zzz * reg_const4.xyz;
    reg_t6 = dx9texldl(reg_sampler15_deftype, reg_sampler15_srgb_flag, reg_sampler15t, reg_sampler15s, reg_t6, reg_t6.w);
    reg_t0.z = dx9_rcp(reg_t6.w);
    reg_t4.xzw = reg_t0.zzz * reg_t6.xyz;
    reg_t4.xzw = reg_t4.xzw * reg_t4.xzw;
    reg_t4.xzw = (reg_t4.xzw * reg_const19.xyz ) + (-reg_t1.xyz);
    reg_t0.xzw = (reg_t0.xxx * reg_t4.xzw ) + reg_t1.xyz;
    reg_t1 = dx9texld(reg_sampler3_deftype, reg_sampler3_srgb_flag, reg_sampler3t, reg_sampler3s, inp.reg_inp0);
    reg_t1.xyz = reg_t1.xyz * reg_const3.xyz;
    reg_t1.w = reg_t1.w * reg_const22.y;
    reg_t2.w = dx9_max(reg_const21.x, reg_t1.w);
    reg_t4.xzw = reg_t5.www * reg_t1.xyz;
    reg_t1.xyz = reg_t1.xyz * reg_const20.xxx;
    reg_t1.xyz = reg_t4.yyy * reg_t1.xyz;
    reg_t0.xzw = (reg_t4.xzw * reg_const21.zzz ) + reg_t0.xzw;
    reg_t0.xzw = (reg_t2.xyz * reg_t3.xyz ) + reg_t0.xzw;
    reg_t1.w = reg_t0.y + reg_const25.x;
    reg_t0.y = dx9_log(reg_t0.y);
    {
        float tmpCmpVec = 0;
        reg_t0.y = (reg_t1.w >= tmpCmpVec) ? reg_t0.y : reg_const25.y;
    }
    reg_t0.y = reg_t0.y * reg_t2.w;
    reg_t0.y = dx9_exp(reg_t0.y);
    reg_t0.y = reg_t0.y * (-reg_const22.x);
    reg_t2.xyz = reg_t0.yyy * reg_const4.xyz;
    reg_t0.xyz = (reg_t2.xyz * reg_t1.xyz ) + reg_t0.xzw;
    reg_t0.w = reg_const21.x + (-inp.reg_inp5.w);
    ret.reg_clr_out0.xyz = (reg_t0.xyz * reg_t0.www ) + inp.reg_inp5.xyz;
    ret.reg_clr_out0.w = reg_const1.x;
    
    dx9_ps_write_emulation(dx9_ret_color_reg_ac);
    
    return ret;
    
}
