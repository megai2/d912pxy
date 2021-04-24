/*
MIT License

Copyright(c) 2021 megai2

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

namespace d912pxy {
	namespace extras {
		namespace IFrameMods {

			class NativeShader : public d912pxy_noncom
			{
				ID3D12PipelineState* pso = nullptr;
				D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = { 0 };
				ComPtr<ID3DBlob> vsBlob;
				ComPtr<ID3DBlob> psBlob;
				D3D12_INPUT_ELEMENT_DESC defaultFv4Pos = { 0 };

				void compile();
				ComPtr<ID3DBlob> CompileSh(const char* profile, const wchar_t* shName);

			public:
				NativeShader(const wchar_t* shName);
				~NativeShader() { }

				D3D12_GRAPHICS_PIPELINE_STATE_DESC& getDesc() { return desc; }
			
				ID3D12PipelineState* ptr() 
				{ 
					if (!pso) 
						compile(); 
					return pso; 
				}
			};

		} //namespace IFrameMods
	} //namespace extras
} //namespace d912pxy
