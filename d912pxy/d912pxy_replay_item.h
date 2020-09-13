/*
MIT License

Copyright(c) 2019-2020 megai2

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
	typedef enum class typeName : UINT64 {
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
		custom_batch_data,
		_count
	} typeNames;
	
	static const UINT maxDataSize = 256 - sizeof(typeName);
	static const UINT dataAligment = 16;

#pragma pack(push,4)

	typedef struct dt_barrier {
		d912pxy_resource* res;
		D3D12_RESOURCE_STATES to;
		D3D12_RESOURCE_STATES from;

		static constexpr typeName GetTypeName() { return typeName::barrier; };
	} dt_barrier;

	typedef struct dt_om_stencilref {
		DWORD dRef;

		static constexpr typeName GetTypeName() { return typeName::om_stencilref; };
	} dt_om_stencilref;

	typedef struct dt_om_blendfactor {
		float color[4];

		static constexpr typeName GetTypeName() { return typeName::om_blendfactor; };
	} dt_om_blendfactor;

	typedef struct dt_view_scissor {
		D3D12_VIEWPORT viewport;
		D3D12_RECT scissor;

		static constexpr typeName GetTypeName() { return typeName::view_scissor; };
	} dt_view_scissor;

	typedef struct dt_draw_indexed {
		UINT IndexCountPerInstance;
		UINT InstanceCount;
		UINT StartIndexLocation;
		INT BaseVertexLocation;
		UINT StartInstanceLocation;
		UINT batchId;

		static constexpr typeName GetTypeName() { return typeName::draw_indexed; };
	} dt_draw_indexed;

	typedef struct dt_om_render_targets {
		d912pxy_surface* rtv[PXY_INNER_MAX_RENDER_TARGETS];
		d912pxy_surface* dsv;

		static constexpr typeName GetTypeName() { return typeName::om_render_targets; };
	} dt_om_render_targets;

	typedef struct dt_vbuf_bind {
		d912pxy_vstream* buf;
		UINT stride;
		UINT slot;
		UINT offset;

		static constexpr typeName GetTypeName() { return typeName::vbuf_bind; };
	} dt_vbuf_bind;

	typedef struct dt_ibuf_bind {
		d912pxy_vstream* buf;

		static constexpr typeName GetTypeName() { return typeName::ibuf_bind; };
	} dt_ibuf_bind;

	typedef struct dt_clear_rt {
		d912pxy_surface* tgt;
		float clr[4];
		D3D12_RECT clearRect;
		D3D12_RESOURCE_STATES cuState;

		static constexpr typeName GetTypeName() { return typeName::clear_rt; };
	} dt_clear_rt;

	typedef struct dt_clear_ds {
		d912pxy_surface* tgt;
		float depth;
		UINT8 stencil;
		D3D12_CLEAR_FLAGS flag;
		D3D12_RECT clearRect;
		D3D12_RESOURCE_STATES cuState;

		static constexpr typeName GetTypeName() { return typeName::clear_ds; };
	} dt_clear_ds;

	typedef struct dt_pso_raw {
		d912pxy_trimmed_pso_desc rawState;

		static constexpr typeName GetTypeName() { return typeName::pso_raw; };
	} dt_pso_raw;

	typedef struct dt_pso_raw_feedback {
		d912pxy_trimmed_pso_desc rawState;
		void** feedbackPtr;

		static constexpr typeName GetTypeName() { return typeName::pso_raw_feedback; };
	} dt_pso_raw_feedback;

	typedef struct dt_rect_copy {
		d912pxy_surface* src;
		d912pxy_surface* dst;
		D3D12_RESOURCE_STATES prevS;
		D3D12_RESOURCE_STATES prevD;

		static constexpr typeName GetTypeName() { return typeName::rect_copy; };
	} dt_rect_copy;

	typedef struct dt_pso_compiled {
		d912pxy_pso_item* psoItem;

		static constexpr typeName GetTypeName() { return typeName::pso_compiled; };
	} dt_pso_compiled;

	typedef struct dt_gpu_write_ctl {
		UINT32 streamIdx;
		UINT32 offset;
		UINT32 size;
		UINT32 bn;

		static constexpr typeName GetTypeName() { return typeName::gpu_write_ctl; };
	} dt_gpu_write_ctl;

	typedef struct dt_ia_prim_topo {
		UINT8 newTopo;

		static constexpr typeName GetTypeName() { return typeName::ia_prim_topo; };
	} dt_ia_prim_topo;

	typedef struct dt_query_mark {
		d912pxy_query* obj;
		UINT8 start;

		static constexpr typeName GetTypeName() { return typeName::query_mark; };
	} dt_query_mark;

	struct dt_custom_batch_data {
		D3D12_GPU_VIRTUAL_ADDRESS batchDataPtr;
		static constexpr typeName GetTypeName() { return typeName::custom_batch_data; };
	};

#pragma pack(pop)

	template<class dataType>
	static constexpr UINT GetDataAlignedSize()
	{
		constexpr auto origSize = sizeof(dataType);
		constexpr bool isMisaligned = (origSize & (dataAligment - 1)) != 0;
		return (isMisaligned ? (origSize & (~(dataAligment - 1))) + dataAligment : origSize) + sizeof(typeName);
			
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
	d912pxy_replay_item* Advance(dataType** data)
	{
		*data = SetType<dataType>();
		return ((d912pxy_replay_item*)((intptr_t)this + GetSizeConst(dataType::GetTypeName())));
	}

	template<class dataType>
	void Set(dataType data)
	{
		dataType* dst = SetType<dataType>();
		*dst = data;
	}


	template<class dataType>
	dataType* SetType()
	{	
		static_assert(
			GetDataAlignedSize<dataType>() < maxDataSize
			, "replay item is too big"
		);
		
		iName = dataType::GetTypeName();
		return GetData<dataType>();
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

	d912pxy_replay_item() : iName(typeName::_count){ };
	~d912pxy_replay_item() { };
	   	
private:
	static const UINT dataTypeSize[(UINT)typeName::_count];

	typeName iName;
	BYTE storage[maxDataSize];
};