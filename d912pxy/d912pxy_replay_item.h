/*
MIT License

Copyright(c) 2019 megai2

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

class d912pxy_replay_item {

public:
	typedef enum class typeName : UINT32 {
		barrier = 0,
		om_stencilref = 1,
		om_blendfactor,
		view_scissor,
		draw_indexed,
		om_render_targets,
		vbuf_bind,
		ibuf_bind,
		clear_rt,
		clear_ds,
		pso_raw,
		pso_raw_feedback,
		pso_compiled,
		rect_copy,
		gpu_write_ctl,
		ia_prim_topo,
		query_mark,
		_count
	} typeNames;
	
	static const UINT maxDataSize = 256 - sizeof(typeName);
	static const UINT dataAligment = 16;

#pragma pack(push,4)

	typedef struct dt_barrier {
		d912pxy_resource* res;
		D3D12_RESOURCE_STATES to;
		D3D12_RESOURCE_STATES from;
	} dt_barrier;

	typedef struct dt_pso_cached {
		d912pxy_pso_cache_item* cachedPSO;
	} dt_pso_cached;

	typedef struct dt_om_stencilref {
		DWORD dRef;
	} dt_om_stencilref;

	typedef struct dt_om_blendfactor {
		float color[4];
	} dt_om_blendfactor;

	typedef struct dt_view_scissor {
		D3D12_VIEWPORT viewport;
		D3D12_RECT scissor;
	} dt_view_scissor;

	typedef struct dt_draw_indexed {
		UINT IndexCountPerInstance;
		UINT InstanceCount;
		UINT StartIndexLocation;
		INT BaseVertexLocation;
		UINT StartInstanceLocation;
		UINT batchId;
	} dt_draw_indexed;

	typedef struct dt_om_render_targets {
		d912pxy_surface* rtv;
		d912pxy_surface* dsv;
		UINT32 aligment;
	} dt_om_render_targets;

	typedef struct dt_vbuf_bind {
		d912pxy_vstream* buf;
		UINT stride;
		UINT slot;
		UINT offset;
	} dt_vbuf_bind;

	typedef struct dt_ibuf_bind {
		d912pxy_vstream* buf;
	} dt_ibuf_bind;

	typedef struct dt_clear_rt {
		d912pxy_surface* tgt;
		float clr[4];
		D3D12_RECT clearRect;
		D3D12_RESOURCE_STATES cuState;
	} dt_clear_rt;

	typedef struct dt_clear_ds {
		d912pxy_surface* tgt;
		float depth;
		UINT8 stencil;
		D3D12_CLEAR_FLAGS flag;
		D3D12_RECT clearRect;
		D3D12_RESOURCE_STATES cuState;
	} dt_clear_ds;

	typedef struct dt_pso_raw {
		d912pxy_trimmed_dx12_pso rawState;
	} dt_pso_raw;

	typedef struct dt_pso_raw_feedback {
		d912pxy_trimmed_dx12_pso rawState;
		void** feedbackPtr;
	} dt_pso_raw_feedback;

	typedef struct dt_rect_copy {
		d912pxy_surface* src;
		d912pxy_surface* dst;
		D3D12_RESOURCE_STATES prevS;
		D3D12_RESOURCE_STATES prevD;
	} dt_rect_copy;

	typedef struct dt_pso_compiled {
		d912pxy_pso_cache_item* psoItem;
	} dt_pso_compiled;

	typedef struct dt_gpu_write_ctl {
		UINT32 streamIdx;
		UINT32 offset;
		UINT32 size;
		UINT32 bn;
	} dt_gpu_write_ctl;

	typedef struct dt_ia_prim_topo {
		UINT8 newTopo;
	} dt_ia_prim_topo;

	typedef struct dt_query_mark {
		d912pxy_query* obj;
		UINT8 start;
	} dt_query_mark;

#pragma pack(pop)

	template<class dataType>
	static constexpr UINT GetDataAlignedSize()
	{
		constexpr auto origSize = sizeof(dataType) + sizeof(typeName);
		constexpr bool isMisaligned = (origSize & (dataAligment - 1)) != 0;
		return isMisaligned ? (origSize & (~(dataAligment - 1))) + dataAligment : origSize;
			
	}

	const UINT GetSizeConst(const typeName name)
	{
		return dataTypeSize[(UINT)name];
	}

	const UINT GetSize(typeName name)
	{
		return dataTypeSize[(UINT)name];
	}

	const wchar_t* GetTypeNameStr()
	{
		static const wchar_t* dsc[] = {
			L"barrier",
			L"om_stencilref",
			L"om_blendfactor",
			L"view_scissor",
			L"draw_indexed",
			L"om_render_targets",
			L"vbuf_bind",
			L"ibuf_bind",
			L"clear_rt",
			L"clear_ds",
			L"pso_raw",
			L"pso_raw_feedback",
			L"pso_compiled",
			L"rect_copy",
			L"gpu_write_ctl",
			L"ia_prim_topo",
			L"query_mark"
		};

		return dsc[(UINT)iName];
	}

	template<class dataType>
	d912pxy_replay_item* SetAndAdvance(const typeName name, dataType data)
	{
		Set(name, data);
		return ((d912pxy_replay_item*)((intptr_t)this + GetSizeConst(name)));
	}


	template<class dataType>
	void Set(const typeName name, dataType data)
	{	
		static_assert(
			GetDataAlignedSize<dataType>() < maxDataSize
			, "replay item is too big"
		);


		iName = name;
		*(dataType*)storage = data;
		//new (storage) dataType(data);

	}

	template<class dataType>
	dataType* GetData()
	{
		return (dataType*)storage;
	}

	typeName GetTypeName() 
	{
		return iName;
	}

	d912pxy_replay_item* Next()
	{
		return ((d912pxy_replay_item*)((intptr_t)this + GetSize(iName)));
	}

	d912pxy_replay_item() { };
	~d912pxy_replay_item() { };
	   	
private:
	static const UINT dataTypeSize[(UINT)typeName::_count];

	typeName iName;
	BYTE storage[maxDataSize];
};