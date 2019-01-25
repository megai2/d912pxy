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
	d912pxy_s(vfs) = this;

	ZeroMemory(m_vfsBlocks, sizeof(FILE*)*PXY_VFS_MAX_BID);
	ZeroMemory(m_vfsFileOffsets, sizeof(d912pxy_memtree*)*PXY_VFS_MAX_BID);
	ZeroMemory(m_vfsCache, sizeof(void*)*PXY_VFS_MAX_BID);

	InitializeCriticalSection(&lock);
}


d912pxy_vfs::~d912pxy_vfs()
{
	for (int i = 0; i != PXY_VFS_MAX_BID; ++i)
	{
		if (m_vfsBlocks[i] != NULL)
		{
			fseek(m_vfsBlocks[i], 0, SEEK_SET);

			m_vfsFileOffsets[i]->Begin();

			while (!m_vfsFileOffsets[i]->IterEnd())
			{
				UINT64 cid = m_vfsFileOffsets[i]->CurrentCID();
				if (cid)
				{
					UINT32 key = m_vfsFileOffsets[i]->CurrentKey();
					fwrite(&key, 1, 4, m_vfsBlocks[i]);
					fwrite(&cid, 1, 8, m_vfsBlocks[i]);
				}
				m_vfsFileOffsets[i]->Next();
			}

			fflush(m_vfsBlocks[i]);
			fclose(m_vfsBlocks[i]);

			delete m_vfsFileOffsets[i];
		}
	}
}

void d912pxy_vfs::SetRoot(const char * rootPath)
{
	sprintf(m_rootPath, "%s", rootPath);
}

void* d912pxy_vfs::LoadVFS(UINT id, const char * name)
{
	char fn[4096];

	sprintf(fn, "%s/%s.pck", m_rootPath, name);

	m_vfsBlocks[id] = fopen(fn, "rb+");

	if (m_vfsBlocks[id] == NULL)
		m_vfsBlocks[id] = fopen(fn, "wb+");

	if (m_vfsBlocks[id] == NULL)
	{
		return NULL;
	}

	fseek(m_vfsBlocks[id], 0, SEEK_END);
		
	UINT sz = ftell(m_vfsBlocks[id]);

	fseek(m_vfsBlocks[id], 0, SEEK_SET);

	m_vfsFileOffsets[id] = new d912pxy_memtree(4, PXY_VFS_MAX_FILES_PER_BID, 2, 0);

	m_vfsLastFileOffset[id] = PXY_VFS_BID_TABLE_SIZE;
	
	ZeroMemory(s_headerTable, PXY_VFS_BID_TABLE_SIZE);

	if (sz < PXY_VFS_BID_TABLE_SIZE)
	{												
		fwrite(s_headerTable, PXY_VFS_FILE_HEADER_SIZE, PXY_VFS_MAX_FILES_PER_BID, m_vfsBlocks[id]);
		fwrite(&sz, 1, 4, m_vfsBlocks[id]);

		fflush(m_vfsBlocks[id]);
	}
	else {

		fread(s_headerTable, PXY_VFS_FILE_HEADER_SIZE, PXY_VFS_MAX_FILES_PER_BID, m_vfsBlocks[id]);
		
		for (int i = 0; i != PXY_VFS_MAX_FILES_PER_BID; ++i)
		{
			if (s_headerTable[i].offset > m_vfsLastFileOffset[id])			
				m_vfsLastFileOffset[id] = s_headerTable[i].offset;

			if (s_headerTable[i].hash != 0)
			{
				m_vfsFileOffsets[id]->PointAtNH(s_headerTable[i].hash);
				m_vfsFileOffsets[id]->SetValue(s_headerTable[i].offset);
			}
		}

		if (m_vfsLastFileOffset[id] > 0)
		{

			fseek(m_vfsBlocks[id], (UINT32)m_vfsLastFileOffset[id], SEEK_SET);

			UINT32 lastFileSize;

			fread(&lastFileSize, 4, 1, m_vfsBlocks[id]);

			m_vfsLastFileOffset[id] += lastFileSize + 4;
		}
		else
			m_vfsLastFileOffset[id] = PXY_VFS_BID_TABLE_SIZE;

		if (m_vfsLastFileOffset[id] == sz)
		{
			fseek(m_vfsBlocks[id], 0, SEEK_END);
			fwrite(&sz, 1, 4, m_vfsBlocks[id]);
			fflush(m_vfsBlocks[id]);
		}
		
		m_vfsCacheSize[id] = (UINT32)m_vfsLastFileOffset[id] - PXY_VFS_BID_TABLE_SIZE;

		if (m_vfsCacheSize[id])
		{
			m_vfsCache[id] = malloc(m_vfsCacheSize[id]);

			fseek(m_vfsBlocks[id], PXY_VFS_BID_TABLE_SIZE, SEEK_SET);

			fread(m_vfsCache[id], 1, m_vfsCacheSize[id], m_vfsBlocks[id]);
		}
		else {
			m_vfsCache[id] = 0;
		}
	}	

	return m_vfsBlocks[id];
}

UINT64 d912pxy_vfs::IsPresent(char * fnpath, UINT32 vfsId)
{
	UINT32 fnHash = d912pxy_memtree2::memHash32s(fnpath, (UINT32)strlen(fnpath));

	int i = vfsId;
	{
		if (m_vfsBlocks[i] != NULL)
		{
			m_vfsFileOffsets[i]->PointAtNH(fnHash);
			UINT64 offset = m_vfsFileOffsets[i]->CurrentCID();

			if (offset)
			{			
				return offset;
			}

		}
	}

	return 0;
}

void * d912pxy_vfs::LoadFile(char * fnpath, UINT * sz, UINT id)
{	

	EnterCriticalSection(&lock);

	UINT64 offset = IsPresent(fnpath, id);

	if (!offset)
	{
		LeaveCriticalSection(&lock);
		return nullptr;
	}

	if (m_vfsCache[id] && ((offset - PXY_VFS_BID_TABLE_SIZE) < m_vfsCacheSize[id]))
	{
		offset -= PXY_VFS_BID_TABLE_SIZE;

		LeaveCriticalSection(&lock);

		*sz = *((UINT32*)((intptr_t)m_vfsCache[id] + offset));

		void* ret = malloc(*sz);

		memcpy(ret, ((void*)((intptr_t)m_vfsCache[id] + offset + 4)), *sz);

		return ret;
	}

	fseek(m_vfsBlocks[id], (UINT32)offset, SEEK_SET);

	fread(sz, 4, 1, m_vfsBlocks[id]);

	void* ret = malloc(*sz);

	fread(ret, 1, *sz, m_vfsBlocks[id]);

	LeaveCriticalSection(&lock);

	return ret;
}

void d912pxy_vfs::WriteFile(char * fnpath, void * data, UINT sz, UINT id)
{	
	EnterCriticalSection(&lock);

	fseek(m_vfsBlocks[id], (UINT32)m_vfsLastFileOffset[id], SEEK_SET);
	
	fwrite(&sz, 1, 4, m_vfsBlocks[id]);
	fwrite(data, 1, sz, m_vfsBlocks[id]);
	fwrite(&sz, 1, 4, m_vfsBlocks[id]);
	fflush(m_vfsBlocks[id]);
	
	UINT32 fnHash = d912pxy_memtree2::memHash32s(fnpath, (UINT32)strlen(fnpath));

	m_vfsFileOffsets[id]->PointAtNH(fnHash);
	m_vfsFileOffsets[id]->SetValue(m_vfsLastFileOffset[id]);

	m_vfsLastFileOffset[id] += sz + 4;

	LeaveCriticalSection(&lock);
}

void d912pxy_vfs::ReWriteFile(char * fnpath, void * data, UINT sz, UINT id)
{
	EnterCriticalSection(&lock);

	UINT64 offset = IsPresent(fnpath, id);

	if (offset)
	{
		fseek(m_vfsBlocks[id], (UINT32)offset+4, SEEK_SET);		
		fwrite(data, 1, sz, m_vfsBlocks[id]);
		fflush(m_vfsBlocks[id]);

		if (m_vfsCache[id] && (m_vfsCacheSize[id] > (offset - PXY_VFS_BID_TABLE_SIZE)))
			memcpy((void*)((intptr_t)m_vfsCache[id] + (offset - PXY_VFS_BID_TABLE_SIZE) + 4), data, sz);
	}
	else
		WriteFile(fnpath, data, sz, id);

	LeaveCriticalSection(&lock);
}
