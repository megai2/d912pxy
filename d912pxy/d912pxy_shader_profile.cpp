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

d912pxy_shader_profile::d912pxy_shader_profile(d912pxy_shader_uid shdUID) :
	shader(shdUID)
{ }

d912pxy_shader_profile::~d912pxy_shader_profile()
{
	if (dirty)
		d912pxy_s.vfs.WriteFile(d912pxy_vfs_path(shader, d912pxy_vfs_bid::shader_profile), d912pxy_mem_block::use(entryArray, dataSize));		
}

void d912pxy_shader_profile::entryEnable(entry name)
{
	entry_data v;
	v.enable = 1;

	setEntryData(name, v);
}

void d912pxy_shader_profile::entryStageSelect(entry name, DWORD stage)
{
	entry_data v;
	v.stage = stage + 1;

	setEntryData(name, v);
}

bool d912pxy_shader_profile::entryEnabled(entry name)
{
	return getEntry(name).enable != 0;
}

bool d912pxy_shader_profile::entryStageSelected(entry name, DWORD stage)
{
	if (!getEntry(name).stage)
		return false;
	else 
		return (getEntry(name).stage - 1) == stage;
}

d912pxy_shader_profile::entry_data d912pxy_shader_profile::getEntry(entry name)
{
	return entryArray[static_cast<UINT32>(name)];
}

d912pxy_shader_profile d912pxy_shader_profile::load(d912pxy_shader_uid shdUID)
{
	auto ret = d912pxy_shader_profile(shdUID);

	ret.LoadDataFromVFS();

	return ret;
}

d912pxy_shader_profile d912pxy_shader_profile::unknown(d912pxy_shader* shader)
{
	if (!shader)
	{
		auto ret = d912pxy_shader_profile(0);		
		return ret;
	} else
		return d912pxy_shader_profile(shader->GetID());
}

void d912pxy_shader_profile::setEntryData(entry name, entry_data data)
{
	if (!loaded)
		LoadDataFromVFS();

	if (data.raw != getEntry(name).raw)
	{
		entryArray[static_cast<UINT32>(name)] = data;

		dirty = true;		
	}
}

void d912pxy_shader_profile::LoadDataFromVFS()
{
	//d912pxy_s.vfs.WriteFile(d912pxy_vfs_path(shader, d912pxy_vfs_bid::shader_profile), d912pxy_mem_block::use(entryArray, dataSize));

	d912pxy_vfs_path vfsPath = d912pxy_vfs_path(shader, d912pxy_vfs_bid::shader_profile);
	d912pxy_mem_block mem = d912pxy_mem_block::use(entryArray, dataSize);

	if (!d912pxy_s.vfs.ReadFile(vfsPath, mem))
	{
		mem.FillZero();
		d912pxy_s.vfs.WriteFile(vfsPath, mem);
	}

	loaded = true;
}
