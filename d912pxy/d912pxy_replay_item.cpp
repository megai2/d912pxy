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
#include "stdafx.h"

const UINT d912pxy_replay_item::dataTypeSize[(UINT)typeName::_count] = {
	GetDataAlignedSize<dt_barrier>(),
	GetDataAlignedSize<dt_om_stencilref>(),
	GetDataAlignedSize<dt_om_blendfactor>(),
	GetDataAlignedSize<dt_view_scissor>(),
	GetDataAlignedSize<dt_draw_indexed>(),
	GetDataAlignedSize<dt_om_render_targets>(),
	GetDataAlignedSize<dt_vbuf_bind>(),
	GetDataAlignedSize<dt_ibuf_bind>(),
	GetDataAlignedSize<dt_clear_rt>(),
	GetDataAlignedSize<dt_clear_ds>(),
	GetDataAlignedSize<dt_pso_raw>(),
	GetDataAlignedSize<dt_pso_raw_feedback>(),
	GetDataAlignedSize<dt_pso_compiled>(),
	GetDataAlignedSize<dt_rect_copy>(),
	GetDataAlignedSize<dt_gpu_write_ctl>(),
	GetDataAlignedSize<dt_ia_prim_topo>(),
	GetDataAlignedSize<dt_query_mark>()
};