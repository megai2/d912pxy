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

#define _CRT_SECURE_NO_WARNINGS

#include "build_version.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <d3d9.h>
#include <map>
#include <unordered_map>

// DirectX 12 specific headers.
#include <d3d12.h>
#include <dxgidebug.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <wrl.h>

#include "targetver.h"
#include <chrono>
#include <type_traits>
#include "stopwatch.h"
#include "d912pxy_performance_graph.h"

#ifndef DISABLE_P7LIB
	#include "../thirdparty/p7logger/Headers/P7_Trace.h"
	#include "../thirdparty/p7logger/Headers/P7_Telemetry.h"
#endif

#include "dbg_imagewriter.h"
#include "stb_image.h"
#include "IDirect3D9Proxy.h"
#include "IDirect3DDevice9Proxy.h"
#include "d3d9_proxy_dll.h"
#include "d912pxy_thread_lock.h"
#include "d912pxy_fwdecl.h"
#include "d912pxy_log.h"
#include "d912pxy_com_tools.h"
#include "d912pxy_thread.h"
#include "d912pxy_helper.h"
#include "d912pxy_noncom.h"
#include "d912pxy_config.h"
#include "d912pxy_metrics.h"
#include "d912pxy_ringbuffer.h"
#include "d912pxy_comhandler.h"
#include "d912pxy_linked_list.h"
#include "d912pxy_cleanup_thread.h"
#include "d912pxy_memtree.h"
#include "d912pxy_memtree2.h"
#include "d912pxy_vfs.h"
#include "d912pxy_swapchain.h"
#include "d912pxy_resource.h"
#include "d912pxy_basetexture.h"
#include "d912pxy_pool.h"
#include "d912pxy_pool_memcat.h"
#include "d912pxy_upload_pool.h"
#include "d912pxy_texture.h"
#include "d912pxy_vtexture.h"
#include "d912pxy_ctexture.h"
#include "d912pxy_vstream.h"
#include "d912pxy_vstream_pool.h"
#include "d912pxy_surface.h"
#include "d912pxy_surface_layer.h"
#include "d912pxy_surface_pool.h"
#include "d912pxy_sblock.h"
#include "d912pxy_vdecl.h"
#include "d912pxy_shader_replacer.h"
#include "d912pxy_shader.h"
#include "d912pxy_pshader.h"
#include "d912pxy_vshader.h"
#include "d912pxy_shader_db.h"
#include "d912pxy_shader_pair.h"
#include "d912pxy_hlsl_generator.h"
#include "d912pxy_query.h"
#include "d912pxy_dheap.h"
#include "d912pxy_cbuffer.h"
#include "d912pxy_gpu_cleanup_thread.h"
#include "d912pxy_gpu_cmd_list.h"
#include "d912pxy_gpu_que.h"
#include "d912pxy_texture_state.h"
#include "d912pxy_pso_cache.h"
#include "d912pxy_batch.h"
#include "d912pxy_iframe.h"
#include "d912pxy_async_upload_thread.h"
#include "d912pxy_texture_loader.h"
#include "d912pxy_buffer_loader.h"
#include "d912pxy_replay_thread.h"
#include "d912pxy_replay.h"
#include "d912pxy_device.h"

#include "frame_analyzer.h"