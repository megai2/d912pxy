/*
    ps_3_0
    def c26, 1, 0, 2, -1
    def c27, -0.5, 4.99999987e-006, -0.400000006, 1.66666663
    def c28, 128, -2000, -0.000500000024, 4
    def c29, -2, 3, -3, 6
    def c30, 0.800000012, 0.349999994, 0.649999976, 0.25
    def c31, -9.99999975e-005, -13.2877121, 0, 0
    dcl_texcoord v0
    dcl_texcoord1_pp v1
    dcl_texcoord2_pp v2
    dcl_texcoord3_pp v3
    dcl_texcoord4_pp v4.x
    dcl_texcoord5 v5
    dcl_texcoord6 v6
    dcl vPos.xy
    dcl_2d s0
    dcl_2d s1
    dcl_2d s2
    dcl_2d s3
    dcl_2d s4
    dcl_2d s5
    dcl_2d s6
    dcl_2d s7
    dcl_2d s13
    dcl_cube s14
    dcl_2d s15
    texld_pp r0, v0, s0
    add_sat_pp r1.x, r0.w, r0.w
    add_pp r1, r1.x, c27.x
    texkill r1
    texld_pp r1, v0.zwzw, s2
    mad_pp r1.xy, r1, c26.z, c26.w
    texld_pp r2, v0, s3
    add_sat_pp r1.zw, r2.xyxy, c21.x
    mul_pp r1.xy, r1, r1.w
    mad_pp r1.xy, r1, c22.x, v0
    texld_pp r2, r1, s4
    mad r2, r2.xyzx, c26.xxxy, c26.yyyx
    dp4_pp r3.x, r2, c12
    dp4_pp r3.y, r2, c16
    dp4_pp r3.z, r2, c20
    mul_pp r1.xyw, r3.xyzz, c23.x
    mul_pp r1.xyz, r1.z, r1.xyww
    mul r2, c26.xxyy, v5.xyxx
    texldl r2, r2, s15
    mad r1.w, r2.x, -r2.x, r2.y
    add_sat r2.y, r1.w, c27.y
    cmp r1.w, r1.w, r2.y, c27.y
    add r2.y, r2.x, -v5.z
    add r2.x, -r2.x, v5.z
    cmp r2.x, r2.x, c26.y, c26.x
    mad r2.y, r2.y, r2.y, r1.w
    rcp r2.y, r2.y
    mad r1.w, r1.w, r2.y, c27.z
    mul_sat r1.w, r1.w, c27.w
    max r3.x, r2.x, r1.w
    mad_sat r1.w, v5.w, c3.x, c3.y
    lrp r2.x, r1.w, c26.x, r3.x
    add r1.w, -r2.x, c26.x
    mov r3.x, c26.x
    add r2.y, r3.x, c5.w
    mad_pp r1.w, r1.w, -r2.y, c26.x
    add r2.y, r3.x, -c5.w
    mul_pp r2.x, r2.y, r2.x
    cmp_pp r1.w, c5.w, r2.x, r1.w
    texld_pp r2, v0, s6
    mul r2.xyz, r2, c4
    mul_pp r2.w, r2.w, c28.x
    max_pp r3.x, c26.x, r2.w
    mad_pp r3.yzw, r1.w, r2.xxyz, r2.xxyz
    mul_pp r2.xyz, r2, c25.x
    mul r3.yzw, r3, c4.xxyz
    texld_pp r4, v0, s1
    dp4_pp r1.w, r4, c26.x
    add r2.w, -r1.w, c26.x
    rcp r1.w, r1.w
    mul_pp r4, r1.w, r4
    max_pp r1.w, r2.w, c26.y
    mad r5, r0.xyzx, c26.xxxy, c26.yyyx
    dp4_pp r6.x, c9, r5
    dp4_pp r6.y, c10, r5
    dp4_pp r6.z, c11, r5
    dp4_pp r6.w, c12, r5
    dp4_pp r6.x, r6, r4
    dp4_pp r7.x, c13, r5
    dp4_pp r7.y, c14, r5
    dp4_pp r7.z, c15, r5
    dp4_pp r7.w, c16, r5
    dp4_pp r6.y, r7, r4
    dp4_pp r7.x, c17, r5
    dp4_pp r7.y, c18, r5
    dp4_pp r7.z, c19, r5
    dp4_pp r7.w, c20, r5
    dp4_pp r6.z, r7, r4
    lrp_pp r4.xyz, r1.w, r0, r6
    add_pp r0.x, r0.w, c27.x
    add_sat_pp r0.x, r0.x, r0.x
    add r0.yz, c8.x, vPos.xxyw
    mul r0.yz, r0, c2.xxyw
    texld r5, r0.yzzw, s13
    mul_pp r5, r5, c0.y
    mul_pp r0.yzw, r4.xxyz, r5.xxyz
    add_pp r1.w, c28.y, v4.x
    mul_sat_pp r1.w, r1.w, c28.z
    mad_pp r2.w, r1.w, c29.x, c29.y
    mul_pp r1.w, r1.w, r1.w
    mul_pp r1.w, r1.w, r2.w
    mad_pp r2.w, r1.w, -r0.x, c26.x
    mad_pp r6.w, r1.w, c29.z, c29.w
    mul_pp r1.w, r2.w, c28.w
    cmp_pp r7.w, r2.w, r1.w, c26.y
    texld_pp r8, v0, s5
    mad_pp r5.xy, r8, c26.z, c26.w
    mul_pp r8.xyz, r5.y, v1
    mad_pp r8.xyz, r5.x, v2, r8
    dp2add_pp r1.w, r5, -r5, c26.x
    max_pp r2.w, r1.w, c26.y
    rsq_pp r1.w, r2.w
    rcp_pp r1.w, r1.w
    mad_pp r5.xyz, r1.w, v3, r8
    nrm_pp r6.xyz, r5
    mov_pp r5.x, v2.w
    mov_pp r5.y, v1.w
    mov_pp r5.z, v3.w
    dp3_pp r1.w, r5, r5
    rsq_pp r1.w, r1.w
    mul_pp r8.xyz, r1.w, r5
    mad_pp r5.xyz, r5, r1.w, c7
    nrm_pp r9.xyz, r5
    dp3_pp r1.w, r6, r9
    dp3_pp r2.w, -r8, r6
    add_pp r2.w, r2.w, r2.w
    mad_pp r7.xyz, r6, -r2.w, -r8
    texldl_pp r7, r7, s14
    rcp r2.w, r7.w
    mul_pp r5.xyz, r2.w, r7
    mul_pp r5.xyz, r5, r5
    mad_pp r5.xyz, r5, c24, -r0.yzww
    mul_pp r2.w, r0.x, c24.w
    mad_pp r0.x, r0.x, c30.y, c30.z
    mad_pp r0.yzw, r2.w, r5.xxyz, r0
    mad r0.yzw, r3, r5.w, r0
    mad_pp r0.yzw, r1.xxyz, c26.z, r0
    texldl_pp r5, r6, s14
    dp3_sat r1.x, r6, c7
    mul r1.xyz, r1.x, c6
    mul_pp r3.yzw, r5.xxyz, c30.x
    mul_pp r5.xyz, r0.x, r3.yzww
    mul_pp r4.xyz, r4, r5
    mad r3.yzw, r3, r0.x, -r4.xxyz
    mad_pp r3.yzw, r3, c30.w, r4.xxyz
    mul_pp r3.yzw, r3, c25.x
    texld_pp r4, v0, s7
    mul_pp r3.yzw, r3, r4.y
    mul_pp r2.xyz, r2, r4.y
    mad_pp r0.xyz, r1, r3.yzww, r0.yzww
    add r0.w, r1.w, c31.x
    log_pp r1.x, r1.w
    cmp_pp r0.w, r0.w, r1.x, c31.y
    mul_pp r0.w, r0.w, r3.x
    exp_pp r0.w, r0.w
    mul_pp r0.w, r0.w, -c27.x
    mul r1.xyz, r0.w, c6
    mad_pp r0.xyz, r1, r2, r0
    add r0.w, c26.x, -v6.w
    mad_pp oC0.xyz, r0, r0.w, v6
    mov_pp oC0.w, c1.x

// approximately 149 instruction slots used (15 texture, 134 arithmetic)

*/
#define dx9_texture_srgb_read(a,b) 
#include "../../common.hlsli"
	
struct PS_INPUT
{
	float4 reg_misc0: SV_POSITION;
	float4 reg_inp0: TEXCOORD0;
	float4 reg_inp1: TEXCOORD1;
	float4 reg_inp2: TEXCOORD2;
	float4 reg_inp3: TEXCOORD3;
	float4 reg_inp4: TEXCOORD4;
	float4 reg_inp5: TEXCOORD5;
	float4 reg_inp6: TEXCOORD6;
};
	
struct PS_OUTPUT
{
	float4 reg_clr_out0: SV_TARGET;
};
 
PS_OUTPUT main(PS_INPUT inp)
{ 
	PS_OUTPUT ret; 
	
	float4 reg_misc0 = inp.reg_misc0;
	half4 reg_t0 = { 0, 0, 0, 0 };
	half4 reg_t1 = { 0, 0, 0, 0 };
	half4 reg_t2 = { 0, 0, 0, 0 };
	float4 reg_const21 = getPassedPSFv(21);
	float4 reg_const22 = getPassedPSFv(22);
	half4 reg_t3 = { 0, 0, 0, 0 };
	float4 reg_const12 = getPassedPSFv(12);
	float4 reg_const16 = getPassedPSFv(16);
	float4 reg_const20 = getPassedPSFv(20);
	float4 reg_const23 = getPassedPSFv(23);
	float4 reg_const3 = getPassedPSFv(3);
	float4 reg_const5 = getPassedPSFv(5);
	float4 reg_const4 = getPassedPSFv(4);
	float4 reg_const25 = getPassedPSFv(25);
	half4 reg_t4 = { 0, 0, 0, 0 };
	float4 reg_t5 = { 0, 0, 0, 0 };
	half4 reg_t6 = { 0, 0, 0, 0 };
	float4 reg_const9 = getPassedPSFv(9);
	float4 reg_const10 = getPassedPSFv(10);
	float4 reg_const11 = getPassedPSFv(11);
	half4 reg_t7 = { 0, 0, 0, 0 };
	float4 reg_const13 = getPassedPSFv(13);
	float4 reg_const14 = getPassedPSFv(14);
	float4 reg_const15 = getPassedPSFv(15);
	float4 reg_const17 = getPassedPSFv(17);
	float4 reg_const18 = getPassedPSFv(18);
	float4 reg_const19 = getPassedPSFv(19);
	float4 reg_const8 = getPassedPSFv(8);
	float4 reg_const2 = getPassedPSFv(2);
	float4 reg_const0 = getPassedPSFv(0);
	half4 reg_t8 = { 0, 0, 0, 0 };
	float4 reg_const7 = getPassedPSFv(7);
	half4 reg_t9 = { 0, 0, 0, 0 };
	float4 reg_const24 = getPassedPSFv(24);
	float4 reg_const6 = getPassedPSFv(6);
	#define dx9_ret_color_reg_ac ret.reg_clr_out0
	float4 reg_const1 = getPassedPSFv(1);
	 
	
    float4 reg_const26 = { 1.000000 , 0.000000 , 2.000000 , -1.000000 };
    float4 reg_const27 = { -0.500000 , 0.000005 , -0.400000 , 1.666667 };
    float4 reg_const28 = { 128.000000 , -2000.000000 , -0.000500 , 4.000000 };
    float4 reg_const29 = { -2.000000 , 3.000000 , -3.000000 , 6.000000 };
    float4 reg_const30 = { 0.800000 , 0.350000 , 0.650000 , 0.250000 };
    float4 reg_const31 = { -0.000100 , -13.287712 , 0.000000 , 0.000000 };
    reg_misc0 = reg_misc0 - 0.5f;
    #define ps_ros_reg_ac reg_misc0
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
     
    Texture2DArray reg_sampler5t = textureBinds[texState.texture_s5];
    sampler reg_sampler5s = samplerBinds[texState.sampler_s5];
    #define reg_sampler5_deftype tex2d
    #define reg_sampler5_srgb_flag 32
     
    Texture2DArray reg_sampler6t = textureBinds[texState.texture_s6];
    sampler reg_sampler6s = samplerBinds[texState.sampler_s6];
    #define reg_sampler6_deftype tex2d
    #define reg_sampler6_srgb_flag 64
     
    Texture2DArray reg_sampler7t = textureBinds[texState.texture_s7];
    sampler reg_sampler7s = samplerBinds[texState.sampler_s7];
    #define reg_sampler7_deftype tex2d
    #define reg_sampler7_srgb_flag 128
     
    Texture2DArray reg_sampler13t = textureBinds[texState.texture_s13];
    sampler reg_sampler13s = samplerBinds[texState.sampler_s13];
    #define reg_sampler13_deftype tex2d
    #define reg_sampler13_srgb_flag 8192
     
    TextureCube reg_sampler14t = textureBindsCubed[texState.texture_s14];
    sampler reg_sampler14s = samplerBinds[texState.sampler_s14];
    #define reg_sampler14_deftype texCube
    #define reg_sampler14_srgb_flag 16384
     
    Texture2DArray reg_sampler15t = textureBinds[texState.texture_s15];
    sampler reg_sampler15s = samplerBinds[texState.sampler_s15];
    #define reg_sampler15_deftype tex2d
    #define reg_sampler15_srgb_flag 32768
     
    reg_t0 = (half4)(dx9texld(reg_sampler0_deftype, reg_sampler0_srgb_flag, reg_sampler0t, reg_sampler0s, ((half4)(inp.reg_inp0))));
    reg_t1.x = saturate((half)(((half)(reg_t0.w)) + ((half)(reg_t0.w))));
    reg_t1 = (half4)(((half4)(reg_t1.xxxx)) + ((half4)(reg_const27.xxxx)));
    clip(reg_t1);
    reg_t1 = (half4)(dx9texld(reg_sampler2_deftype, reg_sampler2_srgb_flag, reg_sampler2t, reg_sampler2s, ((half4)(inp.reg_inp0.zwzw))));
    reg_t1.xy = (half2)((((half2)(reg_t1.xy)) * ((half2)(reg_const26.zz)) ) + ((half2)(reg_const26.ww)));
    reg_t2 = (half4)(dx9texld(reg_sampler3_deftype, reg_sampler3_srgb_flag, reg_sampler3t, reg_sampler3s, ((half4)(inp.reg_inp0))));
    reg_t1.zw = saturate((half2)(((half2)(reg_t2.xy)) + ((half2)(reg_const21.xx))));
    reg_t1.xy = (half2)(((half2)(reg_t1.xy)) * ((half2)(reg_t1.ww)));
    reg_t1.xy = (half2)((((half2)(reg_t1.xy)) * ((half2)(reg_const22.xx)) ) + ((half2)(inp.reg_inp0.xy)));
    reg_t2 = (half4)(dx9texld(reg_sampler4_deftype, reg_sampler4_srgb_flag, reg_sampler4t, reg_sampler4s, ((half4)(reg_t1))));
    reg_t2 = (reg_t2.xyzx * reg_const26.xxxy ) + reg_const26.yyyx;
    reg_t3.x = (half)(dot(((half4)(reg_t2)),((half4)(reg_const12))));
    reg_t3.y = (half)(dot(((half4)(reg_t2)),((half4)(reg_const16))));
    reg_t3.z = (half)(dot(((half4)(reg_t2)),((half4)(reg_const20))));
    reg_t1.xyw = (half3)(((half3)(reg_t3.xyz)) * ((half3)(reg_const23.xxx)));
    reg_t1.xyz = (half3)(((half3)(reg_t1.zzz)) * ((half3)(reg_t1.xyw)));
    reg_t2 = reg_const26.xxyy * inp.reg_inp5.xyxx;
    reg_t2 = dx9texldl(reg_sampler15_deftype, reg_sampler15_srgb_flag, reg_sampler15t, reg_sampler15s, reg_t2, reg_t2.w);
    reg_t1.w = (reg_t2.x * (-reg_t2.x) ) + reg_t2.y;
    reg_t2.y = saturate(reg_t1.w + reg_const27.y);
    {
        float tmpCmpVec = 0;
        reg_t1.w = (reg_t1.w >= tmpCmpVec) ? reg_t2.y : reg_const27.y;
    }
    reg_t2.y = reg_t2.x + (-inp.reg_inp5.z);
    reg_t2.x = (-reg_t2.x) + inp.reg_inp5.z;
    {
        float tmpCmpVec = 0;
        reg_t2.x = (reg_t2.x >= tmpCmpVec) ? reg_const26.y : reg_const26.x;
    }
    reg_t2.y = (reg_t2.y * reg_t2.y ) + reg_t1.w;
    reg_t2.y = dx9_rcp(reg_t2.y);
    reg_t1.w = (reg_t1.w * reg_t2.y ) + reg_const27.z;
    reg_t1.w = saturate(reg_t1.w * reg_const27.w);
    reg_t3.x = dx9_max(reg_t2.x, reg_t1.w);
    reg_t1.w = saturate((inp.reg_inp5.w * reg_const3.x ) + reg_const3.y);
    reg_t2.x = dx9_lerp(reg_t3.x, reg_const26.x, reg_t1.w);
    reg_t1.w = (-reg_t2.x) + reg_const26.x;
    reg_t3.x = reg_const26.x;
    reg_t2.y = reg_t3.x + reg_const5.w;
    reg_t1.w = (half)((((half)(reg_t1.w)) * ((half)((-reg_t2.y))) ) + ((half)(reg_const26.x)));
    reg_t2.y = reg_t3.x + (-reg_const5.w);
    reg_t2.x = (half)(((half)(reg_t2.y)) * ((half)(reg_t2.x)));
    {
        float tmpCmpVec = 0;
        reg_t1.w = (half)((((half)(reg_const5.w)) >= tmpCmpVec) ? ((half)(reg_t2.x)) : ((half)(reg_t1.w)));
    }
    reg_t2 = (half4)(dx9texld(reg_sampler6_deftype, reg_sampler6_srgb_flag, reg_sampler6t, reg_sampler6s, ((half4)(inp.reg_inp0))));
    reg_t2.xyz = reg_t2.xyz * reg_const4.xyz;
    reg_t2.w = (half)(((half)(reg_t2.w)) * ((half)(reg_const28.x)));
    reg_t3.x = (half)(dx9_max(((half)(reg_const26.x)), ((half)(reg_t2.w))));
    reg_t3.yzw = (half3)((((half3)(reg_t1.www)) * ((half3)(reg_t2.xyz)) ) + ((half3)(reg_t2.xyz)));
    reg_t2.xyz = (half3)(((half3)(reg_t2.xyz)) * ((half3)(reg_const25.xxx)));
    reg_t3.yzw = reg_t3.yzw * reg_const4.xyz;
    reg_t4 = (half4)(dx9texld(reg_sampler1_deftype, reg_sampler1_srgb_flag, reg_sampler1t, reg_sampler1s, ((half4)(inp.reg_inp0))));
    reg_t1.w = (half)(dot(((half4)(reg_t4)),((half4)(reg_const26.xxxx))));
    reg_t2.w = (-reg_t1.w) + reg_const26.x;
	//reg_t1.w = dx9_rcp(reg_t1.w);
    reg_t1.w = dx9_rcp_guarded(reg_t1.w);
    reg_t4 = (half4)(((half4)(reg_t1.wwww)) * ((half4)(reg_t4)));
    reg_t1.w = (half)(dx9_max(((half)(reg_t2.w)), ((half)(reg_const26.y))));
    reg_t5 = (reg_t0.xyzx * reg_const26.xxxy ) + reg_const26.yyyx;
    reg_t6.x = (half)(dot(((half4)(reg_const9)),((half4)(reg_t5))));
    reg_t6.y = (half)(dot(((half4)(reg_const10)),((half4)(reg_t5))));
    reg_t6.z = (half)(dot(((half4)(reg_const11)),((half4)(reg_t5))));
    reg_t6.w = (half)(dot(((half4)(reg_const12)),((half4)(reg_t5))));
    reg_t6.x = (half)(dot(((half4)(reg_t6)),((half4)(reg_t4))));
    reg_t7.x = (half)(dot(((half4)(reg_const13)),((half4)(reg_t5))));
    reg_t7.y = (half)(dot(((half4)(reg_const14)),((half4)(reg_t5))));
    reg_t7.z = (half)(dot(((half4)(reg_const15)),((half4)(reg_t5))));
    reg_t7.w = (half)(dot(((half4)(reg_const16)),((half4)(reg_t5))));
    reg_t6.y = (half)(dot(((half4)(reg_t7)),((half4)(reg_t4))));
    reg_t7.x = (half)(dot(((half4)(reg_const17)),((half4)(reg_t5))));
    reg_t7.y = (half)(dot(((half4)(reg_const18)),((half4)(reg_t5))));
    reg_t7.z = (half)(dot(((half4)(reg_const19)),((half4)(reg_t5))));
    reg_t7.w = (half)(dot(((half4)(reg_const20)),((half4)(reg_t5))));
    reg_t6.z = (half)(dot(((half4)(reg_t7)),((half4)(reg_t4))));
    reg_t4.xyz = (half3)(dx9_lerp(((half3)(reg_t6.xyz)), ((half3)(reg_t0.xyz)), ((half3)(reg_t1.www))));
    reg_t0.x = (half)(((half)(reg_t0.w)) + ((half)(reg_const27.x)));
    reg_t0.x = saturate((half)(((half)(reg_t0.x)) + ((half)(reg_t0.x))));
    reg_t0.yz = reg_const8.xx + reg_misc0.xy;
    reg_t0.yz = reg_t0.yz * reg_const2.xy;
    reg_t5 = dx9texld(reg_sampler13_deftype, reg_sampler13_srgb_flag, reg_sampler13t, reg_sampler13s, reg_t0.yzzw);
    reg_t5 = (half4)(((half4)(reg_t5)) * ((half4)(reg_const0.yyyy)));
    reg_t0.yzw = (half3)(((half3)(reg_t4.xyz)) * ((half3)(reg_t5.xyz)));
    reg_t1.w = (half)(((half)(reg_const28.y)) + ((half)(inp.reg_inp4.x)));
    reg_t1.w = saturate((half)(((half)(reg_t1.w)) * ((half)(reg_const28.z))));
    reg_t2.w = (half)((((half)(reg_t1.w)) * ((half)(reg_const29.x)) ) + ((half)(reg_const29.y)));
    reg_t1.w = (half)(((half)(reg_t1.w)) * ((half)(reg_t1.w)));
    reg_t1.w = (half)(((half)(reg_t1.w)) * ((half)(reg_t2.w)));
    reg_t2.w = (half)((((half)(reg_t1.w)) * ((half)((-reg_t0.x))) ) + ((half)(reg_const26.x)));
    reg_t6.w = (half)((((half)(reg_t1.w)) * ((half)(reg_const29.z)) ) + ((half)(reg_const29.w)));
    reg_t1.w = (half)(((half)(reg_t2.w)) * ((half)(reg_const28.w)));
    {
        float tmpCmpVec = 0;
        reg_t7.w = (half)((((half)(reg_t2.w)) >= tmpCmpVec) ? ((half)(reg_t1.w)) : ((half)(reg_const26.y)));
    }
    reg_t8 = (half4)(dx9texld(reg_sampler5_deftype, reg_sampler5_srgb_flag, reg_sampler5t, reg_sampler5s, ((half4)(inp.reg_inp0))));
    reg_t5.xy = (half2)((((half2)(reg_t8.xy)) * ((half2)(reg_const26.zz)) ) + ((half2)(reg_const26.ww)));
    reg_t8.xyz = (half3)(((half3)(reg_t5.yyy)) * ((half3)(inp.reg_inp1.xyz)));
    reg_t8.xyz = (half3)((((half3)(reg_t5.xxx)) * ((half3)(inp.reg_inp2.xyz)) ) + ((half3)(reg_t8.xyz)));
    reg_t1.w = (half)(dot(((half2)(reg_t5.xy)), ((half2)((-reg_t5.xy)))) + ((half)(reg_const26.x)));
    reg_t2.w = (half)(dx9_max(((half)(reg_t1.w)), ((half)(reg_const26.y))));
    reg_t1.w = (half)(dx9_rsqrt(((half)(reg_t2.w))));
    reg_t1.w = (half)(dx9_rcp(((half)(reg_t1.w))));
    reg_t5.xyz = (half3)((((half3)(reg_t1.www)) * ((half3)(inp.reg_inp3.xyz)) ) + ((half3)(reg_t8.xyz)));
    reg_t6.xyz = (half3)(dx9_normalize(((half3)(reg_t5.xyz))));
    reg_t5.x = (half)(((half)(inp.reg_inp2.w)));
    reg_t5.y = (half)(((half)(inp.reg_inp1.w)));
    reg_t5.z = (half)(((half)(inp.reg_inp3.w)));
    reg_t1.w = (half)(dot(((half3)(reg_t5.xyz)),((half3)(reg_t5.xyz))));
    reg_t1.w = (half)(dx9_rsqrt(((half)(reg_t1.w))));
    reg_t8.xyz = (half3)(((half3)(reg_t1.www)) * ((half3)(reg_t5.xyz)));
    reg_t5.xyz = (half3)((((half3)(reg_t5.xyz)) * ((half3)(reg_t1.www)) ) + ((half3)(reg_const7.xyz)));
    reg_t9.xyz = (half3)(dx9_normalize(((half3)(reg_t5.xyz))));
    reg_t1.w = (half)(dot(((half3)(reg_t6.xyz)),((half3)(reg_t9.xyz))));
    reg_t2.w = (half)(dot(((half3)((-reg_t8.xyz))),((half3)(reg_t6.xyz))));
    reg_t2.w = (half)(((half)(reg_t2.w)) + ((half)(reg_t2.w)));
    reg_t7.xyz = (half3)((((half3)(reg_t6.xyz)) * ((half3)((-reg_t2.www))) ) + ((half3)((-reg_t8.xyz))));
    reg_t7 = (half4)(dx9texldl(reg_sampler14_deftype, reg_sampler14_srgb_flag, reg_sampler14t, reg_sampler14s, ((half4)(reg_t7)), ((half)(reg_t7.w))));
    reg_t2.w = dx9_rcp(reg_t7.w);
    reg_t5.xyz = (half3)(((half3)(reg_t2.www)) * ((half3)(reg_t7.xyz)));
    reg_t5.xyz = (half3)(((half3)(reg_t5.xyz)) * ((half3)(reg_t5.xyz)));
    reg_t5.xyz = (half3)((((half3)(reg_t5.xyz)) * ((half3)(reg_const24.xyz)) ) + ((half3)((-reg_t0.yzw))));
    reg_t2.w = (half)(((half)(reg_t0.x)) * ((half)(reg_const24.w)));
    reg_t0.x = (half)((((half)(reg_t0.x)) * ((half)(reg_const30.y)) ) + ((half)(reg_const30.z)));
    reg_t0.yzw = (half3)((((half3)(reg_t2.www)) * ((half3)(reg_t5.xyz)) ) + ((half3)(reg_t0.yzw)));
    reg_t0.yzw = (reg_t3.yzw * reg_t5.www ) + reg_t0.yzw;
	reg_t0.yzw = (half3)((((half3)(reg_t1.xyz)) * ((half3)(reg_const26.zzz)) ) + ((half3)(reg_t0.yzw)));
	reg_t5 = (half4)(dx9texldl(reg_sampler14_deftype, reg_sampler14_srgb_flag, reg_sampler14t, reg_sampler14s, ((half4)(reg_t6)), ((half)(reg_t6.w))));
    reg_t1.x = saturate(dot(reg_t6.xyz,reg_const7.xyz));
    reg_t1.xyz = reg_t1.xxx * reg_const6.xyz;
    reg_t3.yzw = (half3)(((half3)(reg_t5.xyz)) * ((half3)(reg_const30.xxx)));
    reg_t5.xyz = (half3)(((half3)(reg_t0.xxx)) * ((half3)(reg_t3.yzw)));
    reg_t4.xyz = (half3)(((half3)(reg_t4.xyz)) * ((half3)(reg_t5.xyz)));
    reg_t3.yzw = (reg_t3.yzw * reg_t0.xxx ) + (-reg_t4.xyz);
    reg_t3.yzw = (half3)((((half3)(reg_t3.yzw)) * ((half3)(reg_const30.www)) ) + ((half3)(reg_t4.xyz)));
    reg_t3.yzw = (half3)(((half3)(reg_t3.yzw)) * ((half3)(reg_const25.xxx)));
    reg_t4 = (half4)(dx9texld(reg_sampler7_deftype, reg_sampler7_srgb_flag, reg_sampler7t, reg_sampler7s, ((half4)(inp.reg_inp0))));
    reg_t3.yzw = (half3)(((half3)(reg_t3.yzw)) * ((half3)(reg_t4.yyy)));
    reg_t2.xyz = (half3)(((half3)(reg_t2.xyz)) * ((half3)(reg_t4.yyy)));
    reg_t0.xyz = (half3)((((half3)(reg_t1.xyz)) * ((half3)(reg_t3.yzw)) ) + ((half3)(reg_t0.yzw)));
    reg_t0.w = reg_t1.w + reg_const31.x;
    reg_t1.x = (half)(dx9_log(((half)(reg_t1.w))));
    {
        float tmpCmpVec = 0;
        reg_t0.w = (half)((((half)(reg_t0.w)) >= tmpCmpVec) ? ((half)(reg_t1.x)) : ((half)(reg_const31.y)));
    }
	
    reg_t0.w = (half)(((half)(reg_t0.w)) * ((half)(reg_t3.x)));
    reg_t0.w = (half)(dx9_exp(((half)(reg_t0.w))));
    reg_t0.w = (half)(((half)(reg_t0.w)) * ((half)((-reg_const27.x))));
    reg_t1.xyz = reg_t0.www * reg_const6.xyz;
    reg_t0.xyz = (half3)((((half3)(reg_t1.xyz)) * ((half3)(reg_t2.xyz)) ) + ((half3)(reg_t0.xyz)));
    reg_t0.w = reg_const26.x + (-inp.reg_inp6.w);
    ret.reg_clr_out0.xyz = (half3)((((half3)(reg_t0.xyz)) * ((half3)(reg_t0.www)) ) + ((half3)(inp.reg_inp6.xyz)));
    ret.reg_clr_out0.w = (half)(((half)(reg_const1.x)));
  
    dx9_ps_write_emulation(dx9_ret_color_reg_ac);
    
    return ret;
    
}
