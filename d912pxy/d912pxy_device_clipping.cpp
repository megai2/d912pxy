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

#define API_OVERHEAD_TRACK_LOCAL_ID_DEFINE PXY_METRICS_API_OVERHEAD_DEVICE_CLIPPING

HRESULT d912pxy_device::SetClipPlane(DWORD Index, CONST float* pPlane)
{
	if (Index > 0)
		return D3D_OK;

	
	d912pxy_s.render.batch.SetShaderConstF(1, PXY_INNER_EXTRA_SHADER_CONST_CLIP_P0, 1, (float*)pPlane);  
	return D3D_OK;	
}

//scissors

HRESULT d912pxy_device::SetScissorRect(CONST RECT* pRect)
{			
	d912pxy_s.render.iframe.SetScissors((D3D12_RECT*)pRect);
	
	return D3D_OK;
}

HRESULT d912pxy_device::SetViewport(CONST D3DVIEWPORT9* pViewport)
{
	D3D12_VIEWPORT main_viewport;
	main_viewport.Height = pViewport->Height * 1.0f;
	main_viewport.Width = pViewport->Width * 1.0f;
	main_viewport.TopLeftX = pViewport->X * 1.0f;
	main_viewport.TopLeftY = pViewport->Y * 1.0f;
	main_viewport.MaxDepth = pViewport->MaxZ;
	main_viewport.MinDepth = pViewport->MinZ;

	d912pxy_s.render.iframe.SetViewport(&main_viewport);

	return D3D_OK;
}

HRESULT d912pxy_device::SetViewport_CAR(const D3DVIEWPORT9 * pViewport)
{
	D3D12_VIEWPORT main_viewport;
	main_viewport.Height = pViewport->Height * 1.0f;
	main_viewport.Width = pViewport->Width * 1.0f;
	main_viewport.TopLeftX = pViewport->X * 1.0f;
	main_viewport.TopLeftY = pViewport->Y * 1.0f;
	main_viewport.MaxDepth = pViewport->MaxZ;
	main_viewport.MinDepth = pViewport->MinZ;

	d912pxy_s.render.iframe.SetViewportIfChanged(&main_viewport);

	return D3D_OK;
}

HRESULT d912pxy_device::SetScissorRect_CAR(const RECT * pRect)
{
	d912pxy_s.render.iframe.SetScissorsIfChanged((D3D12_RECT*)pRect);

	return D3D_OK;
}

#undef API_OVERHEAD_TRACK_LOCAL_ID_DEFINE 