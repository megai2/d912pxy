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
#include "../thirdparty/fastlz/fastlz.h"

d912pxy_vfs_packer::d912pxy_vfs_packer(wchar_t * rootPath, d912pxy_vfs_id_name * id)
{
	sprintf(m_rootPath, "%s/%ws", d912pxy_helper::GetFilePath(FP_VFS_PREFIX)->s, rootPath);
	items = id;
}

d912pxy_vfs_packer::~d912pxy_vfs_packer()
{
}

bool d912pxy_vfs_packer::IsUnpackNeeded()
{
	int i = 0;
	
	while (items[i].name != 0)
	{
		char fn[4096];

		sprintf(fn, "%s/%s.pck", m_rootPath, items[i].name);
		
		if (d912pxy_helper::IsFileExist(fn))
			return false;

		++i;
	}

	return true;
}

void d912pxy_vfs_packer::UnpackArchive(const char * name)
{
	char fn[4096];
	sprintf(fn, "%s/%s.zpck", m_rootPath, name);

	FILE* f = fopen(fn, "rb");

	UINT32 header[3] = { 0 };

	fread(header, 1, 4 * 3, f);

	void* zipData = NULL;
	void* rawData = NULL;

	PXY_MALLOC(zipData, header[0], void*);
	PXY_MALLOC(rawData, header[1], void*);

	fread(zipData, 1, header[0], f);

	if (fastlz_decompress(zipData, header[0], rawData, header[1]) != header[1])
		return;

	PXY_FREE(zipData);

	intptr_t listingPtr = (intptr_t)rawData;
	intptr_t dataPtrBase = (intptr_t)rawData + header[2];
	intptr_t dataPtr = dataPtrBase;
	
	while (listingPtr < dataPtrBase)
	{
		UINT32* listingInfo = (UINT32*)listingPtr;

		listingPtr += 8;

		char fn_vfs[4096];
		sprintf(fn_vfs, "%s/%s.pck", m_rootPath, items[listingInfo[0]].name);

		FILE* vfs_f = fopen(fn_vfs, "wb+");

		UINT64 signature[2] = { PXY_VFS_SIGNATURE, PXY_VFS_VER };

		d912pxy_vfs_file_header* headerTable = NULL;
		PXY_MALLOC(headerTable, PXY_VFS_BID_TABLE_SIZE, d912pxy_vfs_file_header*);
		ZeroMemory(headerTable, PXY_VFS_BID_TABLE_SIZE);

		fseek(vfs_f, 0, SEEK_SET);
		fwrite(signature, 1, 16, vfs_f);
		fwrite(headerTable, PXY_VFS_FILE_HEADER_SIZE, PXY_VFS_MAX_FILES_PER_BID, vfs_f);

		int fileNum = 0;
		int offset = PXY_VFS_BID_TABLE_SIZE + PXY_VFS_BID_TABLE_START;
	
		while (listingInfo[1] != 0)
		{			
			headerTable[fileNum].hash = *(UINT64*)(listingPtr);
			listingPtr += 8;
			headerTable[fileNum].offset = offset;
			
			UINT fileSize = *(UINT32*)(listingPtr);

			fwrite((void*)listingPtr, 1, 4, vfs_f);
			fwrite((void*)dataPtr, 1, fileSize, vfs_f);
			
			dataPtr += fileSize;
			listingPtr += 4;

			offset += fileSize + 4;
			++fileNum;
			--listingInfo[1];
		}		

		fseek(vfs_f, 0, SEEK_SET);
		fwrite(signature, 1, 16, vfs_f);
		fwrite(headerTable, PXY_VFS_FILE_HEADER_SIZE, PXY_VFS_MAX_FILES_PER_BID, vfs_f);

		fclose(vfs_f);

		PXY_FREE(headerTable);
	}

	PXY_FREE(rawData);
}

void d912pxy_vfs_packer::PackArchive(const char * name)
{
	char fn[4096];
	sprintf(fn, "%s/%s.zpck", m_rootPath, name);

	FILE* of = fopen(fn, "wb");

	StreamInit(VFS_PCK_STREAM_LISTING, 1 << 20);
	StreamInit(VFS_PCK_STREAM_DATA, 1 << 20);

	int i = 0;

	while (items[i].name != 0)
	{
		ReadVFS(&items[i]);
		++i;
	}

	UINT sz, oSz;
	
	void* retMem = PackStreams(&sz, &oSz);
	
	fwrite(&sz, 1, 4, of);
	fwrite(&oSz, 1, 4, of);
	fwrite(&streams[VFS_PCK_STREAM_LISTING].offset, 1, 4, of);
	fwrite(retMem, 1, sz, of);

	fclose(of);

	PXY_FREE(retMem);
}

void d912pxy_vfs_packer::StreamInit(d912pxy_vfs_packer_stream_id id, UINT warmupSz)
{
	d912pxy_vfs_packer_stream* stream = &streams[id];

	PXY_MALLOC(stream->buf, warmupSz, intptr_t);

	stream->offset = 0;
	stream->sz = warmupSz;
}

void d912pxy_vfs_packer::StreamWrite(d912pxy_vfs_packer_stream_id id, void * mem, UINT sz)
{
	d912pxy_vfs_packer_stream* stream = &streams[id];

	UINT newOffset = sz + stream->offset;

	StreamReAlloc(stream, newOffset);

	memcpy((void*)(stream->buf + stream->offset), mem, sz);

	stream->offset = newOffset;
}

void d912pxy_vfs_packer::StreamWriteFrom(d912pxy_vfs_packer_stream_id id, FILE * f, UINT sz)
{
	d912pxy_vfs_packer_stream* stream = &streams[id];

	UINT newOffset = sz + stream->offset;

	StreamReAlloc(stream, newOffset);

	fread((void*)(stream->buf + stream->offset), 1, sz, f);

	stream->offset = newOffset;
}

void d912pxy_vfs_packer::StreamReAlloc(d912pxy_vfs_packer_stream * stream, UINT newOffset)
{
	if (newOffset >= stream->sz)
	{
		UINT32 nsz = (newOffset + (1 << 20) * 10);
		PXY_REALLOC(stream->buf, nsz, intptr_t);
		stream->sz = nsz;
	}
}

void* d912pxy_vfs_packer::PackStreams(UINT * sz, UINT* oSz)
{
	UINT totalSz = 0;

	for (int i = 0; i != VFS_PCK_STREAM_CNT; ++i)
	{
		totalSz += streams[i].offset;
	}

	void* rawData = NULL;
	void* zipData = NULL;

	*sz = (UINT32)(((UINT64)totalSz * 120ULL) / 100ULL);

	PXY_MALLOC(rawData, totalSz, void*);
	PXY_MALLOC(zipData, *sz, void*);

	UINT wPtr = 0;

	for (int i = 0; i != VFS_PCK_STREAM_CNT; ++i)
	{
		memcpy((void*)((intptr_t)rawData + wPtr), (void*)streams[i].buf, streams[i].offset);
		wPtr += streams[i].offset;
		PXY_FREE(streams[i].buf);
	}

	*oSz = totalSz;
	*sz = fastlz_compress(rawData, totalSz, zipData);

	PXY_FREE(rawData);
	
	return zipData;
}

void d912pxy_vfs_packer::ReadVFS(d912pxy_vfs_id_name * id)
{
	char fn[4096];

	sprintf(fn, "%s/%s.pck", m_rootPath, id->name);

	FILE* f = fopen(fn, "rb+");

	if (f == NULL)
		return;
	
	fseek(f, 0, SEEK_END);
	UINT sz = ftell(f);
	fseek(f, 0, SEEK_SET);
		
	UINT64 signature[2] = { PXY_VFS_SIGNATURE, PXY_VFS_VER };

	if (sz < PXY_VFS_BID_TABLE_SIZE + PXY_VFS_BID_TABLE_START)
	{
		fclose(f);
		return;		
	}
	else {

		UINT64 readedSignature[2] = { 0,0 };

		if (fread(readedSignature, 8, 2, f) != 2)
		{
			fclose(f);
			return;
		}

		if (memcmp(signature, readedSignature, 16))
		{
			fclose(f);
			return;
		}

		fseek(f, 16, SEEK_SET);

		d912pxy_vfs_file_header* headerTable = NULL;
		PXY_MALLOC(headerTable, PXY_VFS_BID_TABLE_SIZE, d912pxy_vfs_file_header*);
		
		fread(headerTable, PXY_VFS_FILE_HEADER_SIZE, PXY_VFS_MAX_FILES_PER_BID, f);

		d912pxy_memtree2* files = new d912pxy_memtree2(8, 256, 2);

		UINT32 haveFiles = 0;

		for (int i = 0; i != PXY_VFS_MAX_FILES_PER_BID; ++i)
		{
			if (headerTable[i].hash != 0)
			{
				files->PointAtMem(&headerTable[i].hash, 8);
				files->SetValue(i+1);				
				++haveFiles;
			}
		}

		if (haveFiles)
		{
			StreamWrite(VFS_PCK_STREAM_LISTING, &id->num, 4);

			//megai2: count unique files
			UINT uniqueFiles = haveFiles;

			files->Begin();

			while (!files->IterEnd())
			{
				if (files->CurrentCID() != 0)
					--haveFiles;
				files->Next();
			}

			uniqueFiles -= haveFiles;

			StreamWrite(VFS_PCK_STREAM_LISTING, &uniqueFiles, 4);
									
			files->Begin();

			void* vfsFileData = NULL;
			PXY_MALLOC(vfsFileData, (sz - PXY_VFS_DATA_OFFSET), void*);			
			fread(vfsFileData, 1, sz - PXY_VFS_DATA_OFFSET, f);

			while (!files->IterEnd())
			{
				if (files->CurrentCID() == 0)
				{
					files->Next();
					continue;
				}

				d912pxy_vfs_file_header fDsc = headerTable[files->CurrentCID()-1];
								
				StreamWrite(VFS_PCK_STREAM_LISTING, &fDsc.hash, 8);				
				StreamWrite(VFS_PCK_STREAM_LISTING, (void*)((intptr_t)vfsFileData + fDsc.offset - PXY_VFS_DATA_OFFSET) , 4);

				StreamWrite(VFS_PCK_STREAM_DATA, (void*)((intptr_t)vfsFileData + fDsc.offset - PXY_VFS_DATA_OFFSET + 4), *(UINT32*)((intptr_t)vfsFileData + fDsc.offset - PXY_VFS_DATA_OFFSET));
				

				files->Next();
			}			

			PXY_FREE(vfsFileData);
		}

		PXY_FREE(headerTable);				
		delete files;

		fclose(f);
	}	
}
