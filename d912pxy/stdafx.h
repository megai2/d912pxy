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

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "build_version.h"

#include <atomic>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <d3d9.h>
#include <map>
#include <unordered_map>
#include <algorithm>

// DirectX 12 specific headers.
#include <d3d12.h>
#include <dxgidebug.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <wrl.h>
#include "../thirdparty/D3D12Downlevel.h"

#include "targetver.h"
#include <chrono>
#include <type_traits>
#include "stopwatch.h"
#include "d912pxy_performance_graph.h"

#ifndef DISABLE_P7LIB
	#include "../thirdparty/p7logger/Headers/P7_Trace.h"
	#include "../thirdparty/p7logger/Headers/P7_Telemetry.h"
#endif

#include "v3/base_object.h"
#include "v3/util/log/error_handler.h"
#include "v3/util/memory_block.h"
#include "v3/util/hash.h"
#include "v3/util/trivial/linear_array.h"
#include "v3/util/trivial/push_buffer.h"
#include "v3/util/mt/lock.h"
#include "v3/util/mt/container.h"
#include "v3/util/memtree.h"
#include "v3/util/dir_reader.h"
#include "v3/util/key_value_reader.h"

#include "../thirdparty/imgui/imgui.h"
#include "../thirdparty/imgui/imgui_impl_dx12.h"
#include "../thirdparty/imgui/imgui_impl_win32.h"
#include "../thirdparty/nv_api/nv_api_oc.h"
#include "dbg_imagewriter.h"
#include "stb_image.h"
#include "../thirdparty/fastlz/fastlz.h"
#include "IDirect3D9Proxy.h"
#include "IDirect3DDevice9Proxy.h"
#include "d912pxy_com_routing.h"
#include "d3d9_proxy_dll.h"
#include "d912pxy_thread_lock.h"
#include "d912pxy_fwdecl.h"
#include "d912pxy_log.h"
#include "d912pxy_thread.h"
#include "d912pxy_helper.h"
#include "d912pxy_noncom.h"
#include "d912pxy_dynamic_imports.h"
#include "d912pxy_mem_block.h"
#include "d912pxy_mem_mgr.h"
#include "d912pxy_mem_va_table.h"
#include "d912pxy_com_mgr.h"
#include "d912pxy_config.h"
#include "d912pxy_metrics.h"
#include "d912pxy_ringbuffer.h"
#include "d912pxy_swap_list.h"
#include "d912pxy_comhandler.h"
#include "d912pxy_linked_list.h"
#include "d912pxy_cleanup_thread.h"
#include "d912pxy_vfs_pck.h"
#include "d912pxy_vfs_entry.h"
#include "d912pxy_vfs.h"
#include "d912pxy_vfs_packer.h"
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
#include "d912pxy_d3dx9.h"
#include "d912pxy_dxbc9.h"
#include "d912pxy_hlsl_gen.h"
#include "d912pxy_shader_replacer.h"
#include "d912pxy_trimmed_pso.h"
#include "d912pxy_pso_item.h"
#include "d912pxy_shader.h"
#include "d912pxy_shader_db.h"
#include "d912pxy_shader_pair.h"
#include "d912pxy_query.h"
#include "d912pxy_query_occlusion.h"
#include "d912pxy_dheap.h"
#include "d912pxy_cbuffer.h"
#include "d912pxy_gpu_cleanup_thread.h"
#include "d912pxy_gpu_cmd_list.h"
#include "d912pxy_gpu_que.h"
#include "d912pxy_texture_state.h"
#include "d912pxy_pso_mt_dispatcher.h"
#include "d912pxy_pso_db.h"
#include "d912pxy_dx9_pipeline_state.h"
#include "d912pxy_folded_buffer.h"
#include "d912pxy_batch.h"
#include "d912pxy_iframe.h"
#include "d912pxy_async_upload_thread.h"
#include "d912pxy_texture_loader.h"
#include "d912pxy_buffer_loader.h"
#include "d912pxy_replay_item.h"
#include "d912pxy_replay_buffer.h"
#include "d912pxy_replay_thread.h"
#include "d912pxy_replay.h"
#include "d912pxy_draw_up.h"
#include "d912pxy_surface_ops.h"
#include "v3/dx12/extras/shader_pair/info.h"
#include "v3/dx12/extras/shader_pair/tracker.h"
#include "v3/dx12/extras/iframe_mods/manager.h"
#include "v3/dx12/extras/iframe_mods/state_holder.h"
#include "v3/dx12/extras/iframe_mods/pass_detector.h"
#include "v3/dx12/extras/iframe_mods/native_draw.h"
#include "v3/dx12/extras/iframe_mods/generic_taa.h"
#include "v3/dx12/extras/iframe_mods/gw2_taa.h"
#include "d912pxy_extras.h"
#include "d912pxy_device.h"
#include "d912pxy_instance.h"
#include "d912pxy_com_object.h"
