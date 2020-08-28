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
#pragma once
#include "stdafx.h"

#define PXY_VFS_MAX_BID 256

#define PXY_VFS_LATEST_PCK L"latest.pck"

static const wchar_t* d912pxy_vfs_entry_name_str[] = {
	L"Compiled shader cache",
	L"Shader profiles",
	L"PSO cache keys",
	L"PSO precompile list",
	L"Shader sources",
	L"PSO derived VS code",
	L"PSO derived PS code",
	L"PSO derived alias refs",
	L"VFS original paths"
};

enum class d912pxy_vfs_bid : UINT32 {
	cso = 0,
	shader_profile = 1,
	pso_cache_keys = 2,
	pso_precompile_list = 3,
	shader_sources = 4,
	derived_cso_vs = 5,
	derived_cso_ps = 6,
	derived_cso_refs = 7,
	vfs_paths = 8,
	end = 9
};

typedef UINT64 d912pxy_vfs_path_hash;

class d912pxy_vfs_path {
public:
	d912pxy_vfs_path(const char* fnpath, d912pxy_vfs_bid bid) :
		i_bid(bid)
	{
		i_path = HashFromName(fnpath);
	}

	d912pxy_vfs_path(d912pxy_vfs_path_hash path, d912pxy_vfs_bid bid) :
		i_bid(bid),
		i_path(path)
	{

	}

	~d912pxy_vfs_path()
	{

	}

	d912pxy_vfs_bid bid() { return i_bid; };
	UINT32 bidIndex() { return static_cast<UINT32>(i_bid); };
	d912pxy_vfs_path_hash pathHash() { return i_path; }

private:
	d912pxy_vfs_path_hash HashFromName(const char* fnpath);

	d912pxy_vfs_path_hash i_path;
	d912pxy_vfs_bid i_bid;
};

class d912pxy_vfs_locked_entry 
{
public:
	d912pxy_vfs_locked_entry(d912pxy_vfs_bid bid, d912pxy_vfs_entry** itemArray) :
		i_bid(static_cast<UINT32>(bid))
	{
		i_item = itemArray[i_bid];
		itemLocks[0/*i_bid*/].Hold();
	}

	~d912pxy_vfs_locked_entry()
	{
		itemLocks[0/*i_bid*/].Release();
	}

	d912pxy_vfs_entry* operator->() {
		return i_item;
	};

private:
	UINT32 i_bid;
	d912pxy_vfs_entry* i_item;

	static d912pxy_thread_lock itemLocks[PXY_VFS_MAX_BID];
};

class d912pxy_vfs : public d912pxy_noncom
{
	friend d912pxy_vfs_locked_entry;
public:
	d912pxy_vfs();
	~d912pxy_vfs();

	void Init(const char* lockPath);
	void UnInit();

	void SetRoot(wchar_t* rootPath);
	void LoadVFS();

	bool IsFilePresent(d912pxy_vfs_path path);
	bool ReadFile(d912pxy_vfs_path path, d912pxy_mem_block to);
	d912pxy_mem_block ReadFile(d912pxy_vfs_path path);
	void WriteFile(d912pxy_vfs_path path, d912pxy_mem_block data);		
		
	d912pxy_ringbuffer<d912pxy_vfs_path_hash>* GetFileList(d912pxy_vfs_bid bid);

	UINT32 IsWriteAllowed() { return writeAllowed; };		
	void SetWriteMask(UINT32 val);
	void TakeOutWriteAccess();

	d912pxy_vfs_locked_entry GetBidLocked(d912pxy_vfs_path path);
	d912pxy_vfs_locked_entry GetBidLocked(d912pxy_vfs_bid bid);

	d912pxy_vfs_pck_chunk* WriteFileToPck(d912pxy_vfs_pck_chunk* prevChunk, UINT id, UINT64 namehash, void* data, UINT sz);

private:
	HANDLE lockFile;
	UINT32 writeAllowed;
	char m_rootPath[2048];
	
	d912pxy_vfs_entry* items[PXY_VFS_MAX_BID];

	void LoadPckFromRootPath();

	d912pxy_vfs_pck* cuPck;

	UINT32 cuWriteMask;
};

