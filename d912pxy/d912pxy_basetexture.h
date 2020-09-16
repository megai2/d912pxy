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

#pragma pack(push, 1)

struct d912pxy_basetexture_cache_data
{
	UINT32 srvId;
	union 
	{
		struct 
		{
			UINT16 shouldBarrier;
			UINT16 compareFormat;
		};
		UINT32 extraData;
	};
};

#pragma pack(pop)

class d912pxy_basetexture : private d912pxy_vtable, public d912pxy_resource
{
public:
	d912pxy_basetexture();
	~d912pxy_basetexture();

	UINT GetSRVHeapId();
	UINT GetSRVHeapId(UINT mode);
	bool UsesCompareFormat() { return attachedCache.compareFormat > 0; }

	D912PXY_METHOD_(DWORD, SetLOD)(PXY_THIS_ DWORD LODNew);
	D912PXY_METHOD_(DWORD, GetLOD)(PXY_THIS);
	D912PXY_METHOD_(DWORD, GetLevelCount)(PXY_THIS);
	D912PXY_METHOD(SetAutoGenFilterType)(PXY_THIS_ D3DTEXTUREFILTERTYPE FilterType);
	D912PXY_METHOD_(D3DTEXTUREFILTERTYPE, GetAutoGenFilterType)(PXY_THIS);
	D912PXY_METHOD_(void, GenerateMipSubLevels)(PXY_THIS);

	D912PXY_METHOD_(DWORD, GetPriority_SRVhack)(PXY_THIS);

	D912PXY_METHOD_NC_(DWORD, GetPriority_SRVhack)(THIS);

	d912pxy_surface* GetBaseSurface() { return baseSurface; };

	UINT FinalRelease();

protected:
	d912pxy_basetexture_cache_data attachedCache;

	d912pxy_surface * baseSurface = nullptr;

	UINT m_levels;	
};