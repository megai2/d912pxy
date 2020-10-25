/*
MIT License

Copyright(c) 2020 megai2

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

using namespace d912pxy::extras::IFrameMods;

NativeDraw::NativeDraw(ID3D12PipelineState* psoObj, const NativeDrawData& data)
	: pso(psoObj)
{
	ibuf = d912pxy_s.pool.vstream.GetVStreamObject((UINT)data.index.getSize(), D3DFMT_INDEX32, 1);
	ibuf->LoadFromBlock(data.index);
	vbuf = d912pxy_s.pool.vstream.GetVStreamObject((UINT)data.vertex.getSize(), 0, 0);
	ibuf->LoadFromBlock(data.index);
	cbuf = d912pxy_s.pool.vstream.GetVStreamObject((UINT)data.cb0.getSize(), 0, 0);
	vstride = data.vstride;
	indexCount = ((UINT)data.index.getSize() / sizeof(uint32_t));
}

d912pxy::extras::IFrameMods::NativeDraw::~NativeDraw()
{
	pso->Release();
	ibuf->Release();
	vbuf->Release();
	cbuf->Release();
}

void NativeDraw::draw(const d912pxy_replay_thread_context& rpCtx)
{
	StateHolder(rpCtx, StateHolder::ST_PSO | StateHolder::ST_VSTREAM0 | StateHolder::ST_INDEX | StateHolder::ST_PRIMTOPO);
	rpCtx.cl->SetPipelineState(pso);
	ibuf->IFrameBindIB(rpCtx.cl);
	vbuf->IFrameBindVB(vstride, 0, 0, rpCtx.cl);
	rpCtx.cl->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	rpCtx.cl->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
}