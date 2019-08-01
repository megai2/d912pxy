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


typedef enum d912pxy_config_value {
	PXY_CFG_POOLING_UPLOAD_ALLOC_STEP = 0,
	PXY_CFG_POOLING_UPLOAD_LIMITS = 1,
	PXY_CFG_POOLING_VSTREAM_ALLOC_STEP,
	PXY_CFG_POOLING_VSTREAM_LIMITS,
	PXY_CFG_POOLING_SURFACE_ALLOC_STEP,
	PXY_CFG_POOLING_SURFACE_LIMITS,
	PXY_CFG_POOLING_LIFETIME,
	PXY_CFG_POOLING_HOST_VA_RESERVE,
	PXY_CFG_POOLING_KEEP_RESIDENT,
	PXY_CFG_SAMPLERS_MIN_LOD,	
	PXY_CFG_CLEANUP_PERIOD,
	PXY_CFG_CLEANUP_SUBSLEEP,	
	PXY_CFG_SDB_KEEP_PAIRS,
	PXY_CFG_SDB_USE_PSO_PRECOMPILE,
	PXY_CFG_SDB_USE_PSO_KEY_CACHE,
	PXY_CFG_SDB_ALLOW_PP_SUFFIX,
	PXY_CFG_SDB_ENABLE_PROFILING,
	PXY_CFG_SDB_FORCE_UNUSED_REGS,
	PXY_CFG_SDB_ALLOW_REALTIME_CHECKS,
	PXY_CFG_SDB_NAN_GUARD_FLAG,
	PXY_CFG_SDB_SRGB_ALPHATEST_FLAG,
	PXY_CFG_REPLAY_BEHAIVOUR,
	PXY_CFG_REPLAY_THREADS,	
	PXY_CFG_REPLAY_ITEMS_PER_BATCH,
	PXY_CFG_MT_VSTREAM_CTOR,
	PXY_CFG_MT_SURFACE_CTOR,
	PXY_CFG_LOG_P7CONFIG,
	PXY_CFG_LOG_PERF_GRAPH,
	PXY_CFG_LOG_DBG_MEM_MGR_SAVE_NEW_CALLER,
	PXY_CFG_LOG_ENABLE_VEH,
	PXY_CFG_LOG_LOAD_RDOC,
	PXY_CFG_UPLOAD_TEX_ASYNC,	
	PXY_CFG_MISC_GPU_TIMEOUT,
	PXY_CFG_DX_DBG_RUNTIME,
	PXY_CFG_MISC_USE_DX9,
	PXY_CFG_MISC_DRAW_UP_BUFFER_LENGTH,
	PXY_CFG_COMPAT_OCCLUSION,
	PXY_CFG_COMPAT_CLEAR,
	PXY_CFG_COMPAT_CPU_API_REDUCTION,
	PXY_CFG_COMPAT_BATCH_COMMIT,
	PXY_CFG_COMPAT_OMRT_VIEWPORT_RESET,
	PXY_CFG_COMPAT_TRACK_RS,
	PXY_CFG_VFS_ROOT,
	PXY_CFG_VFS_MEMCACHE_MASK,
	PXY_CFG_CNT
} d912pxy_config_value;

typedef struct d912pxy_config_value_dsc {
	wchar_t section[256];
	wchar_t name[256];
	wchar_t value[256];
} d912pxy_config_value_dsc;

#define PXY_CFG_FILE_NAME "d912pxy/config.ini"

class d912pxy_config 
{
public:
	d912pxy_config();
	~d912pxy_config();

	void Init();

	UINT64 GetValueXI64(d912pxy_config_value val);
	UINT64 GetValueUI64(d912pxy_config_value val);
	UINT32 GetValueUI32(d912pxy_config_value val);
	wchar_t* GetValueRaw(d912pxy_config_value val);

	d912pxy_config_value_dsc* GetEntryRaw(d912pxy_config_value val);

private:

	d912pxy_config_value_dsc data[PXY_CFG_CNT] = {
		{L"pooling", L"upload_alloc_step", L"16"},//PXY_CFG_POOLING_UPLOAD_ALLOC_STEP
		{L"pooling", L"upload_limit", L"128"},//PXY_CFG_POOLING_UPLOAD_LIMITS		
		{L"pooling", L"vstream_alloc_step", L"16"},//PXY_CFG_POOLING_VSTREAM_ALLOC_STEP
		{L"pooling", L"vstream_limit", L"256"},//PXY_CFG_POOLING_VSTREAM_LIMITS
		{L"pooling", L"surface_alloc_step",L"0"},//PXY_CFG_POOLING_SURFACE_ALLOC_STEP
		{L"pooling", L"surface_limits",L"00000"},//PXY_CFG_POOLING_SURFACE_LIMITS
		{L"pooling", L"lifetime",L"10000"},//PXY_CFG_POOLING_LIFETIME
		{L"pooling", L"host_va_reserve",L"37"},//PXY_CFG_POOLING_HOST_VA_RESERVE
		{L"pooling", L"keep_resident",L"0"},//PXY_CFG_POOLING_KEEP_RESIDENT
		{L"samplers", L"min_lod", L"0"},//PXY_CFG_SAMPLERS_MIN_LOD		
		{L"cleanup", L"period",L"10000"},//PXY_CFG_CLEANUP_PERIOD
		{L"cleanup", L"subsleep",L"100"},//PXY_CFG_CLEANUP_SUBSLEEP	
		{L"sdb", L"keep_pairs", L"1"},//PXY_CFG_SDB_KEEP_PAIRS
		{L"sdb", L"use_pso_precompile", L"0"},//PXY_CFG_SDB_USE_PSO_PRECOMPILE
		{L"sdb", L"use_pso_key_cache", L"0"},//PXY_CFG_SDB_USE_PSO_KEY_CACHE
		{L"sdb", L"allow_pp_suffix", L"1"},//PXY_CFG_SDB_ALLOW_PP_SUFFIX
		{L"sdb", L"enable_profiling", L"0"},//PXY_CFG_SDB_ENABLE_PROFILING
		{L"sdb", L"force_unused_regs", L"0"},//PXY_CFG_SDB_FORCE_UNUSED_REGS
		{L"sdb", L"allow_realtime_checks", L"0"},//PXY_CFG_SDB_ALLOW_REALTIME_CHECKS
		{L"sdb", L"nan_guard_flag", L"41"},//PXY_CFG_SDB_NAN_GUARD_FLAG
		{L"sdb", L"srgb_alphatest_flag", L"0"},//PXY_CFG_SDB_SRGB_ALPHATEST_FLAG
		{L"replay", L"replay", L"1"},//PXY_CFG_REPLAY_BEHAIVOUR
		{L"replay", L"replay_threads", L"1"},//PXY_CFG_REPLAY_THREADS
		{L"replay", L"items_per_batch", L"100"},//PXY_CFG_REPLAY_ITEMS_PER_BATCH
		{L"mt", L"vstream_ctor", L"0"},//PXY_CFG_MT_VSTREAM_CTOR
		{L"mt", L"surface_ctor", L"0"},//PXY_CFG_MT_SURFACE_CTOR
		{L"log", L"p7config", L"/P7.Pool=32768 /P7.Sink=FileBin"},//PXY_CFG_LOG_P7CONFIG
		{L"log", L"perf_graph", L"0"},//PXY_CFG_LOG_PERF_GRAPH		
		{L"log", L"dbg_mem_mgr_save_new_caller", L"0"},//PXY_CFG_LOG_DBG_MEM_MGR_SAVE_NEW_CALLER
		{L"log", L"enable_veh", L"1"}, //PXY_CFG_LOG_ENABLE_VEH
		{L"log", L"load_rdoc", L"0"},//PXY_CFG_LOG_LOAD_RDOC
		{L"upload",L"tex_async",L"0"},//PXY_CFG_UPLOAD_TEX_ASYNC
		{L"misc",L"gpu_timeout",L"5000"},//PXY_CFG_MISC_GPU_TIMEOUT
		{L"misc",L"dx_debug",L"0"},//PXY_CFG_DX_DBG_RUNTIME
		{L"misc",L"use_dx9",L"0"},//PXY_CFG_MISC_USE_DX9
		{L"misc",L"draw_up_buffer_length", L"FFFF"},//PXY_CFG_MISC_DRAW_UP_BUFFER_LENGTH
		{L"compat",L"occlusion",L"0"},//PXY_CFG_COMPAT_OCCLUSION
		{L"compat",L"clear",L"0"},//PXY_CFG_COMPAT_CLEAR
		{L"compat",L"cpu_api_reduction",L"0"},//PXY_CFG_COMPAT_CPU_API_REDUCTION
		{L"compat",L"batch_commit",L"0"},//PXY_CFG_COMPAT_BATCH_COMMIT
		{L"compat",L"omrt_viewport_reset",L"0"},//PXY_CFG_COMPAT_OMRT_VIEWPORT_RESET
		{L"compat",L"track_rs",L"0"},//PXY_CFG_COMPAT_TRACK_RS
		{L"vfs", L"root", L"./d912pxy/pck"},//PXY_CFG_VFS_ROOT
		{L"vfs", L"memcache_mask", L"6F"}//PXY_CFG_VFS_MEMCACHE_MASK
	};

};

