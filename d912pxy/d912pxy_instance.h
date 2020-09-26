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
#include "stdafx.h"

IDirect3DDevice9* app_cb_D3D9Dev_create(IDirect3DDevice9Proxy* dev, IDirect3D9* obj);

typedef struct d912pxy_instance {
	d912pxy_instance() : dynamicFilePaths(nullptr), running(0) {};
	~d912pxy_instance() {  };

	struct pool {
		d912pxy_vstream_pool vstream;
		d912pxy_surface_pool surface;
		d912pxy_surface_pool rtds;
		d912pxy_upload_pool upload;
		d912pxy_mem_va_table hostPow2;
	} pool;

	struct thread {
		d912pxy_cleanup_thread cleanup;
		d912pxy_texture_loader texld;
		d912pxy_buffer_loader bufld;
	} thread;

	struct log {
		d912pxy_metrics metrics;
		d912pxy_log text;
	} log;

	struct render {

		d912pxy_iframe iframe;

		struct state {
			d912pxy_texture_state tex;
			d912pxy_dx9_pipeline_state pso;
		} state;

		d912pxy_batch_buffer batch;
		d912pxy_draw_up draw_up;
		d912pxy_replay replay;

		struct db {
			d912pxy_pso_db pso;
			d912pxy_shader_db shader;
			d912pxy_pso_mt_dispatcher psoMTCompiler;
		} db;
	} render;

	struct dx12 {
		d912pxy_gpu_que que;
		d912pxy_gpu_cmd_list* cl;
		ID3D12Device* dev;
	} dx12;

	union {
		void* dev_vtable;
		d912pxy_device dev;
	};
	
	d912pxy_vfs vfs;		
	d912pxy_config config;
	d912pxy_file_path* dynamicFilePaths;

	d912pxy_com_mgr com;
	d912pxy_mem_mgr mem;

	d912pxy_dynamic_imports imports;

	d912pxy_extras extras;
	//TODO: maybe move it to another category
	d912pxy::extras::ShaderPair::InfoStorage spairInfo;
	d912pxy::extras::IFrameMods::Manager iframeMods;

	UINT running;
} d912pxy_instance;

class d912pxy_global 
{
	public:
		static d912pxy_instance instance;
};

void d912pxy_first_init();
void d912pxy_final_cleanup();