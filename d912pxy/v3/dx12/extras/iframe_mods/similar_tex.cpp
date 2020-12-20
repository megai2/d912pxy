/*
MIT License

Copyright(c) 2020 megai2

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

d912pxy_surface* d912pxy::extras::IFrameMods::SimilarTex::surfFromTempl(D3DSURFACE_DESC& descTempl)
{
	UINT levels = 1;
	d912pxy_surface* ret = d912pxy_surface::d912pxy_surface_com(
		descTempl.Width,
		descTempl.Height,
		descTempl.Format,
		descTempl.Usage,
		descTempl.MultiSampleType,
		descTempl.MultiSampleQuality,
		false,
		&levels,
		0,
		nullptr
	);

	//TODO: ensure we have rtds at first TAA draw call
	ret->GetSRVHeapIdRTDS();

	return ret;
}

d912pxy::extras::IFrameMods::SimilarTex::~SimilarTex()
{
	UnInit();
}

void d912pxy::extras::IFrameMods::SimilarTex::UnInit()
{
	if (surf)
		surf->Release();
	surf = nullptr;
}

bool d912pxy::extras::IFrameMods::SimilarTex::syncFrom(d912pxy_surface* source)
{
	D3DSURFACE_DESC actualDesc = source->GetDX9DescAtLevel(0);

	if (surf)
	{
		D3DSURFACE_DESC oldDesc = surf->GetDX9DescAtLevel(0);
		if ((oldDesc.Height != actualDesc.Height) || (oldDesc.Width != actualDesc.Width))
		{
			surf->Release();
			surf = nullptr;
		}
	} 

	if (!surf)
	{
		surf = surfFromTempl(actualDesc);
		return true;
	}
	else
		return false;
}
