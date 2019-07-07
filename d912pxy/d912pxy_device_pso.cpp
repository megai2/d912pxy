/*
MIT License

Copyright(c) 2018-2019 megai2

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
#include "stdafx.h"

#define API_OVERHEAD_TRACK_LOCAL_ID_DEFINE PXY_METRICS_API_OVERHEAD_DEVICE_PSO

HRESULT d912pxy_device::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
{ 
	LOG_DBG_DTDM("RS %u = %u", State, Value);

	API_OVERHEAD_TRACK_START(0)

	/*if (State > D3DRS_BLENDOPALPHA)
		return D3DERR_INVALIDCALL;*/

	switch (State)
	{
		case D3DRS_ENABLE_D912PXY_API_HACKS:
			return 343434;
		break;
		case D3DRS_D912PXY_SETUP_PSO:
			d912pxy_s(psoCache)->UseCompiled(0);
			d912pxy_s(psoCache)->MarkDirty(0);
		break;
		case D3DRS_D912PXY_GPU_WRITE:
			gpuWriteDsc = Value;
		break;
		case D3DRS_D912PXY_DRAW:
			if (Value == 0)
			{
				d912pxy_s(textureState)->Use();
			}
			else {
				d912pxy_s(textureState)->AddDirtyFlag(Value);
			}
		break;
		case D3DRS_BLENDFACTOR:
		{
			DWORD Color = Value;

			float fvClra[4];

			for (int i = 0; i != 4; ++i)
			{
				fvClra[i] = ((Color >> (i << 3)) & 0xFF) / 255.0f;
			}

			d912pxy_s(CMDReplay)->OMBlendFac(fvClra);
		}
		break; //193,   /* D3DCOLOR used for a constant blend factor during alpha blending for devices that support D3DPBLENDCAPS_BLENDFACTOR */
		
//megai2: various fixed pipeline states and obsolete functions that noone use nowdays, but must be done sometime
//also things that overly complex and don't used by target app
		case D3DRS_SHADEMODE: LOG_DBG_DTDM("RS shademode unimpl"); break; //9,    /* D3DSHADEMODE */
		case D3DRS_LASTPIXEL: LOG_DBG_DTDM("RS lastpixel unimpl"); break; //16,   /* TRUE for last-pixel on lines */
		case D3DRS_DITHERENABLE: LOG_DBG_DTDM("RS dither unimpl"); break; //26,   /* TRUE to enable dithering */
		case D3DRS_FOGENABLE: LOG_DBG_DTDM("RS fog unimpl"); break; //28,   /* TRUE to enable fog blending */
		case D3DRS_SPECULARENABLE: LOG_DBG_DTDM("RS fog unimpl"); break; //29,   /* TRUE to enable specular */
		case D3DRS_FOGCOLOR: LOG_DBG_DTDM("RS fog unimpl"); break; //34,   /* D3DCOLOR */
		case D3DRS_FOGTABLEMODE: LOG_DBG_DTDM("RS fog unimpl"); break; //35,   /* D3DFOGMODE */
		case D3DRS_FOGSTART: LOG_DBG_DTDM("RS fog unimpl"); break; //36,   /* Fog start (for both vertex and pixel fog) */
		case D3DRS_FOGEND: LOG_DBG_DTDM("RS fog unimpl"); break; //37,   /* Fog end      */
		case D3DRS_FOGDENSITY: LOG_DBG_DTDM("RS fog unimpl"); break; //38,   /* Fog density  */
		case D3DRS_RANGEFOGENABLE: LOG_DBG_DTDM("RS fog unimpl"); break; //48,   /* Enables range-based fog */
		case D3DRS_WRAP0: LOG_DBG_DTDM("RS wrapN unimpl"); break; //128,  /* wrap for 1st texture coord. set */
		case D3DRS_WRAP1: LOG_DBG_DTDM("RS wrapN unimpl"); break; //129,  /* wrap for 2nd texture coord. set */
		case D3DRS_WRAP2: LOG_DBG_DTDM("RS wrapN unimpl"); break; //130,  /* wrap for 3rd texture coord. set */
		case D3DRS_WRAP3: LOG_DBG_DTDM("RS wrapN unimpl"); break; //131,  /* wrap for 4th texture coord. set */
		case D3DRS_WRAP4: LOG_DBG_DTDM("RS wrapN unimpl"); break; //132,  /* wrap for 5th texture coord. set */
		case D3DRS_WRAP5: LOG_DBG_DTDM("RS wrapN unimpl"); break; //133,  /* wrap for 6th texture coord. set */
		case D3DRS_WRAP6: LOG_DBG_DTDM("RS wrapN unimpl"); break; //134,  /* wrap for 7th texture coord. set */
		case D3DRS_WRAP7: LOG_DBG_DTDM("RS wrapN unimpl"); break; //135,  /* wrap for 8th texture coord. set */
		case D3DRS_WRAP8: LOG_DBG_DTDM("RS wrapN unimpl"); break; //198,   /* Additional wrap states for vs_3_0+ attributes with D3DDECLUSAGE_TEXCOORD */
		case D3DRS_WRAP9: LOG_DBG_DTDM("RS wrapN unimpl"); break; //199,
		case D3DRS_WRAP10: LOG_DBG_DTDM("RS wrapN unimpl"); break; //200,
		case D3DRS_WRAP11: LOG_DBG_DTDM("RS wrapN unimpl"); break; //201,
		case D3DRS_WRAP12: LOG_DBG_DTDM("RS wrapN unimpl"); break; //202,
		case D3DRS_WRAP13: LOG_DBG_DTDM("RS wrapN unimpl"); break; //203,
		case D3DRS_WRAP14: LOG_DBG_DTDM("RS wrapN unimpl");  break; //204,
		case D3DRS_WRAP15: LOG_DBG_DTDM("RS wrapN unimpl"); break; //205,
		case D3DRS_TEXTUREFACTOR: LOG_DBG_DTDM("RS texturefactor unimpl"); break; //60,   /* D3DCOLOR used for multi-texture blend */
		case D3DRS_CLIPPING: LOG_DBG_DTDM("RS clipping unimpl"); break; //136,
		case D3DRS_LIGHTING: LOG_DBG_DTDM("RS unimpl"); break; //137,
		case D3DRS_AMBIENT: LOG_DBG_DTDM("RS unimpl 139"); break;
		case D3DRS_FOGVERTEXMODE: LOG_DBG_DTDM("RS unimpl 140"); break;
		case D3DRS_COLORVERTEX: LOG_DBG_DTDM("RS unimpl 141"); break;
		case D3DRS_LOCALVIEWER: LOG_DBG_DTDM("RS unimpl 142"); break;
		case D3DRS_NORMALIZENORMALS: LOG_DBG_DTDM("RS unimpl 143"); break;
		case D3DRS_DIFFUSEMATERIALSOURCE: LOG_DBG_DTDM("RS unimpl 145"); break;
		case D3DRS_SPECULARMATERIALSOURCE: LOG_DBG_DTDM("RS unimpl 146"); break;
		case D3DRS_AMBIENTMATERIALSOURCE: LOG_DBG_DTDM("RS unimpl 147"); break;
		case D3DRS_EMISSIVEMATERIALSOURCE: LOG_DBG_DTDM("RS unimpl 148"); break;
		case D3DRS_VERTEXBLEND: LOG_DBG_DTDM("RS unimpl 151"); break;		
		case D3DRS_POINTSIZE: LOG_DBG_DTDM("RS unimpl 154"); break;   /* float point size */
		case D3DRS_POINTSIZE_MIN: LOG_DBG_DTDM("RS unimpl 155"); break;   /* float point size min threshold */
		case D3DRS_POINTSPRITEENABLE: LOG_DBG_DTDM("RS unimpl 156"); break;   /* BOOL point texture coord control */
		case D3DRS_POINTSCALEENABLE: LOG_DBG_DTDM("RS unimpl 157"); break;   /* BOOL point size scale enable */
		case D3DRS_POINTSCALE_A: LOG_DBG_DTDM("RS unimpl 158"); break;   /* float point attenuation A value */
		case D3DRS_POINTSCALE_B: LOG_DBG_DTDM("RS unimpl 159"); break;   /* float point attenuation B value */
		case D3DRS_POINTSCALE_C: LOG_DBG_DTDM("RS unimpl 160"); break;   /* float point attenuation C value */			
		case D3DRS_MULTISAMPLEANTIALIAS: LOG_DBG_DTDM("RS unimpl 161"); break;  // BOOL - set to do FSAA with multisample buffer
		case D3DRS_MULTISAMPLEMASK: LOG_DBG_DTDM("RS unimpl 162"); break;  // DWORD - per-sample enable/disable		
		case D3DRS_PATCHEDGESTYLE: LOG_DBG_DTDM("RS unimpl 163"); break;  // Sets whether patch edges will use float style tessellation
		case D3DRS_DEBUGMONITORTOKEN: LOG_DBG_DTDM("RS unimpl 165"); break;  // DEBUG ONLY - token to debug monitor
		case D3DRS_POINTSIZE_MAX: LOG_DBG_DTDM("RS unimpl 166"); break;   /* float point size max threshold */
		case D3DRS_INDEXEDVERTEXBLENDENABLE: LOG_DBG_DTDM("RS unimpl 167"); break;
		case D3DRS_TWEENFACTOR: LOG_DBG_DTDM("RS unimpl 170"); break;   // float tween factor
		case D3DRS_POSITIONDEGREE: LOG_DBG_DTDM("RS unimpl 172"); break;   // NPatch position interpolation degree. D3DDEGREE_LINEAR or D3DDEGREE_CUBIC (default)
		case D3DRS_NORMALDEGREE: LOG_DBG_DTDM("RS unimpl 173"); break;   // NPatch normal interpolation degree. D3DDEGREE_LINEAR (default) or D3DDEGREE_QUADRATIC		
		case D3DRS_MINTESSELLATIONLEVEL: LOG_DBG_DTDM("RS unimpl 178"); break;
		case D3DRS_MAXTESSELLATIONLEVEL: LOG_DBG_DTDM("RS unimpl 179"); break;
		case D3DRS_ADAPTIVETESS_X: LOG_DBG_DTDM("RS unimpl 180"); break;
		case D3DRS_ADAPTIVETESS_Y: LOG_DBG_DTDM("RS unimpl 181"); break;
		case D3DRS_ADAPTIVETESS_Z: LOG_DBG_DTDM("RS unimpl 182"); break;
		case D3DRS_ADAPTIVETESS_W: LOG_DBG_DTDM("RS unimpl 183"); break;
	//	case D3DRS_SRGBWRITEENABLE: LOG_DBG_DTDM("RS unimpl 194"); break;   /* Enable rendertarget writes to be DE-linearized to SRGB (for formats that expose D3DUSAGE_QUERY_SRGBWRITE) */
		case D3DRS_ENABLEADAPTIVETESSELLATION: LOG_DBG_DTDM("RS unimpl 184"); break;

		default:
			d912pxy_s(psoCache)->State(State,Value);
	}

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK; 
}

HRESULT d912pxy_device::GetRenderState(D3DRENDERSTATETYPE State, DWORD* pValue)
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)
	
	switch (State)
	{
	case D3DRS_D912PXY_ENQUEUE_PSO_COMPILE:
		d912pxy_s(psoCache)->UseWithFeedbackPtr((void**)pValue);
		break;
	case D3DRS_D912PXY_SETUP_PSO:
		d912pxy_s(psoCache)->UseCompiled((d912pxy_pso_cache_item*)pValue);
		break;
	case D3DRS_D912PXY_GPU_WRITE:
		d912pxy_s(batch)->GPUWrite((void*)pValue, gpuWriteDsc & 0xFFFF, (gpuWriteDsc >> 16));
		break;
	case D3DRS_D912PXY_SAMPLER_ID:
		d912pxy_s(textureState)->Use();
		*pValue = d912pxy_s(textureState)->GetCurrent()->splHeapID[*pValue];
		break;
	default:
		*pValue = d912pxy_s(psoCache)->GetDX9RsValue(State);		
	}

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK;
}

HRESULT d912pxy_device::BeginStateBlock(void) 
{ 
	LOG_DBG_DTDM(__FUNCTION__);
	return D3D_OK; 
}

HRESULT d912pxy_device::EndStateBlock(IDirect3DStateBlock9** ppSB) 
{ 
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)
			
	*ppSB = PXY_COM_CAST_(IDirect3DStateBlock9, d912pxy_sblock::d912pxy_sblock_com(D3DSBT_ALL));

	API_OVERHEAD_TRACK_END(0)

	return D3D_OK; 
}

#undef API_OVERHEAD_TRACK_LOCAL_ID_DEFINE 