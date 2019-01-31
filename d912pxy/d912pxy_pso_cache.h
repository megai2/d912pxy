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
#pragma once
#include "stdafx.h"

#define d912pxy_trimmed_dx12_pso_hash_offset (8*3)

typedef struct d912pxy_trimmed_dx12_pso {
	d912pxy_vshader* VS;
	d912pxy_pshader* PS;
	d912pxy_vdecl* InputLayout;

	UINT vdeclHash;
	D3D12_RENDER_TARGET_BLEND_DESC BlendStateRT0;
	D3D12_RASTERIZER_DESC RasterizerState;
	D3D12_DEPTH_STENCIL_DESC DepthStencilState;
	DXGI_FORMAT RTVFormat0;
	DXGI_FORMAT DSVFormat;
	UINT NumRenderTargets;
} d912pxy_trimmed_dx12_pso;

class d912pxy_pso_cache_item : public d912pxy_noncom {

public:
	d912pxy_pso_cache_item(d912pxy_device* dev, d912pxy_trimmed_dx12_pso* sDsc);

	void Compile();

	~d912pxy_pso_cache_item()
	{
		obj = nullptr;
	};

	//UINT Status() { return m_status; };

	ID3D12PipelineState* GetPtr() { return retPtr; };

private:

	ID3D12PipelineState * retPtr;
	ComPtr<ID3D12PipelineState> obj;

	d912pxy_trimmed_dx12_pso* desc;
	//UINT m_status;
};

class d912pxy_pso_cache :
	public d912pxy_noncom, public d912pxy_thread
{
public:
	d912pxy_pso_cache(d912pxy_device* dev);
	~d912pxy_pso_cache();

	//things that affect pso only
	void State(D3DRENDERSTATETYPE State, DWORD Value);
	void VShader(d912pxy_vshader* vs);
	void PShader(d912pxy_pshader* ps);
	void IAFormat(d912pxy_vdecl* vertexDcl);
	void IAFormatInstanced(d912pxy_vdecl* vertexDcl);

	//things that affect pso but indirectly

	void RTVFormat(DXGI_FORMAT fmt, UINT idx);
	void DSVFormat(DXGI_FORMAT fmt);
	void OMReflect(UINT RTcnt, D3D12_CPU_DESCRIPTOR_HANDLE* dsv);

	DWORD GetDX9RsValue(D3DRENDERSTATETYPE State) { return DX9RSvalues[State]; };
	UINT Use();
	UINT UseCompiled(d912pxy_pso_cache_item* it);
	UINT UseWithFeedbackPtr(void** feedback);

	d912pxy_pso_cache_item* UseByDesc(d912pxy_trimmed_dx12_pso* dsc, UINT32 frameStartTime);

	void SetRootSignature(ComPtr<ID3D12RootSignature> sig);

	void MarkDirty(UINT force);

	d912pxy_vdecl* GetIAFormat() {
		return mVDecl;
	};

	d912pxy_pshader* GetPShader();
	d912pxy_vshader* GetVShader();

	void ThreadJob();

	void QueueShaderCleanup(d912pxy_shader* v);
	void ProcessShaderCleanup();

	static D3D12_GRAPHICS_PIPELINE_STATE_DESC cDscBase;

	static UINT psMaxVars;
	static UINT vsMaxVars;

	void CompileItem(d912pxy_pso_cache_item* item);

	UINT IsCompileQueueFree();

private:
	d912pxy_memtree2* cacheIndexes;
	UINT32 cacheIncID;
		
	d912pxy_ringbuffer<d912pxy_shader*>* shaderCleanupBuffer;

	d912pxy_vdecl* mVDecl;

	ID3D12PipelineState* psoPtr;

	UINT8 dirty;
	d912pxy_trimmed_dx12_pso cDsc;	
	DWORD DX9RSvalues[D3DRS_BLENDOPALPHA + 1];

	HANDLE psoCompileThread;
	d912pxy_ringbuffer<d912pxy_pso_cache_item*>* psoCompileBuffer;

	ID3D12GraphicsCommandList* frameCl;
};

