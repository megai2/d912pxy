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

#define API_OVERHEAD_TRACK_LOCAL_ID_DEFINE PXY_METRICS_API_OVERHEAD_DEVICE_DRAWING

//scene terminators

HRESULT d912pxy_device::BeginScene(void)
{
	return D3D_OK;
}

HRESULT d912pxy_device::EndScene(void)
{
	d912pxy_s.render.iframe.EndSceneReset();

	return D3D_OK;
}

//drawers

HRESULT d912pxy_device::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
{ 
	if (1)
	{
		LOG_DBG_DTDM3("DP NON INDEXED SKIPPING");
		return D3D_OK;
	}
}

HRESULT d912pxy_device::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{ 	
	d912pxy_s.render.iframe.CommitBatch(PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);

#ifdef PER_BATCH_FLUSH_DEBUG
	replayer->Finish();

	iframe->End();
	mGPUque->Flush(0);

	iframe->Start();
#endif
		
	return D3D_OK;
}

HRESULT d912pxy_device::DrawIndexedPrimitive_Compat(D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
	d912pxy_s.render.iframe.CommitBatch2(PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);

#ifdef PER_BATCH_FLUSH_DEBUG
	replayer->Finish();

	iframe->End();
	mGPUque->Flush(0);

	iframe->Start();
#endif

	

	return D3D_OK;
}


HRESULT d912pxy_device::DrawIndexedPrimitive_PS(D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
	

#ifdef PER_BATCH_FLUSH_DEBUG
	replayer->Finish();

	iframe->End();
	mGPUque->Flush(0);

	iframe->Start();
#endif

	for (int i = 0; i != 32; ++i)
		if ((stageFormatsTrack[i] == D3DFMT_D24X8) || (stageFormatsTrack[i] == D3DFMT_D24S8))
		{
			if (d912pxy_s.render.db.pso.GetPShader())
				TrackShaderCodeBugs(PXY_INNER_SHDR_BUG_PCF_SAMPLER, i + 1, d912pxy_s.render.db.pso.GetPShader()->GetID());
		}
	
	UINT srgbState = d912pxy_s.render.tex.GetTexStage(30);
	if (srgbState)
		for (int i = 0; i != 32; ++i)
		{
			if (srgbState & 1)
			{
				if (d912pxy_s.render.tex.GetTexStage(i) != mNullTextureSRV)
				{
					if (d912pxy_s.render.db.pso.GetPShader())
						TrackShaderCodeBugs(PXY_INNER_SHDR_BUG_SRGB_READ, 1, d912pxy_s.render.db.pso.GetPShader()->GetID());
					break;
				}
			}
			srgbState = srgbState >> 1;
		}

	if (d912pxy_s.render.db.pso.GetDX9RsValue(D3DRS_SRGBWRITEENABLE))
		if (d912pxy_s.render.db.pso.GetPShader())
			TrackShaderCodeBugs(PXY_INNER_SHDR_BUG_SRGB_WRITE, 1, d912pxy_s.render.db.pso.GetPShader()->GetID());

	if (d912pxy_s.render.db.pso.GetDX9RsValue(D3DRS_ALPHATESTENABLE))
		if (d912pxy_s.render.db.pso.GetPShader())
			TrackShaderCodeBugs(PXY_INNER_SHDR_BUG_ALPHA_TEST, 1, d912pxy_s.render.db.pso.GetPShader()->GetID());

	if (d912pxy_s.render.db.pso.GetDX9RsValue(D3DRS_CLIPPLANEENABLE))
	{
		UINT32 cp = d912pxy_s.render.db.pso.GetDX9RsValue(D3DRS_CLIPPLANEENABLE);
		if (cp & 1)
		{
			if (d912pxy_s.render.db.pso.GetVShader())
				TrackShaderCodeBugs(PXY_INNER_SHDR_BUG_CLIPPLANE0, 1, d912pxy_s.render.db.pso.GetVShader()->GetID());
		}
	}

	d912pxy_vdecl* vdcl = d912pxy_s.render.db.pso.GetIAFormat();

	if (vdcl)
	{
		UINT numElms = 0;
		D3DVERTEXELEMENT9* vdArr = vdcl->GetDeclarationPtr(&numElms);

		for (int i = 0; i != numElms; ++i)
		{
			if ((vdArr[i].Usage == D3DDECLUSAGE_NORMAL) && (vdArr[i].Type == D3DDECLTYPE_UBYTE4))
				if (d912pxy_s.render.db.pso.GetVShader())
					TrackShaderCodeBugs(PXY_INNER_SHDR_BUG_UINT_NORMALS, 1, d912pxy_s.render.db.pso.GetVShader()->GetID());
			if ((vdArr[i].Usage == D3DDECLUSAGE_TANGENT) && (vdArr[i].Type == D3DDECLTYPE_UBYTE4))
				if (d912pxy_s.render.db.pso.GetVShader())
					TrackShaderCodeBugs(PXY_INNER_SHDR_BUG_UINT_TANGENTS, 1, d912pxy_s.render.db.pso.GetVShader()->GetID());
		}		
	}

	d912pxy_s.render.iframe.CommitBatch2(PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);

	

	return D3D_OK;
}

//megai2: you should know, that there is no apps, that can't storage their data in vertex buffers 
HRESULT d912pxy_device::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	d912pxy_s.render.draw_up.DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);

	return D3D_OK;
}


HRESULT d912pxy_device::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	d912pxy_s.render.draw_up.DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);

	return D3D_OK;
}


#undef API_OVERHEAD_TRACK_LOCAL_ID_DEFINE 