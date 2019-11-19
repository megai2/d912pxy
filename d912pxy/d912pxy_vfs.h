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

#define PXY_VFS_MAX_BID 256

#define PXY_VFS_BID_CSO 0
#define PXY_VFS_BID_SHADER_PROFILE 1
#define PXY_VFS_BID_PSO_CACHE_KEYS 2
#define PXY_VFS_BID_PSO_PRECOMPILE_LIST 3
#define PXY_VFS_BID_SHADER_SOURCES 4
#define PXY_VFS_BID_DERIVED_CSO_VS 5
#define PXY_VFS_BID_DERIVED_CSO_PS 6
#define PXY_VFS_BID_END 7

#define PXY_VFS_LATEST_PCK L"latest.pck"

static const wchar_t* d912pxy_vfs_entry_name_str[] = {
	L"Compiled shader cache",
	L"Shader profiles",
	L"PSO cache keys",
	L"PSO precompile list",
	L"Shader sources",
	L"PSO derived VS cache",
	L"PSO derived PS cache"
};

class d912pxy_vfs : public d912pxy_noncom
{
public:
	d912pxy_vfs();
	~d912pxy_vfs();

	void Init(const char* lockPath);
	void UnInit();

	void SetRoot(wchar_t* rootPath);
	void LoadVFS();

	UINT64 IsPresentN(const char* fnpath, UINT32 vfsId);
	void* GetFileDataN(const char* fnpath, UINT* sz, UINT id);
	void WriteFileN(const char* fnpath, void* data, UINT sz, UINT id);
	void ReWriteFileN(const char* fnpath, void* data, UINT sz, UINT id);
	
	UINT64 IsPresentH(UINT64 fnHash, UINT32 vfsId);
	void* GetFileDataH(UINT64 namehash, UINT* sz, UINT id);
	void WriteFileH(UINT64 namehash, void* data, UINT sz, UINT id);
	void ReWriteFileH(UINT64 namehash, void* data, UINT sz, UINT id);

	UINT64 HashFromName(const char* fnpath);
		
	UINT32 IsWriteAllowed() { return writeAllowed; };

	d912pxy_memtree2* GetChunkTree(UINT id) { return items[id]->GetChunkTree(); };

	d912pxy_vfs_pck_chunk* WriteFileToPck(d912pxy_vfs_pck_chunk* prevChunk,UINT id, UINT64 namehash, void* data, UINT sz);

	void SetWriteMask(UINT32 val);
	void TakeOutWriteAccess();

private:
	HANDLE lockFile;
	UINT32 writeAllowed;
	char m_rootPath[2048];

	d912pxy_vfs_entry* items[PXY_VFS_MAX_BID];

	void LoadPckFromRootPath();

	d912pxy_vfs_pck* cuPck;

	d912pxy_thread_lock lock;	

	UINT32 cuWriteMask;
};

