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

//D3D9 API extenders =======================

#define D3DRS_ENABLE_D912PXY_API_HACKS (D3DRENDERSTATETYPE)220
#define D3DRS_D912PXY_ENQUEUE_PSO_COMPILE (D3DRENDERSTATETYPE)221
#define D3DRS_D912PXY_SETUP_PSO (D3DRENDERSTATETYPE)222
#define D3DRS_D912PXY_GPU_WRITE (D3DRENDERSTATETYPE)223
#define D3DRS_D912PXY_DRAW (D3DRENDERSTATETYPE)224
#define D3DRS_D912PXY_SAMPLER_ID (D3DRENDERSTATETYPE)225
#define D3DRS_D912PXY_CUSTOM_BATCH_DATA (D3DRENDERSTATETYPE)226

#define D3DDECLMETHOD_PER_VERTEX_CONSTANT 8
#define D3DDECLMETHOD_PER_INSTANCE_CONSTANT 16
#define D3DUSAGE_D912PXY_FORCE_RT 0x0F000000L

#define D912PXY_ENCODE_GPU_WRITE_DSC(sz, offset) (((sz) & 0xFFFF) | (((offset) & 0xFFFF) << 16))

#define D912PXY_GPU_WRITE_OFFSET_TEXBIND 0
#define D912PXY_GPU_WRITE_OFFSET_SAMPLER 8 
#define D912PXY_GPU_WRITE_OFFSET_VS_VARS 16
#define D912PXY_GPU_WRITE_OFFSET_PS_VARS (16 + 256)

struct d912pxy_custom_batch_data
{
	IDirect3DVertexBuffer9* buffer;
	int index;
};

//configuration switches =======================

#define UPLOAD_POOL_USE_AND_DISCARD 
//#define ENABLE_METRICS
//#define PER_DRAW_FLUSH 
//#define USE_PIX_EVENT_ANNOTATIONS

//#define THREAD_FAST_WAKE
#define THREAD_MODEST_WAKE

#ifdef _DEBUG	
	#define ENABLE_DEBUG_LOGGING
#endif

#ifdef ENABLE_DEBUG_LOGGING
	#define ENABLE_METRICS
#endif

//inner max/structure defenitions =======================

#define d912pxy_shader_uid UINT64
#define d912pxy_shader_pair_hash_type UINT64

#define PXY_INNER_GPU_QUEUE_BUFFER_COUNT 2
#define PXY_INNER_MAX_SWAP_CHAINS 4
#define PXY_INNER_MAX_RENDER_TARGETS 8
#define PXY_INNER_MAX_TEXTURE_STAGES 32
#define PXY_INNER_MAX_SHADER_SAMPLERS 32
#define PXY_INNER_MAX_SHADER_SAMPLERS_SZ 1664
#define PXY_INNER_MAX_SHADER_CONSTS_IDX 256
#define PXY_INNER_MAX_SHADER_CONSTS (256*4)
#define PXY_INNER_EXTRA_SHADER_CONST_CLIP_P0 256
#define PXY_INNER_EXTRA_SHADER_CONST_HALFPIXEL_FIX 257
#define PXY_INNER_EXTRA_SHADER_CONST_BASE_IDX 260
#define PXY_INNER_EXTRA_SHADER_CONST_CLEAR_COLOR 260
#define PXY_INNER_EXTRA_SHADER_CONST_CLEAR_RECT 261
#define PXY_INNER_EXTRA_SHADER_CONST_CLEAR_ZWH 262
#define PXY_INNER_EXTRA_SHADER_CONST_TAA_PARAMS 263
#define PXY_INNER_EXTRA_SHADER_CONST_UNUSED 264
#define PXY_INNER_MAX_VBUF_STREAMS 10
#define PXY_INNER_MAX_IFRAME_CLEANUPS (1024*1024)
#define PXY_INNER_MAX_CLEANUPS_PER_SYNC 128
#define PXY_INNER_MAX_IFRAME_PSO_CACHES 4096
#define PXY_INNER_MAX_CACHE_NODES_SAMPLERS 4096
#define PXY_INNER_MAX_TEXSTATE_CACHE_NODES 0xFFFFF
#define PXY_INNER_MAX_PSO_CACHE_NODES 0xFFFF
#define PXY_INNER_MAX_DHEAP_CLEANUP_PER_SYNC 16
#define PXY_INNER_MAX_ASYNC_TEXLOADS 5120
#define PXY_INNER_MAX_ASYNC_BUFFERLOADS 5120
#define PXY_INNER_MAX_OCCLUSION_QUERY_COUNT_PER_FRAME 512

#define PXY_INNER_CLG_PRIO_FIRST 0 
#define PXY_INNER_CLG_PRIO_ASYNC_LOAD 1
#define PXY_INNER_CLG_PRIO_REPLAY 4
#define PXY_INNER_CLG_PRIO_LAST 100

#define PXY_INNER_MAX_DSC_HEAPS 4
#define PXY_INNER_HEAP_RTV 0
#define PXY_INNER_HEAP_DSV 1
#define PXY_INNER_HEAP_SRV 2
#define PXY_INNER_HEAP_CBV 2
#define PXY_INNER_HEAP_SPL 3

#define PXY_INNER_THREADID_TEX_LOADER 0
#define PXY_INNER_THREADID_BUF_LOADER 1
#define PXY_INNER_THREADID_RPL_THRD0 2
#define PXY_INNER_THREADID_RPL_THRD1 3
#define PXY_INNER_THREADID_RPL_THRD2 4
#define PXY_INNER_THREADID_RPL_THRD3 5
#define PXY_INNER_THREADID_MAX 6

#define PXY_INNER_REPLAY_THREADS_MAX 4

#ifdef THREAD_MODEST_WAKE
	#define PXY_WAKE_FACTOR_TEXTURE 1
	#define PXY_WAKE_FACTOR_BUFFER 10
	#define PXY_WAKE_FACTOR_REPLAY 100
#endif

#ifdef THREAD_FAST_WAKE
	#define PXY_WAKE_FACTOR_TEXTURE 1
	#define PXY_WAKE_FACTOR_BUFFER 1
	#define PXY_WAKE_FACTOR_REPLAY 10
#endif

//memory manager ============================

#define PXY_MEM_MGR_TRIES 100
#define PXY_MEM_MGR_RETRY_WAIT 100

#define USE_VA_FOR_HOST_MEMORY 

#ifdef USE_VA_FOR_HOST_MEMORY

#define PXY_MALLOC_GPU_HOST_COPY(pointer, size, tcast) pointer = d912pxy_s.pool.hostPow2.AllocateObjPow2(size);
#define PXY_FREE_GPU_HOST_COPY(pointer) d912pxy_s.pool.hostPow2.DeAllocateObj(pointer);

#else 

#define PXY_MALLOC_GPU_HOST_COPY(pointer, size, tcast) PXY_MALLOC(pointer, size, tcast)
#define PXY_FREE_GPU_HOST_COPY(pointer) PXY_FREE(pointer)

#endif

#ifdef _DEBUG

#define PXY_MALLOC(pointer, size, tcast) (d912pxy_s.mem.pxy_malloc_dbg((void**)&pointer, size, __FILE__, __LINE__, __FUNCTION__))
#define PXY_REALLOC(pointer, size, tcast) (d912pxy_s.mem.pxy_realloc_dbg((void**)&pointer, size, __FILE__, __LINE__, __FUNCTION__))
#define PXY_FREE(pointer) (d912pxy_s.mem.pxy_free_dbg((void**)&pointer, __FILE__, __LINE__, __FUNCTION__))

#else

#define PXY_MALLOC(pointer, size, tcast) pointer = (tcast)(d912pxy_s.mem.pxy_malloc(size))
#define PXY_REALLOC(pointer, size, tcast) pointer = (tcast)(d912pxy_s.mem.pxy_realloc((void*)pointer, size))
#define PXY_FREE(pointer) (d912pxy_s.mem.pxy_free((void*)pointer))

#endif

//metrics macros =======================

#ifdef ENABLE_METRICS
	#define FRAME_METRIC_CLEANUPS(a) d912pxy_s.log.metrics.TrackCleanupCount(a);
	#define FRAME_METRIC_DHEAP(a,b) d912pxy_s.log.metrics.TrackDHeapSlots(a,b);
	#define FRAME_METRIC_EXEC(a) d912pxy_s.log.metrics.TrackIFrameTime(a, PXY_METRICS_IFRAME_EXEC);
	#define FRAME_METRIC_SYNC(a) d912pxy_s.log.metrics.TrackIFrameTime(a, PXY_METRICS_IFRAME_SYNC);
	#define FRAME_METRIC_SYNC_WAKE(a) d912pxy_s.log.metrics.TrackIFrameTime(a, PXY_METRICS_IFRAME_SYNC_WAKE);
	#define FRAME_METRIC_THREAD(a,b) d912pxy_s.log.metrics.TrackIFrameTime(a, PXY_METRICS_IFRAME_THREAD_TEX+b);
	#define FRAME_METRIC_PRESENT(a) d912pxy_s.log.metrics.TrackIFrameTime(a, PXY_METRICS_IFRAME_PREP);
	#define FRAME_METRIC_RESIDENCY(a) d912pxy_s.log.metrics.TrackIFrameTime(a, PXY_METRICS_IFRAME_RESIDENCY);	
#else 
	#define FRAME_METRIC_CLEANUPS(a)
	#define FRAME_METRIC_DHEAP(a,b)
	#define FRAME_METRIC_EXEC(a) 
	#define FRAME_METRIC_SYNC(a)
	#define FRAME_METRIC_SYNC_WAKE(a)
	#define FRAME_METRIC_THREAD(a,b)
	#define FRAME_METRIC_PRESENT(a) 
	#define FRAME_METRIC_RESIDENCY(a)
#endif

//logging macro =======================

#ifdef DISABLE_P7LIB
	#define TM(i_pStr)         L##i_pStr
#endif

#define LOG_INFO_DTDM(fmt, ...) (d912pxy_s.log.text._PXY_LOG_INFO(LGC_DEFAULT, TM(fmt), __VA_ARGS__))
#define LOG_WARN_DTDM(fmt, ...) (d912pxy_s.log.text._PXY_LOG_WARNING(LGC_DEFAULT, TM(fmt), __VA_ARGS__))
#define LOG_ERR_DTDM(fmt, ...) (d912pxy_s.log.text._PXY_LOG_ERROR(LGC_DEFAULT, TM(fmt), __VA_ARGS__))
#define LOG_INFO_DTDM2(code, fmt, ...) code; LOG_INFO_DTDM(fmt, __VA_ARGS__)

#ifdef ENABLE_DEBUG_LOGGING
	#ifdef _DEBUG
		#define LOG_DBG_DTDM(fmt, ...) ;//(d912pxy_s.log.text._PXY_LOG_DEBUG(LGC_DEFAULT, TM(fmt), __VA_ARGS__))
		#define LOG_DBG_DTDM2(fmt, ...) ;//(d912pxy_s.log.text._PXY_LOG_DEBUG(LGC_DEFAULT, TM(fmt), __VA_ARGS__))
		#define LOG_DBG_DTDM3(fmt, ...) (d912pxy_s.log.text._PXY_LOG_DEBUG(LGC_DEFAULT, TM(fmt), __VA_ARGS__))
		#define LOG_DX_SET_NAME(obj, val) obj->SetName(val)
        #define LOG_ASSERT(cnd, text) if (!(cnd)) LOG_ERR_THROW2(-1, text)
	#else 
		#define LOG_DBG_DTDM(fmt, ...) (d912pxy_s.log.text._PXY_LOG_DEBUG(LGC_DEFAULT, TM(fmt), __VA_ARGS__))
		#define LOG_DBG_DTDM2(fmt, ...) (d912pxy_s.log.text._PXY_LOG_DEBUG(LGC_DEFAULT, TM(fmt), __VA_ARGS__))
		#define LOG_DBG_DTDM3(fmt, ...) (d912pxy_s.log.text._PXY_LOG_DEBUG(LGC_DEFAULT, TM(fmt), __VA_ARGS__))
		#define LOG_DX_SET_NAME(obj, val) obj->SetName(val)
		#define LOG_ASSERT(cnd, text) if (!(cnd)) LOG_ERR_THROW2(-1, text)
        #define PER_DRAW_FLUSH 
	#endif
#else
	#define LOG_DBG_DTDM(fmt, ...) ;
	#define LOG_DBG_DTDM2(fmt, ...) ;
	#define LOG_DBG_DTDM3(fmt, ...) ;
	#define LOG_DX_SET_NAME(obj, val) 
	#define LOG_ASSERT(cnd, text) (cnd)
#endif

#define LOG_ERR_THROW(hr) LOG_ERR_THROW2(hr, hr)
#define LOG_ERR_THROW2(hr, hr2) ThrowErrorDbg(hr, #hr2 )

//pix event wrappers	   
#ifdef USE_PIX_EVENT_ANNOTATIONS
	#include <pix3.h>
#else 
	#define PIXBeginEvent(a, ...) //
	#define PIXEndEvent(...) //
#endif

//paths to files =======================

typedef enum d912pxy_file_path_id {
	FP_CS_HLSL_DIR,
	FP_CS_CSO_DIR,
	FP_SHADER_DB_HLSL_DIR,
	FP_VFS_LOCK_FILE,
	FP_VFS_PACK_FILE,
	FP_SHADER_DB_HLSL_CUSTOM_DIR,
	FP_SHADER_DB_CSO_DIR,
	FP_SHADER_DB_BUGS_DIR,
	FP_CRASH_LOG,
	FP_LOG,
	FP_LOG_OLD,
	FP_PERF_GRAPH_OUTFILE,
	FP_PERF_GRAPH_DX9_OUTFILE,
	FP_PERF_GRAPH_OUTFILE_PNG,
	FP_PERF_GRAPH_DX9_OUTFILE_PNG,
	FP_CONFIG,
	FP_W7_12ON7,
	FP_VFS_PREFIX,
	FP_IMGUI_INI,
	FP_IMGUI_LOG,
	FP_SPAIR_INFO_BASE_PATH,
	FP_IFRAME_MODS_BASE_PATH,
	FP_PROXY_DATA_PATH,
	FP_NO_PATH
} d912pxy_file_path_id;

typedef union d912pxy_file_path {
	struct {
		const char* s;
		const wchar_t* w;
	};
	struct {
		char* ds;
		wchar_t* dw;
	};
} d912pxy_file_path;

#define FP_DEF(a) {a, L##a }

static const d912pxy_file_path d912pxy_file_paths_default[] = {
	FP_DEF("./d912pxy/shaders/cs"),
	FP_DEF("cs/cso"),
	FP_DEF("./d912pxy/shaders/hlsl"),
	FP_DEF("./d912pxy/pck/pid.lock"),
	FP_DEF("vfs_archive"),
	FP_DEF("./d912pxy/shaders/hlsl/custom"),
	FP_DEF("shaders/cso"),
	FP_DEF("shaders/bugs"),
	FP_DEF("d912pxy/crash"),
	FP_DEF("d912pxy/log.txt"),
	FP_DEF("d912pxy/log.1.txt"),
	FP_DEF("./d912pxy/dx12_perf_graph.html"),
	FP_DEF("./d912pxy/dx9_perf_graph.html"),
	FP_DEF("./d912pxy/dx12_perf_graph.png"),
	FP_DEF("./d912pxy/dx9_perf_graph.png"),
	FP_DEF("./d912pxy/config.ini"),
	FP_DEF("./d912pxy/12on7/"),
	FP_DEF("."),
	FP_DEF("./d912pxy/imgui.ini"),
	FP_DEF("./d912pxy/imgui.log"),
	FP_DEF("./d912pxy/shaders/pairs/"),
	FP_DEF("./d912pxy/shaders/iframe_mods/"),
	FP_DEF("./d912pxy/"),
	FP_DEF("")
};

static const d912pxy_file_path d912pxy_file_paths_addon[] = {
	FP_DEF("./addons/d912pxy/shaders/cs"),
	FP_DEF("cs/cso"),
	FP_DEF("./addons/d912pxy/shaders/hlsl"),
	FP_DEF("./addons/d912pxy/pck/pid.lock"),
	FP_DEF("vfs_archive"),
	FP_DEF("./addons/d912pxy/shaders/hlsl/custom"),
	FP_DEF("shaders/cso"),
	FP_DEF("shaders/bugs"),
	FP_DEF("./addons/d912pxy/crash"),
	FP_DEF("./addons/d912pxy/log.txt"),
	FP_DEF("./addons/d912pxy/log.1.txt"),
	FP_DEF("./addons/d912pxy/dx12_perf_graph.html"),
	FP_DEF("./addons/d912pxy/dx9_perf_graph.html"),
	FP_DEF("./addons/d912pxy/dx12_perf_graph.png"),
	FP_DEF("./addons/d912pxy/dx9_perf_graph.png"),
	FP_DEF("./addons/d912pxy/config.ini"),
	FP_DEF("./addons/d912pxy/12on7/"),	
	FP_DEF("./addons/"),
	FP_DEF("./addons/d912pxy/imgui.ini"),
	FP_DEF("./addons/d912pxy/imgui.log"),
	FP_DEF("./addons/d912pxy/shaders/pairs/"),
	FP_DEF("./addons/d912pxy/shaders/iframe_mods/"),
	FP_DEF("./addons/d912pxy/"),
	FP_DEF("")
};

static const d912pxy_file_path d912pxy_file_paths_abs_rh[] = {
	FP_DEF("d912pxy/shaders/cs"),
	FP_DEF("cs/cso"),
	FP_DEF("d912pxy/shaders/hlsl"),
	FP_DEF("d912pxy/pck/pid.lock"),
	FP_DEF("vfs_archive"),
	FP_DEF("addons/d912pxy/shaders/hlsl/custom"),
	FP_DEF("shaders/cso"),
	FP_DEF("shaders/bugs"),
	FP_DEF("d912pxy/crash"),
	FP_DEF("d912pxy/log.txt"),
	FP_DEF("d912pxy/log.1.txt"),
	FP_DEF("d912pxy/dx12_perf_graph.html"),
	FP_DEF("d912pxy/dx9_perf_graph.html"),
	FP_DEF("d912pxy/dx12_perf_graph.png"),
	FP_DEF("d912pxy/dx9_perf_graph.png"),
	FP_DEF("d912pxy/config.ini"),
	FP_DEF("d912pxy/12on7/"),
	FP_DEF(""),
	FP_DEF("d912pxy/imgui.ini"),
	FP_DEF("d912pxy/imgui.log"),
	FP_DEF("d912pxy/shaders/pairs/"),
	FP_DEF("d912pxy/shaders/iframe_mods/"),
	FP_DEF("d912pxy/"),
	FP_DEF("")
};

#undef FP_DEF

//forward class defenitions =======================

class d912pxy_basetexture;
class d912pxy_vtexture;
class d912pxy_texture;
class d912pxy_ctexture;
class d912pxy_surface;
class d912pxy_device;
class d912pxy_dheap;
class d912pxy_vdecl;
class d912pxy_gpu_cmd_list;
class d912pxy_replay_base;
class d912pxy_batch;
class d912pxy_shader_db;
class d912pxy_shader_pair;
class d912pxy_iframe;
class d912pxy_upload_pool;
class d912pxy_upload_item;
class d912pxy_vstream_pool;
class d912pxy_vstream;
class d912pxy_surface_pool;
class d912pxy_cleanup_thread;
class d912pxy_gpu_que;
class d912pxy_gpu_cmd_list;
class d912pxy_texture_state;
class d912pxy_surface_layer;
class d912pxy_texture_loader;
class d912pxy_buffer_loader;
class d912pxy_pso_cache;
class d912pxy_batch_buffer;
class d912pxy_pso_cache_item;
class d912pxy_vfs;
class d912pxy_metrics;
class d912pxy_config;
class d912pxy_log;
class d912pxy_mem_mgr;
class d912pxy_StackWalker;
class d912pxy_trimmed_pso_desc;
class d912pxy_query_occlusion;
class d912pxy_com_mgr;
class d912pxy_comhandler;
class d912pxy_vdecl;
class d912pxy_ctexture;
class d912pxy_vtexture;
class d912pxy_base_texture;
class d912pxy_texture;
class d912pxy_extras;
class d912pxy_shader;
struct d912pxy_com_object;
enum d912pxy_replay_item_type;


typedef struct d912pxy_device_streamsrc {
	d912pxy_vstream* buffer;
	UINT offset;
	UINT stride;
	UINT divider;
} d912pxy_device_streamsrc;

typedef enum d912pxy_gpu_cmd_list_group {
	CLG_TOP = 0,
	CLG_TEX = 1,
	CLG_BUF = 2,	
	CLG_RP1 = 3,
	CLG_RP2 = 4,
	CLG_RP3 = 5,
	CLG_RP4 = 6,
	CLG_SEQ = 7
} d912pxy_gpu_cmd_list_group;

static const wchar_t* d912pxy_gpu_cmd_list_group_name [] = {
	L"CLG_TOP",
	L"CLG_TEX",
	L"CLG_BUF",
	L"CLG_RP1",
	L"CLG_RP2",
	L"CLG_RP3",
	L"CLG_RP4",
	L"CLG_SEQ"
};

#define PXY_INNER_MAX_GPU_CMD_LIST_GROUPS 8

//global singletons =======================

#define PXY_INSTANCE_PAR
#define PXY_INSTANCE_PASS 

#define d912pxy_s d912pxy_global::instance
