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

typedef struct d912pxy_com_object {	
	union {
		void* vtable;
		d912pxy_comhandler_non_derived com;
		d912pxy_vdecl vdecl;
		d912pxy_resource res;
		d912pxy_shader shader;
		d912pxy_surface surface;
		d912pxy_surface_layer layer;
		d912pxy_device device;
		d912pxy_vtexture tex_3d;
		d912pxy_ctexture tex_cube;
		d912pxy_texture tex_2d;
		d912pxy_basetexture basetex;
		d912pxy_vstream vstream;
		d912pxy_query_non_derived query;
		d912pxy_query_occlusion query_occ;
		d912pxy_swapchain swapchain;
		d912pxy_sblock sblock;
		d912pxy_pso_item pso_item;
	};
} d912pxy_com_object;