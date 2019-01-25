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

d912pxy_sblock::d912pxy_sblock(d912pxy_device * dev, D3DSTATEBLOCKTYPE Type) : d912pxy_comhandler(dev, L"state block")
{
}

d912pxy_sblock::~d912pxy_sblock()
{

}

#define D912PXY_METHOD_IMPL_CN d912pxy_sblock

D912PXY_IUNK_IMPL

D912PXY_METHOD_IMPL(GetDevice)(THIS_ IDirect3DDevice9** ppDevice) { *ppDevice = m_dev; return D3D_OK; }

D912PXY_METHOD_IMPL(Capture)(THIS)
{
	//megai2: must save write in all states tagged to this block
	LOG_DBG_DTDM(__FUNCTION__);

	return D3D_OK;
}

D912PXY_METHOD_IMPL(Apply)(THIS)
{
	//megai2: must apply all tagged states from this block
	LOG_DBG_DTDM(__FUNCTION__);

	return D3D_OK;
}

#undef D912PXY_METHOD_IMPL_CN
