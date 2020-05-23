#define dx9_texture_srgb_read(a,b) 
#include "../../common.hlsli"
	
struct VS_INPUT
{
	float4 pos: POSN0E;	
};
	
struct VS_OUTPUT
{	
	float4 tc0: TEXCOORD0;
	float4 pos: SV_POSITION;
};
 
VS_OUTPUT main(VS_INPUT inp)
{ 
	VS_OUTPUT ret; 
		 	
	float2 pos2d = inp.pos.xy;	
	pos2d = pos2d * 2 - 1;
	
    ret.pos.xy = pos2d;
	ret.pos.z = 0;
	ret.pos.w = 1;
    ret.tc0.xy = inp.pos.xy;	
	ret.tc0.z = 0;
	ret.tc0.w = 0;
      
    return ret;
    
}
