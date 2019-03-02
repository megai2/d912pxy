#include "stdafx.h"

#define API_OVERHEAD_TRACK_LOCAL_ID_DEFINE PXY_METRICS_API_OVERHEAD_DEVICE_CLIPPING

HRESULT WINAPI d912pxy_device::SetClipPlane(DWORD Index, CONST float* pPlane)
{
	API_OVERHEAD_TRACK_START(0)

	d912pxy_s(batch)->SetShaderConstF(1, PXY_INNER_MAX_SHADER_CONSTS_IDX - 2 - Index, 1, (float*)pPlane);  return D3D_OK;

	API_OVERHEAD_TRACK_END(0)
}

//scissors

HRESULT WINAPI d912pxy_device::SetScissorRect(CONST RECT* pRect)
{
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

		d912pxy_s(iframe)->SetScissors((D3D12_RECT*)pRect);

	API_OVERHEAD_TRACK_END(0)

		return D3D_OK;
}

HRESULT WINAPI d912pxy_device::SetViewport(CONST D3DVIEWPORT9* pViewport)
{
	LOG_DBG_DTDM(__FUNCTION__);

	API_OVERHEAD_TRACK_START(0)

		D3D12_VIEWPORT main_viewport;
	main_viewport.Height = pViewport->Height * 1.0f;
	main_viewport.Width = pViewport->Width * 1.0f;
	main_viewport.TopLeftX = pViewport->X * 1.0f;
	main_viewport.TopLeftY = pViewport->Y * 1.0f;
	main_viewport.MaxDepth = pViewport->MaxZ;
	main_viewport.MinDepth = pViewport->MinZ;

	d912pxy_s(iframe)->SetViewport(&main_viewport);

	API_OVERHEAD_TRACK_END(0)

		return D3D_OK;
}

#undef API_OVERHEAD_TRACK_LOCAL_ID_DEFINE 