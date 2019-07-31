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

#define d912pxy_trimmed_pso_static_data_size (sizeof(d912pxy_trimmed_dx12_pso) - d912pxy_trimmed_dx12_pso_hash_offset)

#pragma pack(push, 1)

typedef struct d912pxy_trimmed_render_target_blend_desc
{
	UINT8 BlendEnable;
	UINT8 SrcBlend;
	UINT8 DestBlend;
	UINT8 BlendOp;
	UINT8 SrcBlendAlpha;
	UINT8 DestBlendAlpha;
	UINT8 BlendOpAlpha;
	UINT8 RenderTargetWriteMask;
} d912pxy_trimmed_render_target_blend_desc;

typedef struct d912pxy_trimmed_rasterizer_desc
{
	UINT8 FillMode;
	UINT8 CullMode;
	INT DepthBias;
	FLOAT SlopeScaledDepthBias;
	UINT8 AntialiasedLineEnable;
} d912pxy_trimmed_rasterizer_desc;

typedef struct d912pxy_trimmed_depth_stencilop_desc
{
	UINT8 StencilFailOp;
	UINT8 StencilDepthFailOp;
	UINT8 StencilPassOp;
	UINT8 StencilFunc;
} d912pxy_trimmed_depth_stencilop_desc;

typedef struct d912pxy_trimmed_depth_stencil_desc
{
	UINT8 DepthEnable;
	UINT8 DepthWriteMask;
	UINT8 DepthFunc;
	UINT8 StencilEnable;
	UINT8 StencilReadMask;
	UINT8 StencilWriteMask;
	d912pxy_trimmed_depth_stencilop_desc FrontFace;
	d912pxy_trimmed_depth_stencilop_desc BackFace;
} d912pxy_trimmed_depth_stencil_desc;

typedef struct d912pxy_trimmed_dx12_pso {
	d912pxy_shader* VS;
	d912pxy_shader* PS;
	d912pxy_vdecl* InputLayout;

	UINT vdeclHash;
	d912pxy_trimmed_render_target_blend_desc BlendStateRT0;
	d912pxy_trimmed_rasterizer_desc RasterizerState;
	d912pxy_trimmed_depth_stencil_desc DepthStencilState;
	UINT8 RTVFormat0;
	UINT8 DSVFormat;
	UINT8 NumRenderTargets;
} d912pxy_trimmed_dx12_pso;

typedef struct d912pxy_serialized_pso_key {
	D3DVERTEXELEMENT9 declData[PXY_INNER_MAX_VDECL_LEN];

	UINT8 staticPsoDesc[d912pxy_trimmed_pso_static_data_size];
	
} d912pxy_serialized_pso_key;

#pragma pack(pop)

class d912pxy_pso_cache_item : public d912pxy_vtable, public d912pxy_comhandler 
{

public:	
	static d912pxy_pso_cache_item* d912pxy_pso_cache_item_com(d912pxy_trimmed_dx12_pso* sDsc);
	~d912pxy_pso_cache_item()
	{
		obj = nullptr;
	};

	void Compile();
	void CreatePSO();
	void CreatePSODerived(UINT64 alias);

	//UINT Status() { return m_status; };

	ID3D12PipelineState* GetPtr() { return retPtr; };

	void RealtimeIntegrityCheck();

private:
	d912pxy_pso_cache_item(d912pxy_trimmed_dx12_pso* sDsc);


	ID3D12PipelineState * retPtr;
	ComPtr<ID3D12PipelineState> obj;

	d912pxy_trimmed_dx12_pso* desc;
	//UINT m_status;
};

#define PXY_PSO_CACHE_KEYFILE_NAME 1
#define PXY_PSO_CACHE_KEYFILE_WRITE 2
#define PXY_PSO_CACHE_KEYFILE_READ 1

typedef struct fv4Color {
	float val[4];
} fv4Color;

class d912pxy_pso_cache :
	public d912pxy_noncom, public d912pxy_thread
{
public:
	d912pxy_pso_cache();
	~d912pxy_pso_cache();

	void Init();

	//things that affect pso only
	void State(D3DRENDERSTATETYPE State, DWORD Value);

	void SetState(D3DRENDERSTATETYPE State, DWORD Value);
	void SetStateTracked(D3DRENDERSTATETYPE State, DWORD Value);

	void VShader(d912pxy_shader* vs);
	void PShader(d912pxy_shader* ps);
	void IAFormat(d912pxy_vdecl* vertexDcl);
	void IAFormatInstanced(d912pxy_vdecl* vertexDcl);

	//things that affect pso but indirectly

	void RTVFormat(DXGI_FORMAT fmt, UINT idx);
	void DSVFormat(DXGI_FORMAT fmt);
	void OMReflect(UINT RTcnt, D3D12_CPU_DESCRIPTOR_HANDLE* dsv);

	DWORD GetDX9RsValue(D3DRENDERSTATETYPE State);
	UINT Use();
	UINT UseCompiled(d912pxy_pso_cache_item* it);
	UINT UseWithFeedbackPtr(void** feedback);

	d912pxy_pso_cache_item* UseByDesc(d912pxy_trimmed_dx12_pso* dsc, UINT32 frameStartTime);
	d912pxy_pso_cache_item* GetByDescMT(d912pxy_trimmed_dx12_pso* dsc, UINT32 frameStartTime);
	ID3D12PipelineState* UseByDescMT(d912pxy_trimmed_dx12_pso* dsc, UINT32 frameStartTime);

	void SetRootSignature(ComPtr<ID3D12RootSignature> sig);

	void MarkDirty(UINT force);

	d912pxy_vdecl* GetIAFormat() {
		return mVDecl;
	};

	d912pxy_shader* GetPShader();
	d912pxy_shader* GetVShader();

	void ThreadJob();

	static D3D12_GRAPHICS_PIPELINE_STATE_DESC cDscBase;

	static UINT allowRealtimeChecks;

	void CompileItem(d912pxy_pso_cache_item* item);

	UINT IsCompileQueueFree();

	void LockCompileQue(UINT lock);

	void LoadCachedData();

	d912pxy_trimmed_dx12_pso* GetCurrentDsc();

	d912pxy_pso_cache_item* GetCurrentCPSO() { return cCPSO; };

	void SaveKeyToCache(UINT64 id, d912pxy_trimmed_dx12_pso * dsc);

	UINT32 GetHashedKey(d912pxy_trimmed_dx12_pso * dsc);

	fv4Color TransformBlendFactor(DWORD val);
	DWORD TransformBlend2AlphaBlend(DWORD val);

private:
	d912pxy_memtree2* cacheIndexes;
	UINT32 cacheIncID;

	d912pxy_pso_cache_item* cCPSO;
		
	d912pxy_vdecl* mVDecl;

	ID3D12PipelineState* psoPtr;

	UINT8 dirty;
	UINT8 fileCacheFlags;
	d912pxy_serialized_pso_key** psoKeyCache;
	d912pxy_trimmed_dx12_pso cDsc;	
	DWORD DX9RSvalues[226];

	HANDLE psoCompileThread;
	d912pxy_ringbuffer<d912pxy_pso_cache_item*>* psoCompileBuffer;

	ID3D12GraphicsCommandList* frameCl;

	d912pxy_thread_lock externalLock;
	d912pxy_thread_lock compileQueLock;

	void CheckExternalLock();
};

