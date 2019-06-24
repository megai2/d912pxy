#define dx9_texture_srgb_read(a,b) 
#include "../../common.hlsli"
	
struct PS_INPUT
{
	float4 reg_misc0: SV_POSITION;
	float4 color: COLOR;
};
	
struct PS_OUTPUT
{
	float4 reg_clr_out0: SV_TARGET;
};
 
PS_OUTPUT main(PS_INPUT inp)
{ 
	PS_OUTPUT ret; 
	
    ret.reg_clr_out0.xyzw = inp.color;
        
    return ret;
    
}
