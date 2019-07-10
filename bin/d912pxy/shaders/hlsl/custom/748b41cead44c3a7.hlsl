/*
    vs_3_0
    def c14, 0, 1, 0, 0
    dcl_position v0
    dcl_normal v3
    dcl_tangent v4
    dcl_binormal v5
    dcl_texcoord v7
    dcl_texcoord1 v8
    dcl_texcoord2 v9
    dcl_texcoord o0
    dcl_texcoord1 o1
    dcl_texcoord2 o2
    dcl_position o3
    dp4 o3.x, v0, c0
    dp4 o3.y, v0, c1
    dp4 o3.z, v0, c2
    dp4 o3.w, v0, c3
    mov o0.xy, v7.xyxy
    mov r0, c4
    mul r1, r0.x, c5
    mad r1, r0.y, c6, r1
    mad r1, r0.z, c7, r1
    dp3 r0.x, v4, r1
    dp3 r0.y, v5, r1
    dp3 r0.z, v3, r1
    dp3 r0.w, r0, r0
    rsq r0.w, r0.w
    mul r0.xyz, r0, r0.w
    mov o1.xyz, r0
    dp4 r0.x, v0, c5
    dp4 r0.y, v0, c6
    dp4 r0.z, v0, c7
    mov r0.w, c14.y
    add r3.xyz, r0.xyzz, -c13.xyzz
    dp3 r3.w, r3, r3
    rsq r3.w, r3.w
    rcp r3.x, r3.w
    mov r3.y, r0.z
    mul r3.z, r3.x, c11.x
    min r3.z, r3.z, c14.y
    mad r3.xy, r3.xyxw, c12.xzzw, c12.ywww
    expp r3.x, -r3.x
    expp r3.y, -r3.y
    min r3.xy, r3, c14.y
    max r3.xy, r3, c14.x
    add r3.xy, c14.y, -r3
    mul r3.y, r3.y, r3.z
    add r3.z, c14.y, -r3.x
    mov r2, c9
    mad r2, r3.z, c8, r2
    min r4.w, r3.x, r2.w
    mul r4.xyz, r2, r4.w
    min r5.w, r3.y, c10.w
    mul r5.xyz, c10, r5.w
    add r3.x, c14.y, -r3.x
    mad r1, r5, r3.x, r4
    mov o2, r1

// approximately 44 instruction slots used

*/
#define dx9_texture_srgb_read(a,b) 
#include "../../common.hlsli"
	
struct VS_INPUT
{
	float4 reg_inp0: POSN0E;
	float4 reg_inp3: NORM0E;
	float4 reg_inp4: TANG0E;
	float4 reg_inp5: BINO0E;
	float4 reg_inp7: TEXC0E;
	float4 reg_inp8: TEXC1E;
	float4 reg_inp9: TEXC2E;
};
	
struct VS_OUTPUT
{
	float4 reg_tc_vo3: SV_POSITION;
	float4 reg_tc_vo0: TEXCOORD0;
	float4 reg_tc_vo1: TEXCOORD1;
	float4 reg_tc_vo2: TEXCOORD2;
};
 
VS_OUTPUT main(VS_INPUT inp)
{
	VS_OUTPUT ret;
	
	#define dx9_halfpixel_pos_reg_ac ret.reg_tc_vo3
	float4 reg_const0 = getPassedVSFv(0);
	float4 reg_const1 = getPassedVSFv(1);
	float4 reg_const2 = getPassedVSFv(2);
	float4 reg_const3 = getPassedVSFv(3);
	float4 reg_t0 = { 0, 0, 0, 0 };
	float4 reg_const4 = getPassedVSFv(4);
	float4 reg_t1 = { 0, 0, 0, 0 };
	float4 reg_const5 = getPassedVSFv(5);
	float4 reg_const6 = getPassedVSFv(6);
	float4 reg_const7 = getPassedVSFv(7);
	float4 reg_t3 = { 0, 0, 0, 0 };
	float4 reg_const13 = getPassedVSFv(13);
	float4 reg_const11 = getPassedVSFv(11);
	float4 reg_const12 = getPassedVSFv(12);
	float4 reg_t2 = { 0, 0, 0, 0 };
	float4 reg_const9 = getPassedVSFv(9);
	float4 reg_const8 = getPassedVSFv(8);
	float4 reg_t4 = { 0, 0, 0, 0 };
	float4 reg_t5 = { 0, 0, 0, 0 };
	float4 reg_const10 = getPassedVSFv(10);
	 
	
    float4 reg_const14 = { 0.000000 , 1.000000 , 0.000000 , 0.000000 };

	//Fixes bad footprint cases
	reg_const4.x = reg_const4.x + 0.000000001;// 
	reg_const4.y = reg_const4.y + 0.000000001;//
	reg_const4.z = reg_const4.z + 0.000000001;//
	reg_const4.w = reg_const4.w + 0.000000001;//

    ret.reg_tc_vo3.x = dot(inp.reg_inp0,reg_const0);
    ret.reg_tc_vo3.y = dot(inp.reg_inp0,reg_const1);
    ret.reg_tc_vo3.z = dot(inp.reg_inp0,reg_const2);
    ret.reg_tc_vo3.w = dot(inp.reg_inp0,reg_const3);
    ret.reg_tc_vo0.xy = inp.reg_inp7.xy;
    reg_t0 = reg_const4;
    reg_t1 = reg_t0.xxxx * reg_const5;
    reg_t1 = (reg_t0.yyyy * reg_const6 ) + reg_t1;
    reg_t1 = (reg_t0.zzzz * reg_const7 ) + reg_t1;
    reg_t0.x = dot(inp.reg_inp4.xyz,reg_t1.xyz);
    reg_t0.y = dot(inp.reg_inp5.xyz,reg_t1.xyz);
    reg_t0.z = dot(inp.reg_inp3.xyz,reg_t1.xyz);
    reg_t0.w = dot(reg_t0.xyz,reg_t0.xyz);
    reg_t0.w = dx9_rsqrt(reg_t0.w);
    reg_t0.xyz = reg_t0.xyz * reg_t0.www;
    ret.reg_tc_vo1.xyz = reg_t0.xyz;
    reg_t0.x = dot(inp.reg_inp0,reg_const5);
    reg_t0.y = dot(inp.reg_inp0,reg_const6);
    reg_t0.z = dot(inp.reg_inp0,reg_const7);
    reg_t0.w = reg_const14.y;
    reg_t3.xyz = reg_t0.xyz + (-reg_const13.xyz);
    reg_t3.w = dot(reg_t3.xyz,reg_t3.xyz);
    reg_t3.w = dx9_rsqrt(reg_t3.w);
    reg_t3.x = dx9_rcp(reg_t3.w);
    reg_t3.y = reg_t0.z;
    reg_t3.z = reg_t3.x * reg_const11.x;
    reg_t3.z = dx9_min(reg_t3.z, reg_const14.y);
    reg_t3.xy = (reg_t3.xy * reg_const12.xz ) + reg_const12.yw;
    reg_t3.x = dx9_expp((-reg_t3.x));
    reg_t3.y = dx9_expp((-reg_t3.y));
    reg_t3.xy = dx9_min(reg_t3.xy, reg_const14.yy);
    reg_t3.xy = dx9_max(reg_t3.xy, reg_const14.xx);
    reg_t3.xy = reg_const14.yy + (-reg_t3.xy);
    reg_t3.y = reg_t3.y * reg_t3.z;
    reg_t3.z = reg_const14.y + (-reg_t3.x);
    reg_t2 = reg_const9;
    reg_t2 = (reg_t3.zzzz * reg_const8 ) + reg_t2;
    reg_t4.w = dx9_min(reg_t3.x, reg_t2.w);
    reg_t4.xyz = reg_t2.xyz * reg_t4.www;
    reg_t5.w = dx9_min(reg_t3.y, reg_const10.w);
    reg_t5.xyz = reg_const10.xyz * reg_t5.www;
    reg_t3.x = reg_const14.y + (-reg_t3.x);
    reg_t1 = (reg_t5 * reg_t3.xxxx ) + reg_t4;
    ret.reg_tc_vo2 = reg_t1;
    
    dx9_halfpixel_pos_reg_ac = dx9_fix_halfpixel_offset(dx9_halfpixel_pos_reg_ac);
    
    return ret;
    
}
