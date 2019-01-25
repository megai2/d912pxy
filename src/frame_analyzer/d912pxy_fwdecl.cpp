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
#include "stdafx.h"

d912pxy_s_dcl(sdb, d912pxy_shader_db);
d912pxy_s_dcl(pool_vstream, d912pxy_vstream_pool);
d912pxy_s_dcl(pool_surface, d912pxy_surface_pool);
d912pxy_s_dcl(thread_cleanup, d912pxy_cleanup_thread);
d912pxy_s_dcl(iframe, d912pxy_iframe);
d912pxy_s_dcl(GPUque, d912pxy_gpu_que);
d912pxy_s_dcl(GPUcl, d912pxy_gpu_cmd_list);
d912pxy_s_dcl(DXDev, ID3D12Device1);
d912pxy_s_dcl(CMDReplay, d912pxy_replay);
d912pxy_s_dcl(textureState, d912pxy_texstage_cache);
d912pxy_s_dcl(texloadThread, d912pxy_texture_loader);
d912pxy_s_dcl(pool_upload, d912pxy_upload_pool);
d912pxy_s_dcl(batch, d912pxy_batch);
d912pxy_s_dcl(samplerState, d912pxy_sampler_cache);
d912pxy_s_dcl(psoCache, d912pxy_pso_cache);
d912pxy_s_dcl(vfs, d912pxy_vfs);
