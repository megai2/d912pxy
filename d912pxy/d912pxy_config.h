/*
MIT License

Copyright(c) 2018-2020 megai2

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
	PXY_CFG_CLEANUP_SOFT_LIMIT,
	PXY_CFG_CLEANUP_HARD_LIMIT,
	PXY_CFG_CLEANUP_AFTER_RESET_MAID,
	PXY_CFG_SDB_KEEP_PAIRS,
	PXY_CFG_SDB_LOAD_PSO_CACHE,
	PXY_CFG_SDB_SAVE_PSO_CACHE,
	PXY_CFG_SDB_ALLOW_PP_SUFFIX,
	PXY_CFG_SDB_FORCE_UNUSED_REGS,
	PXY_CFG_SDB_NAN_GUARD_FLAG,
	PXY_CFG_REPLAY_BEHAIVOUR,
	PXY_CFG_REPLAY_THREADS,	
	PXY_CFG_REPLAY_ITEMS_PER_BATCH,
	PXY_CFG_MT_VSTREAM_CTOR,
	PXY_CFG_MT_SURFACE_CTOR,
	PXY_CFG_MT_DXC_THREADS,
	PXY_CFG_MT_PSO_THREADS,
	PXY_CFG_LOG_P7CONFIG,
	PXY_CFG_LOG_PERF_GRAPH,
	PXY_CFG_LOG_DBG_MEM_MGR_SAVE_NEW_CALLER,
	PXY_CFG_LOG_ENABLE_VEH,
	PXY_CFG_LOG_LOAD_RDOC,
	PXY_CFG_UPLOAD_TEX_ASYNC,	
	PXY_CFG_BATCHING_FORCE_NEW_BATCH,
	PXY_CFG_BATCHING_RAW_GPUW,
	PXY_CFG_BATCHING_MAX_WRITES_PER_BATCH,
	PXY_CFG_BATCHING_MAX_BATCHES_PER_IFRAME,
	PXY_CFG_DX_DBG_RUNTIME,
	PXY_CFG_DX_FRAME_LATENCY,
	PXY_CFG_DX_ROUTE_TO_DX9,
	PXY_CFG_MISC_GPU_TIMEOUT,		
	PXY_CFG_MISC_NV_DISABLE_THROTTLE,	
	PXY_CFG_COMPAT_OCCLUSION,
	PXY_CFG_COMPAT_OCCLUSION_OPT_CONSTRUCTOR,
	PXY_CFG_COMPAT_CLEAR,
	PXY_CFG_COMPAT_CPU_API_REDUCTION,
	PXY_CFG_COMPAT_BATCH_COMMIT,
	PXY_CFG_COMPAT_OMRT_VIEWPORT_RESET,
	PXY_CFG_COMPAT_TRACK_RS,
	PXY_CFG_COMPAT_DUP_UNSAFE,
	PXY_CFG_COMPAT_DHEAP_MODE,
	PXY_CFG_COMPAT_EXPLICIT_D3DCOMPILER,
	PXY_CFG_VFS_ROOT,
	PXY_CFG_VFS_MEMCACHE_MASK,
	PXY_CFG_VFS_PACK_DATA,
	PXY_CFG_VFS_WRITE_MASK,
	PXY_CFG_EXTRAS_ENABLE,
	PXY_CFG_EXTRAS_FPS_LIMIT,
	PXY_CFG_EXTRAS_FPS_LIMIT_INACTIVE,
	PXY_CFG_EXTRAS_SHOW_FPS,
	PXY_CFG_EXTRAS_SHOW_DRAW_COUNT,
	PXY_CFG_EXTRAS_SHOW_FPS_GRAPH,
	PXY_CFG_EXTRAS_SHOW_TIMINGS,
	PXY_CFG_EXTRAS_SHOW_PSO_COMPILE_QUE,
	PXY_CFG_EXTRAS_SHOW_GC_QUE,
	PXY_CFG_EXTRAS_OVERLAY_TOGGLE_KEY,
	PXY_CFG_EXTRAS_FPS_GRAPH_MAX,
	PXY_CFG_EXTRAS_FPS_GRAPH_MIN,
	PXY_CFG_EXTRAS_FPS_GRAPH_W,
	PXY_CFG_EXTRAS_FPS_GRAPH_H,
	PXY_CFG_EXTRAS_ENABLE_CONFIG_EDITOR,
	PXY_CFG_EXTRAS_FIRST_INSTALL_MESSAGE,
	PXY_CFG_EXTRAS_IFRAME_MOD_SOURCE,
	PXY_CFG_EXTRAS_TRACK_SHADER_PAIRS,
	PXY_CFG_EXTRAS_SPAIR_SOURCE,
	PXY_CFG_CNT
} d912pxy_config_value;

typedef struct d912pxy_config_value_dsc {
	wchar_t section[255];
	wchar_t name[255];
	wchar_t value[255];
	const wchar_t* limitation;
	const wchar_t* shortDescription;
	const wchar_t* longDescription;
	char* newValue;
} d912pxy_config_value_dsc;

class d912pxy_config 
{
public:
	d912pxy_config();
	~d912pxy_config();

	void Init();

	UINT64 GetValueXI64(d912pxy_config_value val);
	UINT64 GetValueUI64(d912pxy_config_value val);
	UINT32 GetValueUI32(d912pxy_config_value val);
	bool GetValueB(d912pxy_config_value val);
	wchar_t* GetValueRaw(d912pxy_config_value val);
	void InitNewValueBuffers();
	void UnInitNewValueBuffers();
	void ValueToNewValueBuffers();
	void SaveConfig();
	d912pxy_config_value_dsc* GetEntryRaw(d912pxy_config_value val);

private:

	d912pxy_config_value_dsc data[PXY_CFG_CNT] = {
		{
			L"pooling", 
			L"upload_alloc_step", 
			L"16",
			L"u r:0,1024",
			L"upload segment allocation step",
			L"controls how much space will be allocated for segment that is used to take upload memory from",
			nullptr
		},//PXY_CFG_POOLING_UPLOAD_ALLOC_STEP
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
		{L"cleanup", L"subsleep",L"50"},//PXY_CFG_CLEANUP_SUBSLEEP	
		{L"cleanup", L"soft_limit",L"3000"},//PXY_CFG_CLEANUP_SOFT_LIMIT
		{L"cleanup", L"hard_limit",L"4000"},//PXY_CFG_CLEANUP_HARD_LIMIT
		{L"cleanup", L"after_reset_maid",L"2"},//PXY_CFG_CLEANUP_AFTER_RESET_MAID
		{L"sdb", L"keep_pairs", L"1"},//PXY_CFG_SDB_KEEP_PAIRS
		{L"sdb", L"load_pso_cache", L"0"},//PXY_CFG_SDB_LOAD_PSO_CACHE
		{L"sdb", L"save_pso_cache", L"0"},//PXY_CFG_SDB_SAVE_PSO_CACHE
		{L"sdb", L"allow_pp_suffix", L"1"},//PXY_CFG_SDB_ALLOW_PP_SUFFIX
		{L"sdb", L"force_unused_regs", L"0"},//PXY_CFG_SDB_FORCE_UNUSED_REGS
		{L"sdb", L"nan_guard_flag", L"81"},//PXY_CFG_SDB_NAN_GUARD_FLAG
		{L"replay", L"replay", L"1"},//PXY_CFG_REPLAY_BEHAIVOUR
		{L"replay", L"replay_threads", L"1"},//PXY_CFG_REPLAY_THREADS
		{L"replay", L"items_per_batch", L"100"},//PXY_CFG_REPLAY_ITEMS_PER_BATCH
		{L"mt", L"vstream_ctor", L"1"},//PXY_CFG_MT_VSTREAM_CTOR
		{L"mt", L"surface_ctor", L"1"},//PXY_CFG_MT_SURFACE_CTOR
	    {L"mt", L"dxc_threads", L"-1"},//PXY_CFG_MT_DXC_THREADS
		{L"mt", L"pso_threads", L"-1"},//PXY_CFG_MT_PSO_THREADS
		{L"log", L"p7config", L"/P7.Pool=32768 /P7.Sink=FileBin"},//PXY_CFG_LOG_P7CONFIG
		{L"log", L"perf_graph", L"0"},//PXY_CFG_LOG_PERF_GRAPH		
		{L"log", L"dbg_mem_mgr_save_new_caller", L"0"},//PXY_CFG_LOG_DBG_MEM_MGR_SAVE_NEW_CALLER
		{L"log", L"enable_veh", L"1"}, //PXY_CFG_LOG_ENABLE_VEH
		{L"log", L"load_rdoc", L"0"},//PXY_CFG_LOG_LOAD_RDOC
		{L"upload",L"tex_async",L"0"},//PXY_CFG_UPLOAD_TEX_ASYNC
		{L"batching",L"force_new",L"0"},//PXY_CFG_BATCHING_FORCE_NEW_BATCH
		{L"batching",L"raw_gpuw",L"0"},//PXY_CFG_BATCHING_RAW_GPUW		
		{
			L"batching",
			L"maxWritesPerBatch",
			L"128",
			L"u r:10,+inf",
			L"max GPU writes in one batch",
			L"space that is allocated for GPU writes in batch buffer, affect RAM usage, low values can create crashes",
			nullptr
		},//PXY_CFG_BATCHING_MAX_WRITES_PER_BATCH,
		{
			L"batching",
			L"maxBatchesPerIFrame",
			L"8192",
			L"u r:5,+inf",
			L"max batches in internal frame",
			L"limit of batches in internal frame, affects RAM&VRAM usage, low values reduce performance, high values improve performance in heavy loaded scenes",
			nullptr
		},//PXY_CFG_BATCHING_MAX_BATCHES_PER_IFRAME,
		{L"dx",L"debug",L"0"},//PXY_CFG_DX_DBG_RUNTIME
		{
			L"dx",
			L"dxgi_frame_latency",
			L"0",
			L"u r:0,3",
			L"enables and configures dxgi frame latency feature",
			L"0 disables swapchain waits, any other value enables swapchain waits and trying to set this value as max frame latency"
		},//PXY_CFG_DX_FRAME_LATENCY
		{L"dx",L"route_to_dx9",L"0"},//PXY_CFG_DX_ROUTE_TO_DX9		
		{L"misc",L"gpu_timeout",L"5000"},//PXY_CFG_MISC_GPU_TIMEOUT
		{L"misc",L"nv_disable_throttle", L"0"},//PXY_CFG_MISC_NV_DISABLE_THROTTLE		
		{L"compat",L"occlusion",L"2"},//PXY_CFG_COMPAT_OCCLUSION
		{L"compat",L"occlusion_opt_ctor",L"0"},//PXY_CFG_COMPAT_OCCLUSION_OPT_CONSTRUCTOR
		{L"compat",L"clear",L"0"},//PXY_CFG_COMPAT_CLEAR
		{L"compat",L"cpu_api_reduction",L"0"},//PXY_CFG_COMPAT_CPU_API_REDUCTION
		{L"compat",L"batch_commit",L"0"},//PXY_CFG_COMPAT_BATCH_COMMIT
		{L"compat",L"omrt_viewport_reset",L"0"},//PXY_CFG_COMPAT_OMRT_VIEWPORT_RESET
		{L"compat",L"track_rs",L"0"},//PXY_CFG_COMPAT_TRACK_RS
		{L"compat",L"unsafe_dup",L"0"},//PXY_CFG_COMPAT_DUP_UNSAFE
		{L"compat",L"dheap_mode",L"0"},//PXY_CFG_COMPAT_DHEAP_MODE
		{
			L"compat",
			L"explicit_d3dcompiler_dll",
			L"0",
			L"b r:0,1",
			L"Allows to use d912pxy supplied d3d compiler dll",
			L"If 1 uses d3d compiler 27 v10 lib from 12on7 instead of default one"
		},//PXY_CFG_COMPAT_EXPLICIT_D3DCOMPILER
		{L"vfs", L"root", L"./d912pxy/pck"},//PXY_CFG_VFS_ROOT
		{L"vfs", L"memcache_mask", L"63"},//PXY_CFG_VFS_MEMCACHE_MASK
		{L"vfs", L"pack_data", L"0"},//PXY_CFG_VFS_PACK_DATA
		{L"vfs", L"write_mask", L"0"},//PXY_CFG_VFS_WRITE_MASK
		{L"extras", L"enable", L"1"},//PXY_CFG_EXTRAS_ENABLE
		{L"extras", L"fps_limit", L"0"},//PXY_CFG_EXTRAS_FPS_LIMIT,
		{L"extras", L"fps_limit_inactive", L"0"},//PXY_CFG_EXTRAS_FPS_LIMIT_INACTIVE,
		{L"extras", L"show_fps", L"1"},//PXY_CFG_EXTRAS_SHOW_FPS,
		{L"extras", L"show_draw_count", L"1"},//PXY_CFG_EXTRAS_SHOW_DRAW_COUNT,
		{L"extras", L"show_fps_graph", L"1"},//PXY_CFG_EXTRAS_SHOW_FPS_GRAPH,
		{L"extras", L"show_timings", L"1"},//PXY_CFG_EXTRAS_SHOW_TIMINGS,
		{L"extras", L"show_pso_compile_que", L"1"},//PXY_CFG_EXTRAS_SHOW_PSO_COMPILE_QUE,
		{L"extras", L"show_gc_que", L"1"},//PXY_CFG_EXTRAS_SHOW_GC_QUE,
		{L"extras", L"overlay_toggle_key", L"78"},//PXY_CFG_EXTRAS_OVERLAY_TOGGLE_KEY
		{L"extras", L"fps_graph_max", L"80"},//PXY_CFG_EXTRAS_FPS_GRAPH_MAX
		{L"extras", L"fps_graph_min", L"0"},//PXY_CFG_EXTRAS_FPS_GRAPH_MIN
		{L"extras", L"fps_graph_w", L"512"},//PXY_CFG_EXTRAS_FPS_GRAPH_W
		{L"extras", L"fps_graph_h", L"256"},//PXY_CFG_EXTRAS_FPS_GRAPH_H
		{L"extras", L"enable_config_editor",L"1"},//PXY_CFG_EXTRAS_ENABLE_CONFIG_EDITOR
		{L"extras", L"show_first_install_message",L"1"},//PXY_CFG_EXTRAS_FIRST_INSTALL_MESSAGE
		{
			L"extras", 
			L"iframe_mod_source", 
			L"none",
			L"s r:valid path to file or \"none\" string"
			L"source of iframe modifications",
			L"defines source of iframe modifications scripts"
			L"iframe modifications transform api stream to perform specific effects"
			L"by default no modifications are used. modifications are strongly game dependant!"
			L"builtin modifications live in d912pxy/shaders/mods"
		},//PXY_CFG_EXTRAS_IFRAME_MOD_SOURCE
		{
			L"extras", 
			L"shader_pair_tracker",
			L"0",
			L"b r:[0-1]",
			L"enable/disable shader pair tracker in overlay",
			L"if enabled will record and show every draw shader pair with supplied marks if found in d912pxy/shaders/pairs/<cfg>"
			L"iframe mods should be enabled for this feature to work"
		},//PXY_CFG_EXTRAS_TRACK_SHADER_PAIRS
		{
			L"extras",
			L"spair_source",
			L"none",
			L"s r:valid directory name or \"none\" string",
			L"defines source for shader pair information",
			L"will read data from d912pxy/shaders/pairs/<value> for shader pair info"
		}//PXY_CFG_EXTRAS_SPAIR_SOURCE
	};

};

