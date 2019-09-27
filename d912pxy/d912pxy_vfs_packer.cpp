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

d912pxy_vfs_packer::d912pxy_vfs_packer()
{
	
}

d912pxy_vfs_packer::~d912pxy_vfs_packer()
{
}

void d912pxy_vfs_packer::UnpackArchive(const wchar_t * name)
{
	FILE* f = _wfopen(name, L"rb");

	if (!f)
		return;

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

	//todo

	PXY_FREE(rawData);
}

void d912pxy_vfs_packer::PackArchive(const wchar_t * name)
{
	FILE* of = _wfopen(name, L"wb");

	StreamInit(VFS_PCK_STREAM_LISTING, 1 << 20);
	StreamInit(VFS_PCK_STREAM_DATA, 1 << 20);

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