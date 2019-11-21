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
#pragma once
#include "stdafx.h"

class d912pxy_shader_profile : public d912pxy_noncom 
{
public:	
	~d912pxy_shader_profile();

	typedef union entry_data
	{
		UINT32 stage;
		UINT32 enable;	
		UINT32 raw;
	} entry_data;

	enum class entry : UINT32 {
		pcf_sampler = 0,
		alpha_test,
		srgb_read,
		srgb_write,
		clipplane0,
		uint_normals,
		uint_tangents,
		reserve2,
		reserve3,
		count
	};
	
	void entryEnable(entry name);
	void entryStageSelect(entry name, DWORD stage);

	bool entryEnabled(entry name);
	bool entryStageSelected(entry name, DWORD stage);
		
	bool isValid() { return shader != 0; };

	static d912pxy_shader_profile load(d912pxy_shader_uid shdUID);
	static d912pxy_shader_profile unknown(d912pxy_shader* shader);

	void ignoreChanges() { dirty = false; };

private:
	d912pxy_shader_profile(d912pxy_shader_uid shdUID);

	void setEntryData(entry name, entry_data data);
	entry_data getEntry(entry name);

	void LoadDataFromVFS();
	
	static const UINT32 entryCount = static_cast<UINT32>(entry::count);
	static const UINT32 dataSize = entryCount * sizeof(entry_data);

	entry_data entryArray[entryCount];

	d912pxy_shader_uid shader;
	bool loaded = false;
	bool dirty = false;
};
