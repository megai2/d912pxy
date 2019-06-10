#define dx9_texture_srgb_read(a,b) 
#include "../../common.hlsli"
	
struct VS_INPUT
{
	float4 pos: POSN0E;	
};
	
struct VS_OUTPUT
{
	float4 pos: SV_POSITION;
	float4 clr: COLOR;
};
 
VS_OUTPUT main(VS_INPUT inp)
{ 
	VS_OUTPUT ret; 
	
	float4 color = batchData.extraVars[0];
	float4 rect = batchData.extraVars[1];
	float4 zwh = batchData.extraVars[2];
	 	
	float2 pos2d = ((rect.zw - rect.xy) * inp.pos.xy + rect.xy) / zwh.yz;
	
	pos2d = pos2d * 2 - 1;
	
    ret.pos.xy = pos2d;
	ret.pos.z = zwh.x;
	ret.pos.w = 1;
	ret.clr = color.xzyw;
    
      
    return ret;
    
}
