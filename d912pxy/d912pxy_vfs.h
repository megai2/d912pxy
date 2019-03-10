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
#define PXY_VFS_MAX_FILES_PER_BID 0x80000
#define PXY_VFS_FILE_HEADER_SIZE 16
#define PXY_VFS_BID_TABLE_SIZE (PXY_VFS_MAX_FILES_PER_BID * PXY_VFS_FILE_HEADER_SIZE)
#define PXY_VFS_BID_TABLE_START 16

#define PXY_VFS_BID_CSO 0
#define PXY_VFS_BID_SHADER_PROFILE 1
#define PXY_VFS_BID_PSO_CACHE_KEYS 2
#define PXY_VFS_BID_PSO_PRECOMPILE_LIST 3

#define PXY_VFS_SIGNATURE 0x443931325043b46
#define PXY_VFS_VER 1

#pragma pack(push, 1)
typedef struct d912pxy_vfs_file_header {
	UINT64 hash;
	UINT64 offset;
} d912pxy_vfs_file_header;
#pragma pack(pop)

class d912pxy_vfs
{
public:
	d912pxy_vfs();
	~d912pxy_vfs();

	void SetRoot(const char* rootPath);
	void* LoadVFS(UINT id, const char* name);

	UINT64 IsPresentN(const char* fnpath, UINT32 vfsId);
	UINT64 IsPresentH(UINT64 fnHash, UINT32 vfsId);

	void* LoadFileN(const char* fnpath, UINT* sz, UINT id);
	void WriteFileN(const char* fnpath, void* data, UINT sz, UINT id);
	void ReWriteFileN(const char* fnpath, void* data, UINT sz, UINT id);

	void* LoadFileH(UINT64 namehash, UINT* sz, UINT id);
	void WriteFileH(UINT64 namehash, void* data, UINT sz, UINT id);
	void ReWriteFileH(UINT64 namehash, void* data, UINT sz, UINT id);

	UINT64 HashFromName(const char* fnpath);

	d912pxy_memtree2* GetHeadTree(UINT id);
	void* GetCachePointer(UINT32 offset, UINT id);
		
private:
	CRITICAL_SECTION lock;

	char m_rootPath[2048];
	FILE* m_vfsBlocks[PXY_VFS_MAX_BID];
	void* m_vfsCache[PXY_VFS_MAX_BID];
	UINT32 m_vfsCacheSize[PXY_VFS_MAX_BID];
	UINT32 m_vfsFileCount[PXY_VFS_MAX_BID];

	d912pxy_memtree2* m_vfsFileOffsets[PXY_VFS_MAX_BID];
	UINT64 m_vfsLastFileOffset[PXY_VFS_MAX_BID];

};

