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

d912pxy_vfs_file_header s_headerTable[PXY_VFS_MAX_FILES_PER_BID];

d912pxy_vfs::d912pxy_vfs()
{

}


d912pxy_vfs::~d912pxy_vfs()
{	
	for (int i = 0; i != PXY_VFS_MAX_BID; ++i)
	{
		d912pxy_vfs_entry* item = &items[i];
		if (item->m_vfsBlocks != NULL)
		{
			fflush(item->m_vfsBlocks);
			fclose(item->m_vfsBlocks);

			delete item->m_vfsFileOffsets;

			if (item->m_vfsCache)
				PXY_FREE(item->m_vfsCache);
		}
	}

	if (writeAllowed)
	{		
		CloseHandle(lockFile);
	}
}

void d912pxy_vfs::Init(const char * lockPath)
{	
	lockFile = CreateFileA(lockPath, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_DELETE_ON_CLOSE, NULL);
	if (lockFile == INVALID_HANDLE_VALUE)
	{
		writeAllowed = 0;
	}
	else {
		DWORD pid = GetProcessId(GetCurrentProcess());
		DWORD ret = 0;
		WriteFile(lockFile, &pid, 4, &ret, NULL);
		writeAllowed = 1;
	}

	ZeroMemory(items, sizeof(d912pxy_vfs_entry)*PXY_VFS_MAX_BID);	

	for (int i = 0; i != PXY_VFS_MAX_BID; ++i)
	{
		items[i].lock.Init();
	}
}

void d912pxy_vfs::SetRoot(wchar_t * rootPath)
{
	sprintf(m_rootPath, "%s/%ws", d912pxy_helper::GetFilePath(FP_VFS_PREFIX)->s, rootPath);
}

void* d912pxy_vfs::LoadVFS(d912pxy_vfs_id_name* id, UINT memCache)
{
	char fn[4096];

	sprintf(fn, "%s/%s.pck", m_rootPath, id->name);

	d912pxy_vfs_entry* item = &items[id->num];

	item->m_vfsBlocks = fopen(fn, "rb+");

	if (item->m_vfsBlocks == NULL)
		item->m_vfsBlocks = fopen(fn, "wb+");

	if (item->m_vfsBlocks == NULL)
	{
		return NULL;
	}

	fseek(item->m_vfsBlocks, 0, SEEK_END);
		
	UINT sz = ftell(item->m_vfsBlocks);

	fseek(item->m_vfsBlocks, 0, SEEK_SET);

	item->m_vfsFileOffsets = new d912pxy_memtree2(8, 256, 2);

	item->m_vfsLastFileOffset = PXY_VFS_BID_TABLE_SIZE + PXY_VFS_BID_TABLE_START;
	item->m_vfsFileCount = 0;
	
	ZeroMemory(s_headerTable, PXY_VFS_BID_TABLE_SIZE);

	UINT64 signature[2] = { PXY_VFS_SIGNATURE, PXY_VFS_VER };

	if (sz < PXY_VFS_BID_TABLE_SIZE + PXY_VFS_BID_TABLE_START)
	{					
		if (fwrite(signature, 8, 2, item->m_vfsBlocks) != 2)
			return NULL;
		
		fwrite(s_headerTable, PXY_VFS_FILE_HEADER_SIZE, PXY_VFS_MAX_FILES_PER_BID, item->m_vfsBlocks);
		fwrite(&sz, 1, 4, item->m_vfsBlocks);

		fflush(item->m_vfsBlocks);
	}
	else {

		UINT64 readedSignature[2] = { 0,0 };

		if (fread(readedSignature, 8, 2, item->m_vfsBlocks) != 2)
			return NULL;

		if (memcmp(signature, readedSignature, 16))
		{
			return NULL;
		}

		fseek(item->m_vfsBlocks, 0, SEEK_SET);

		if (fwrite(signature, 8, 2, item->m_vfsBlocks) != 2)
			return NULL;

		fseek(item->m_vfsBlocks, 16, SEEK_SET);

		fread(s_headerTable, PXY_VFS_FILE_HEADER_SIZE, PXY_VFS_MAX_FILES_PER_BID, item->m_vfsBlocks);
		
		for (int i = 0; i != PXY_VFS_MAX_FILES_PER_BID; ++i)
		{
			if (s_headerTable[i].offset > item->m_vfsLastFileOffset)			
				item->m_vfsLastFileOffset = s_headerTable[i].offset;

			if (s_headerTable[i].hash != 0)
			{
				item->m_vfsFileOffsets->PointAtMem(&s_headerTable[i].hash, 8);
				item->m_vfsFileOffsets->SetValue(s_headerTable[i].offset);
				++item->m_vfsFileCount;
			}
		}

		if (item->m_vfsLastFileOffset > 0)
		{

			fseek(item->m_vfsBlocks, (UINT32)item->m_vfsLastFileOffset, SEEK_SET);

			UINT32 lastFileSize;

			fread(&lastFileSize, 4, 1, item->m_vfsBlocks);

			item->m_vfsLastFileOffset += lastFileSize + 4;
		}
		else
			item->m_vfsLastFileOffset = PXY_VFS_BID_TABLE_SIZE + PXY_VFS_BID_TABLE_START;

		if (item->m_vfsLastFileOffset == sz)
		{
			fseek(item->m_vfsBlocks, 0, SEEK_END);
			fwrite(&sz, 1, 4, item->m_vfsBlocks);
			fflush(item->m_vfsBlocks);
		}
		
		item->m_vfsCacheSize = (UINT32)item->m_vfsLastFileOffset - PXY_VFS_BID_TABLE_SIZE - PXY_VFS_BID_TABLE_START;

		if (item->m_vfsCacheSize && memCache)
		{
			PXY_MALLOC(item->m_vfsCache, item->m_vfsCacheSize, void*);


			fseek(item->m_vfsBlocks, PXY_VFS_BID_TABLE_SIZE+PXY_VFS_BID_TABLE_START, SEEK_SET);

			fread(item->m_vfsCache, 1, item->m_vfsCacheSize, item->m_vfsBlocks);
		}
		else {
			item->m_vfsCache = 0;
			item->m_vfsCacheSize = 0;
		}
	}	

	return item->m_vfsBlocks;
}

UINT64 d912pxy_vfs::IsPresentN(const char * fnpath, UINT32 vfsId)
{	
	return IsPresentH(HashFromName(fnpath), vfsId);
}

UINT64 d912pxy_vfs::IsPresentH(UINT64 fnHash, UINT32 vfsId)
{
	d912pxy_vfs_entry* item = &items[vfsId];

	{
		if (item->m_vfsBlocks != NULL)
		{
			item->m_vfsFileOffsets->PointAtMem(&fnHash, 8);
			UINT64 offset = item->m_vfsFileOffsets->CurrentCID();

			if (offset)
			{
				return offset;
			}
		}
	}

	return 0;
}

void * d912pxy_vfs::LoadFileN(const char * fnpath, UINT * sz, UINT id)
{
	return LoadFileH(HashFromName(fnpath), sz, id);
}

void d912pxy_vfs::WriteFileN(const char * fnpath, void * data, UINT sz, UINT id)
{
	WriteFileH(HashFromName(fnpath), data, sz, id);
}

void d912pxy_vfs::ReWriteFileN(const char * fnpath, void * data, UINT sz, UINT id)
{
	ReWriteFileH(HashFromName(fnpath), data, sz, id);
}

void * d912pxy_vfs::LoadFileH(UINT64 namehash, UINT * sz, UINT id)
{
	d912pxy_vfs_entry* item = &items[id];

	item->lock.Hold();
	
	UINT64 offset = IsPresentH(namehash, id);

	if (!offset)
	{
		item->lock.Release();
		return nullptr;
	}

	if (item->m_vfsCache && ((offset - PXY_VFS_BID_TABLE_SIZE - PXY_VFS_BID_TABLE_START) < item->m_vfsCacheSize))
	{
		offset -= PXY_VFS_BID_TABLE_SIZE + PXY_VFS_BID_TABLE_START;

		item->lock.Release();

		*sz = *((UINT32*)((intptr_t)item->m_vfsCache + offset));


		void* ret = NULL;

		PXY_MALLOC(ret, *sz, void*);

		memcpy(ret, ((void*)((intptr_t)item->m_vfsCache + offset + 4)), *sz);

		return ret;
	}

	fseek(item->m_vfsBlocks, (UINT32)offset, SEEK_SET);

	fread(sz, 4, 1, item->m_vfsBlocks);


	void* ret = NULL;
	PXY_MALLOC(ret, *sz, void*);

	fread(ret, 1, *sz, item->m_vfsBlocks);

	item->lock.Release();

	return ret;
}

void d912pxy_vfs::WriteFileH(UINT64 namehash, void * data, UINT sz, UINT id)
{
	if (!writeAllowed)
		return;

	d912pxy_vfs_entry* item = &items[id];

	item->lock.Hold();

	fseek(item->m_vfsBlocks, PXY_VFS_FILE_HEADER_SIZE*item->m_vfsFileCount + PXY_VFS_BID_TABLE_START, SEEK_SET);

	fwrite(&namehash, 8, 1, item->m_vfsBlocks);
	fwrite(&item->m_vfsLastFileOffset, 8, 1, item->m_vfsBlocks);
	++item->m_vfsFileCount;

	fseek(item->m_vfsBlocks, (UINT32)item->m_vfsLastFileOffset, SEEK_SET);

	fwrite(&sz, 1, 4, item->m_vfsBlocks);
	fwrite(data, 1, sz, item->m_vfsBlocks);
	fwrite(&sz, 1, 4, item->m_vfsBlocks);
	fflush(item->m_vfsBlocks);

	item->m_vfsFileOffsets->PointAtMem(&namehash, 8);
	item->m_vfsFileOffsets->SetValue(item->m_vfsLastFileOffset);

	item->m_vfsLastFileOffset += sz + 4;

	item->lock.Release();
}

void d912pxy_vfs::ReWriteFileH(UINT64 namehash, void * data, UINT sz, UINT id)
{
	if (!writeAllowed)
		return;

	d912pxy_vfs_entry* item = &items[id];

	item->lock.Hold();

	UINT64 offset = IsPresentH(namehash, id);

	if (offset)
	{
		fseek(item->m_vfsBlocks, (UINT32)offset + 4, SEEK_SET);
		fwrite(data, 1, sz, item->m_vfsBlocks);
		fflush(item->m_vfsBlocks);

		if (item->m_vfsCache && (item->m_vfsCacheSize > (offset - PXY_VFS_BID_TABLE_SIZE - PXY_VFS_BID_TABLE_START)))
			memcpy((void*)((intptr_t)item->m_vfsCache + (offset - PXY_VFS_BID_TABLE_SIZE - PXY_VFS_BID_TABLE_START) + 4), data, sz);
	}
	else
		WriteFileH(namehash, data, sz, id);

	item->lock.Release();
}

UINT64 d912pxy_vfs::HashFromName(const char * fnpath)
{
	return d912pxy_memtree2::memHash64s((void*)fnpath, (UINT32)strlen(fnpath));
}

d912pxy_memtree2 * d912pxy_vfs::GetHeadTree(UINT id)
{
	d912pxy_vfs_entry* item = &items[id];
	return item->m_vfsFileOffsets;
}

void * d912pxy_vfs::GetCachePointer(UINT32 offset, UINT id)
{
	d912pxy_vfs_entry* item = &items[id];
	offset -= PXY_VFS_BID_TABLE_SIZE + PXY_VFS_BID_TABLE_START;
	return ((void*)((intptr_t)item->m_vfsCache + offset + 4));
}
