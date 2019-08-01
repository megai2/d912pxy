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

HRESULT d912pxy_device::SetRenderState_Tracked(D3DRENDERSTATETYPE State, DWORD Value)
{
	d912pxy_s.render.db.pso.SetStateTracked(State, Value);
	return D3D_OK;
}


HRESULT d912pxy_device::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
{ 			
	switch (State)
	{
		case D3DRS_ENABLE_D912PXY_API_HACKS:
			return 343434;
		break;
		case D3DRS_D912PXY_SETUP_PSO:
			d912pxy_s.render.db.pso.UseCompiled(0);
			d912pxy_s.render.db.pso.MarkDirty(0);
		break;
		case D3DRS_D912PXY_GPU_WRITE:
			gpuWriteDsc = Value;
		break;
		case D3DRS_D912PXY_DRAW:
			if (Value == 0)
			{
				d912pxy_s.render.tex.Use();
			}
			else {
				d912pxy_s.render.tex.AddDirtyFlag(Value);
			}
		break;					
		default:
			d912pxy_s.render.db.pso.SetState(State,Value);
	}
	
	return D3D_OK; 
}

HRESULT d912pxy_device::GetRenderState(D3DRENDERSTATETYPE State, DWORD* pValue)
{ 		
	switch (State)
	{
	case D3DRS_D912PXY_ENQUEUE_PSO_COMPILE:
		d912pxy_s.render.db.pso.UseWithFeedbackPtr((void**)pValue);
		break;
	case D3DRS_D912PXY_SETUP_PSO:
		d912pxy_s.render.db.pso.UseCompiled((d912pxy_pso_cache_item*)pValue);
		break;
	case D3DRS_D912PXY_GPU_WRITE:
		d912pxy_s.render.batch.GPUWrite((void*)pValue, gpuWriteDsc & 0xFFFF, (gpuWriteDsc >> 16));
		break;
	case D3DRS_D912PXY_SAMPLER_ID:
		d912pxy_s.render.tex.Use();
		*pValue = d912pxy_s.render.tex.GetCurrent()->splHeapID[*pValue];
		break;
	default:
		*pValue = d912pxy_s.render.db.pso.GetDX9RsValue(State);		
	}

	return D3D_OK;
}

HRESULT d912pxy_device::BeginStateBlock(void) 
{ 
	return D3D_OK; 
}

HRESULT d912pxy_device::EndStateBlock(IDirect3DStateBlock9** ppSB) 
{ 			
	*ppSB = PXY_COM_CAST_(IDirect3DStateBlock9, d912pxy_sblock::d912pxy_sblock_com(D3DSBT_ALL));
	return D3D_OK; 
}

#undef API_OVERHEAD_TRACK_LOCAL_ID_DEFINE 