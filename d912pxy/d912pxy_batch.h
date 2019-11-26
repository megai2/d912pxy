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

#define PXY_BATCH_GPU_ELEMENT_OFFSET_TEXBINDS 0
#define PXY_BATCH_GPU_ELEMENT_OFFSET_SAMPLERS 8
#define PXY_BATCH_GPU_ELEMENT_OFFSET_SHADER_VARS_VERTEX 16
#define PXY_BATCH_GPU_ELEMENT_OFFSET_SHADER_VARS_PIXEL 272

#define PXY_BATCH_GPU_TRANSIT_ELEMENTS 530
#define PXY_BATCH_GPU_ELEMENT_COUNT 544


#pragma pack(push, 1)

typedef struct d912pxy_batch_buffer_sub_element {
	union {
		float f[4];
		UINT32 ui[4];
		INT32 i[4];
	};
} d912pxy_batch_buffer_sub_element;

typedef struct d912pxy_batch_buffer_element {
	d912pxy_batch_buffer_sub_element texture[8];
	d912pxy_batch_buffer_sub_element sampler[8];
	d912pxy_batch_buffer_sub_element vs_vars[256];
	d912pxy_batch_buffer_sub_element ps_vars[256];
	d912pxy_batch_buffer_sub_element clipPlane0;
	d912pxy_batch_buffer_sub_element halfpixelFix;
	d912pxy_batch_buffer_sub_element ext_tex;
	d912pxy_batch_buffer_sub_element ext_spl;
	d912pxy_batch_buffer_sub_element ext_vars[12];
} d912pxy_batch_buffer_element;

#pragma pack(pop)

class d912pxy_batch_buffer : public d912pxy_folded_buffer<d912pxy_batch_buffer_element, d912pxy_batch_buffer_sub_element>
{
public:
	d912pxy_batch_buffer();
	~d912pxy_batch_buffer();

	void Init();
	void UnInit();
	
	void SetShaderConstF(UINT type, UINT start, UINT cnt4, float* data);
	void GPUWrite(void* src, UINT size, UINT offset);	

	UINT FinishCurrentDraw();
	
	void FrameStart();
	void FrameEnd();
	
	void Bind(ID3D12GraphicsCommandList* cl, UINT batch);

	UINT32 GetBatchCount() { return batchNum; };
	   
private:								
	UINT32 batchNum;

	BYTE doNewBatch;
	BYTE forceNewBatch;
};
