/*
    ps_3_0
    def c24, 1, 0, 2, -1
    def c25, -0.5, 128, -2000, -0.000500000024
    def c26, -2, 3, 4, 4.99999987e-006
    def c27, -0.400000006, 1.66666663, -3, 6
    def c28, 0.800000012, 0.349999994, 0.649999976, 0.25
    def c29, -9.99999975e-005, -13.2877121, 0, 0
    dcl_texcoord v0.xy
    dcl_texcoord1 v1
    dcl_texcoord2_pp v2
    dcl_texcoord3_pp v3
    dcl_texcoord4_pp v4
    dcl_texcoord5_pp v5.x
    dcl_texcoord6_pp v6.xyz
    dcl_texcoord7 v7
    dcl_2d s0
    dcl_2d s1
    dcl_2d s2
    dcl_2d s3
    dcl_2d s4
    dcl_2d s14
    dcl_cube s15
    texld_pp r0, v0, s0
    add_sat_pp r1.x, r0.w, r0.w
    add_pp r1, r1.x, c25.x
    texkill r1
    mul r1, c24.xxyy, v1.xyxx
    texldl r1, r1, s14
    mad r1.y, r1.x, -r1.x, r1.y
    add_sat r1.z, r1.y, c26.w
    cmp r1.y, r1.y, r1.z, c26.w
    add r1.z, r1.x, -v1.z
    add r1.x, -r1.x, v1.z
    cmp r1.x, r1.x, c24.y, c24.x
    mad r1.z, r1.z, r1.z, r1.y
    rcp r1.z, r1.z
    mad r1.y, r1.y, r1.z, c27.x
    mul_sat r1.y, r1.y, c27.y
    max r2.x, r1.x, r1.y
    mad_sat r1.x, v1.w, c1.x, c1.y
    lrp r3.x, r1.x, c24.x, r2.x
    add r1.x, -r3.x, c24.x
    mov r2.x, c24.x
    add r1.y, r2.x, c7.w
    mad_pp r1.x, r1.x, -r1.y, c24.x
    add r1.y, r2.x, -c7.w
    mul_pp r1.y, r1.y, r3.x
    cmp_pp r1.x, c7.w, r1.y, r1.x
    max r2.x, -c25.x, r1.x
    texld_pp r3, v0, s1
    dp4_pp r1.y, r3, c24.x
    add r1.z, -r1.y, c24.x
    rcp r1.y, r1.y
    mul_pp r3, r1.y, r3
    max_pp r2.y, r1.z, c24.y
    mad r4, r0.xyzx, c24.xxxy, c24.yyyx
    dp4_pp r5.x, c10, r4
    dp4_pp r5.y, c11, r4
    dp4_pp r5.z, c12, r4
    dp4_pp r5.w, c13, r4
    dp4_pp r5.x, r5, r3
    dp4_pp r6.x, c14, r4
    dp4_pp r6.y, c15, r4
    dp4_pp r6.z, c16, r4
    dp4_pp r6.w, c17, r4
    dp4_pp r5.y, r6, r3
    dp4_pp r6.x, c18, r4
    dp4_pp r6.y, c19, r4
    dp4_pp r6.z, c20, r4
    dp4_pp r6.w, c21, r4
    dp4_pp r5.z, r6, r3
    lrp_pp r1.yzw, r2.y, r0.xxyz, r5.xxyz
    add_pp r0.x, r0.w, c25.x
    add_sat_pp r0.x, r0.x, r0.x
    texld_pp r3, v0, s2
    mad_pp r0.yz, r3.xxyw, c24.z, c24.w
    mul_pp r2.yzw, r0.z, v2.xxyz
    mad_pp r2.yzw, r0.y, v3.xxyz, r2
    dp2add_pp r0.y, r0.yzzw, -r0.yzzw, c24.x
    max_pp r3.x, r0.y, c24.y
    rsq_pp r0.y, r3.x
    rcp_pp r0.y, r0.y
    mad_pp r0.yzw, r0.y, v4.xxyz, r2
    nrm_pp r3.xyz, r0.yzww
    mad r4, r3.xyzx, c24.xxxy, c24.yyyx
    dp4_pp r5.x, r4, c2
    dp4_pp r5.y, r4, c3
    dp4_pp r5.z, r4, c4
    dp4_sat_pp r0.y, r4, c5
    mul r0.yzw, r0.y, c6.xxyz
    add_pp r2.yzw, r5.xxyz, v6.xxyz
    mad_pp r0.yzw, r0, r1.x, r2
    mul_pp r0.yzw, r0, r1
    add_pp r1.x, c25.z, v5.x
    mul_sat_pp r1.x, r1.x, c25.w
    mad_pp r2.y, r1.x, c26.x, c26.y
    mul_pp r1.x, r1.x, r1.x
    mul_pp r1.x, r1.x, r2.y
    mad_pp r2.y, r1.x, -r0.x, c24.x
    mad_pp r3.w, r1.x, c27.z, c27.w
    texldl_pp r4, r3, s15
    mul_pp r4.xyz, r4, c28.x
    mul_pp r1.x, r2.y, c26.z
    cmp_pp r5.w, r2.y, r1.x, c24.y
    mov_pp r6.x, v3.w
    mov_pp r6.y, v2.w
    mov_pp r6.z, v4.w
    dp3_pp r1.x, r6, r6
    rsq_pp r1.x, r1.x
    mul_pp r2.yzw, r1.x, r6.xxyz
    dp3_pp r3.w, -r2.yzww, r3
    add_pp r3.w, r3.w, r3.w
    mad_pp r5.xyz, r3, -r3.w, -r2.yzww
    texldl_pp r5, r5, s15
    rcp r2.y, r5.w
    mul_pp r2.yzw, r2.y, r5.xxyz
    mul_pp r2.yzw, r2, r2
    mad_pp r2.yzw, r2, c22.xxyz, -r0
    mul_pp r3.w, r0.x, c22.w
    mad_pp r0.x, r0.x, c28.y, c28.z
    mad_pp r0.yzw, r3.w, r2, r0
    mad_pp r2.yzw, r6.xxyz, r1.x, c7.xxyz
    mad_pp r5.xyz, r6, r1.x, c9
    nrm_pp r6.xyz, r5
    dp3_pp r1.x, r3, r6
    nrm_pp r5.xyz, r2.yzww
    dp3_pp r2.y, r3, r5
    dp3_sat r2.z, r3, c9
    mul r3.xyz, r2.z, c8
    add r2.z, r2.y, c29.x
    log_pp r2.y, r2.y
    cmp_pp r2.y, r2.z, r2.y, c29.y
    texld_pp r5, v0, s3
    mul_pp r2.z, r5.w, c25.y
    mul_pp r5.xyz, r5, c6
    max_pp r3.w, c24.x, r2.z
    mul_pp r2.y, r2.y, r3.w
    exp_pp r2.y, r2.y
    mul_pp r2.y, r2.y, -c25.x
    add_pp r6.xyz, r5, r5
    mul_pp r5.xyz, r5, c23.x
    mul_pp r2.yzw, r2.y, r6.xxyz
    mad_pp r0.yzw, r2, r2.x, r0
    mul_pp r2.xyz, r0.x, r4
    mul_pp r1.yzw, r1, r2.xxyz
    mad r2.xyz, r4, r0.x, -r1.yzww
    mad_pp r1.yzw, r2.xxyz, c28.w, r1
    mul_pp r1.yzw, r1, c23.x
    texld_pp r2, v0, s4
    mul_pp r1.yzw, r1, r2.y
    mul_pp r2.xyz, r2.y, r5
    mad_pp r0.xyz, r3, r1.yzww, r0.yzww
    add r0.w, r1.x, c29.x
    log_pp r1.x, r1.x
    cmp_pp r0.w, r0.w, r1.x, c29.y
    mul_pp r0.w, r0.w, r3.w
    exp_pp r0.w, r0.w
    mul_pp r0.w, r0.w, -c25.x
    mul r1.xyz, r0.w, c8
    mad_pp r0.xyz, r1, r2, r0
    add r0.w, c24.x, -v7.w
    mad_pp oC0.xyz, r0, r0.w, v7
    mov_pp oC0.w, c0.x

// approximately 151 instruction slots used (11 texture, 140 arithmetic)

*/
#define dx9_texture_srgb_read(a,b) 
#include "../../common.hlsli"
	
struct PS_INPUT
{
	float4 unusedPos: SV_POSITION;
	float4 reg_inp0: TEXCOORD0;
	float4 reg_inp1: TEXCOORD1;
	float4 reg_inp2: TEXCOORD2;
	float4 reg_inp3: TEXCOORD3;
	float4 reg_inp4: TEXCOORD4;
	float4 reg_inp5: TEXCOORD5;
	float4 reg_inp6: TEXCOORD6;
	float4 reg_inp7: TEXCOORD7;
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
	float4 reg_const1 = getPassedPSFv(1);
	float4 reg_t3 = { 0, 0, 0, 0 };
	float4 reg_const7 = getPassedPSFv(7);
	float4 reg_t4 = { 0, 0, 0, 0 };
	float4 reg_t5 = { 0, 0, 0, 0 };
	float4 reg_const10 = getPassedPSFv(10);
	float4 reg_const11 = getPassedPSFv(11);
	float4 reg_const12 = getPassedPSFv(12);
	float4 reg_const13 = getPassedPSFv(13);
	float4 reg_t6 = { 0, 0, 0, 0 };
	float4 reg_const14 = getPassedPSFv(14);
	float4 reg_const15 = getPassedPSFv(15);
	float4 reg_const16 = getPassedPSFv(16);
	float4 reg_const17 = getPassedPSFv(17);
	float4 reg_const18 = getPassedPSFv(18);
	float4 reg_const19 = getPassedPSFv(19);
	float4 reg_const20 = getPassedPSFv(20);
	float4 reg_const21 = getPassedPSFv(21);
	float4 reg_const2 = getPassedPSFv(2);
	float4 reg_const3 = getPassedPSFv(3);
	float4 reg_const4 = getPassedPSFv(4);
	float4 reg_const5 = getPassedPSFv(5);
	float4 reg_const6 = getPassedPSFv(6);
	float4 reg_const22 = getPassedPSFv(22);
	float4 reg_const9 = getPassedPSFv(9);
	float4 reg_const8 = getPassedPSFv(8);
	float4 reg_const23 = getPassedPSFv(23);
	#define dx9_ret_color_reg_ac ret.reg_clr_out0
	float4 reg_const0 = getPassedPSFv(0);
	 
	
    float4 reg_const24 = { 1.000000 , 0.000000 , 2.000000 , -1.000000 };
    float4 reg_const25 = { -0.500000 , 128.000000 , -2000.000000 , -0.000500 };
    float4 reg_const26 = { -2.000000 , 3.000000 , 4.000000 , 0.000005 };
    float4 reg_const27 = { -0.400000 , 1.666667 , -3.000000 , 6.000000 };
    float4 reg_const28 = { 0.800000 , 0.350000 , 0.650000 , 0.250000 };
    float4 reg_const29 = { -0.000100 , -13.287712 , 0.000000 , 0.000000 };
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
     
    Texture2DArray reg_sampler14t = textureBinds[texState.texture_s14];
    sampler reg_sampler14s = samplerBinds[texState.sampler_s14];
    #define reg_sampler14_deftype tex2d
    #define reg_sampler14_srgb_flag 16384
     
    TextureCube reg_sampler15t = textureBindsCubed[texState.texture_s15];
    sampler reg_sampler15s = samplerBinds[texState.sampler_s15];
    #define reg_sampler15_deftype texCube
    #define reg_sampler15_srgb_flag 32768
     
    reg_t0 = dx9texld(reg_sampler0_deftype, reg_sampler0_srgb_flag, reg_sampler0t, reg_sampler0s, inp.reg_inp0);
    reg_t1.x = saturate(reg_t0.w + reg_t0.w);
    reg_t1 = reg_t1.xxxx + reg_const25.xxxx;
    clip(reg_t1);
    reg_t1 = reg_const24.xxyy * inp.reg_inp1.xyxx;
    reg_t1 = dx9texldl(reg_sampler14_deftype, reg_sampler14_srgb_flag, reg_sampler14t, reg_sampler14s, reg_t1, reg_t1.w);
    reg_t1.y = (reg_t1.x * (-reg_t1.x) ) + reg_t1.y;
    reg_t1.z = saturate(reg_t1.y + reg_const26.w);
    {
        float tmpCmpVec = 0;
        reg_t1.y = (reg_t1.y >= tmpCmpVec) ? reg_t1.z : reg_const26.w;
    }
    reg_t1.z = reg_t1.x + (-inp.reg_inp1.z);
    reg_t1.x = (-reg_t1.x) + inp.reg_inp1.z;
    {
        float tmpCmpVec = 0;
        reg_t1.x = (reg_t1.x >= tmpCmpVec) ? reg_const24.y : reg_const24.x;
    }
    reg_t1.z = (reg_t1.z * reg_t1.z ) + reg_t1.y;
    reg_t1.z = dx9_rcp(reg_t1.z);
    reg_t1.y = (reg_t1.y * reg_t1.z ) + reg_const27.x;
    reg_t1.y = saturate(reg_t1.y * reg_const27.y);
    reg_t2.x = dx9_max(reg_t1.x, reg_t1.y);
    reg_t1.x = saturate((inp.reg_inp1.w * reg_const1.x ) + reg_const1.y);
    reg_t3.x = dx9_lerp(reg_t2.x, reg_const24.x, reg_t1.x);
    reg_t1.x = (-reg_t3.x) + reg_const24.x;
    reg_t2.x = reg_const24.x;
    reg_t1.y = reg_t2.x + reg_const7.w;
    reg_t1.x = (reg_t1.x * (-reg_t1.y) ) + reg_const24.x;
    reg_t1.y = reg_t2.x + (-reg_const7.w);
    reg_t1.y = reg_t1.y * reg_t3.x;
    {
        float tmpCmpVec = 0;
        reg_t1.x = (reg_const7.w >= tmpCmpVec) ? reg_t1.y : reg_t1.x;
    }
    reg_t2.x = dx9_max((-reg_const25.x), reg_t1.x);
    reg_t3 = dx9texld(reg_sampler1_deftype, reg_sampler1_srgb_flag, reg_sampler1t, reg_sampler1s, inp.reg_inp0);
	reg_t1.y = dot(reg_t3,reg_const24.xxxx);
    reg_t1.z = (-reg_t1.y) + reg_const24.x;
    reg_t1.y = dx9_rcp(reg_t1.y + 0.00000001f);//megai2: hackfix div 0 GPU crash(but how?)
    reg_t3 = reg_t1.yyyy * reg_t3;
    reg_t2.y = dx9_max(reg_t1.z, reg_const24.y);
    reg_t4 = (reg_t0.xyzx * reg_const24.xxxy ) + reg_const24.yyyx;
    reg_t5.x = dot(reg_const10,reg_t4);
    reg_t5.y = dot(reg_const11,reg_t4);
    reg_t5.z = dot(reg_const12,reg_t4);
    reg_t5.w = dot(reg_const13,reg_t4);
    reg_t5.x = dot(reg_t5,reg_t3);
    reg_t6.x = dot(reg_const14,reg_t4);
    reg_t6.y = dot(reg_const15,reg_t4);
    reg_t6.z = dot(reg_const16,reg_t4);
    reg_t6.w = dot(reg_const17,reg_t4);
    reg_t5.y = dot(reg_t6,reg_t3);
    reg_t6.x = dot(reg_const18,reg_t4);
    reg_t6.y = dot(reg_const19,reg_t4);
    reg_t6.z = dot(reg_const20,reg_t4);
    reg_t6.w = dot(reg_const21,reg_t4);
    reg_t5.z = dot(reg_t6,reg_t3);
    reg_t1.yzw = dx9_lerp(reg_t5.xyz, reg_t0.xyz, reg_t2.yyy);
		
	//p6
    reg_t0.x = reg_t0.w + reg_const25.x;
    reg_t0.x = saturate(reg_t0.x + reg_t0.x);
    reg_t3 = dx9texld(reg_sampler2_deftype, reg_sampler2_srgb_flag, reg_sampler2t, reg_sampler2s, inp.reg_inp0);
    reg_t0.yz = (reg_t3.xy * reg_const24.zz ) + reg_const24.ww;
    reg_t2.yzw = reg_t0.zzz * inp.reg_inp2.xyz;
    reg_t2.yzw = (reg_t0.yyy * inp.reg_inp3.xyz ) + reg_t2.yzw;
    reg_t0.y = dot(reg_t0.yz, (-reg_t0.yz)) + reg_const24.x;
    reg_t3.x = dx9_max(reg_t0.y, reg_const24.y);
    reg_t0.y = dx9_rsqrt(reg_t3.x);
    reg_t0.y = dx9_rcp(reg_t0.y);
    reg_t0.yzw = (reg_t0.yyy * inp.reg_inp4.xyz ) + reg_t2.yzw;
    reg_t3.xyz = dx9_normalize(reg_t0.yzw);
    reg_t4 = (reg_t3.xyzx * reg_const24.xxxy ) + reg_const24.yyyx;
    reg_t5.x = dot(reg_t4,reg_const2);
    reg_t5.y = dot(reg_t4,reg_const3);
    reg_t5.z = dot(reg_t4,reg_const4);
    reg_t0.y = saturate(dot(reg_t4,reg_const5));
    reg_t0.yzw = reg_t0.yyy * reg_const6.xyz;
    reg_t2.yzw = reg_t5.xyz + inp.reg_inp6.xyz;
    reg_t0.yzw = (reg_t0.yzw * reg_t1.xxx ) + reg_t2.yzw;
    reg_t0.yzw = reg_t0.yzw * reg_t1.yzw;
	
	//p5
    reg_t1.x = reg_const25.z + inp.reg_inp5.x;
    reg_t1.x = saturate(reg_t1.x * reg_const25.w);
    reg_t2.y = (reg_t1.x * reg_const26.x ) + reg_const26.y;
    reg_t1.x = reg_t1.x * reg_t1.x;
    reg_t1.x = reg_t1.x * reg_t2.y;
    reg_t2.y = (reg_t1.x * (-reg_t0.x) ) + reg_const24.x;
    reg_t3.w = (reg_t1.x * reg_const27.z ) + reg_const27.w;
    reg_t4 = dx9texldl(reg_sampler15_deftype, reg_sampler15_srgb_flag, reg_sampler15t, reg_sampler15s, reg_t3, reg_t3.w);
    reg_t4.xyz = reg_t4.xyz * reg_const28.xxx;
    reg_t1.x = reg_t2.y * reg_const26.z;
    {
        float tmpCmpVec = 0;
        reg_t5.w = (reg_t2.y >= tmpCmpVec) ? reg_t1.x : reg_const24.y;
    }
    reg_t6.x = inp.reg_inp3.w;
    reg_t6.y = inp.reg_inp2.w;
    reg_t6.z = inp.reg_inp4.w;
    reg_t1.x = dot(reg_t6.xyz,reg_t6.xyz);
    reg_t1.x = dx9_rsqrt(reg_t1.x);
    reg_t2.yzw = reg_t1.xxx * reg_t6.xyz;
    reg_t3.w = dot((-reg_t2.yzw),reg_t3.xyz);
    reg_t3.w = reg_t3.w + reg_t3.w;
    reg_t5.xyz = (reg_t3.xyz * (-reg_t3.www) ) + (-reg_t2.yzw);
    reg_t5 = dx9texldl(reg_sampler15_deftype, reg_sampler15_srgb_flag, reg_sampler15t, reg_sampler15s, reg_t5, reg_t5.w);
    reg_t2.y = dx9_rcp(reg_t5.w);
    reg_t2.yzw = reg_t2.yyy * reg_t5.xyz;
    reg_t2.yzw = reg_t2.yzw * reg_t2.yzw;
    reg_t2.yzw = (reg_t2.yzw * reg_const22.xyz ) + (-reg_t0.yzw);
    reg_t3.w = reg_t0.x * reg_const22.w;
    reg_t0.x = (reg_t0.x * reg_const28.y ) + reg_const28.z;
    reg_t0.yzw = (reg_t3.www * reg_t2.yzw ) + reg_t0.yzw;
	
	//p4
    reg_t2.yzw = (reg_t6.xyz * reg_t1.xxx ) + reg_const7.xyz;
    reg_t5.xyz = (reg_t6.xyz * reg_t1.xxx ) + reg_const9.xyz;
    reg_t6.xyz = dx9_normalize(reg_t5.xyz);
    reg_t1.x = dot(reg_t3.xyz,reg_t6.xyz);
    reg_t5.xyz = dx9_normalize(reg_t2.yzw);
    reg_t2.y = dot(reg_t3.xyz,reg_t5.xyz);
    reg_t2.z = saturate(dot(reg_t3.xyz,reg_const9.xyz));
    reg_t3.xyz = reg_t2.zzz * reg_const8.xyz;
    reg_t2.z = reg_t2.y + reg_const29.x;
    reg_t2.y = dx9_log(reg_t2.y);
    {
        float tmpCmpVec = 0;
        reg_t2.y = (reg_t2.z >= tmpCmpVec) ? reg_t2.y : reg_const29.y;
    }
    reg_t5 = dx9texld(reg_sampler3_deftype, reg_sampler3_srgb_flag, reg_sampler3t, reg_sampler3s, inp.reg_inp0);
    reg_t2.z = reg_t5.w * reg_const25.y;
    reg_t5.xyz = reg_t5.xyz * reg_const6.xyz;
    reg_t3.w = dx9_max(reg_const24.x, reg_t2.z);
    reg_t2.y = reg_t2.y * reg_t3.w;
    reg_t2.y = dx9_exp(reg_t2.y);
    reg_t2.y = reg_t2.y * (-reg_const25.x);
    reg_t6.xyz = reg_t5.xyz + reg_t5.xyz;
    reg_t5.xyz = reg_t5.xyz * reg_const23.xxx;
    reg_t2.yzw = reg_t2.yyy * reg_t6.xyz;
    reg_t0.yzw = (reg_t2.yzw * reg_t2.xxx ) + reg_t0.yzw;
	
	//p3
    reg_t2.xyz = reg_t0.xxx * reg_t4.xyz;
    reg_t1.yzw = reg_t1.yzw * reg_t2.xyz;
    reg_t2.xyz = (reg_t4.xyz * reg_t0.xxx ) + (-reg_t1.yzw);
    reg_t1.yzw = (reg_t2.xyz * reg_const28.www ) + reg_t1.yzw;
    reg_t1.yzw = reg_t1.yzw * reg_const23.xxx;
    reg_t2 = dx9texld(reg_sampler4_deftype, reg_sampler4_srgb_flag, reg_sampler4t, reg_sampler4s, inp.reg_inp0);
    reg_t1.yzw = reg_t1.yzw * reg_t2.yyy;
    reg_t2.xyz = reg_t2.yyy * reg_t5.xyz;
    reg_t0.xyz = (reg_t3.xyz * reg_t1.yzw ) + reg_t0.yzw;
	
	//p2
    reg_t0.w = reg_t1.x + reg_const29.x;
    reg_t1.x = dx9_log(reg_t1.x);
    {
        float tmpCmpVec = 0;
        reg_t0.w = (reg_t0.w >= tmpCmpVec) ? reg_t1.x : reg_const29.y;
    }
    reg_t0.w = reg_t0.w * reg_t3.w;
    reg_t0.w = dx9_exp(reg_t0.w);
    reg_t0.w = reg_t0.w * (-reg_const25.x);
    reg_t1.xyz = reg_t0.www * reg_const8.xyz;
    reg_t0.xyz = (reg_t1.xyz * reg_t2.xyz ) + reg_t0.xyz;
	
	//p1
    reg_t0.w = reg_const24.x + (-inp.reg_inp7.w);
    ret.reg_clr_out0.xyz = (reg_t0.xyz * reg_t0.www ) + inp.reg_inp7.xyz;
    ret.reg_clr_out0.w = reg_const0.x;
	    
    dx9_ps_write_emulation(dx9_ret_color_reg_ac);
    
    return ret;
    
}
