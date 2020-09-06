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

typedef struct d912pxy_vfs_packer_stream {
	intptr_t buf;
	UINT sz;
	UINT offset;
} d912pxy_vfs_packer_stream;

typedef enum d912pxy_vfs_packer_stream_id {
	VFS_PCK_STREAM_LISTING = 0,
	VFS_PCK_STREAM_DATA = 1,
	VFS_PCK_STREAM_CNT = 2
} d912pxy_vfs_packer_stream_id;

class d912pxy_vfs_packer {

public:
	d912pxy_vfs_packer();
	~d912pxy_vfs_packer();

	void UnpackArchive(const wchar_t* name);
	void PackArchive(const wchar_t* name);

private:
	void StreamInit(d912pxy_vfs_packer_stream_id id, UINT warmupSz);
	void StreamWrite(d912pxy_vfs_packer_stream_id id, void* mem, UINT sz);
	void StreamWriteFrom(d912pxy_vfs_packer_stream_id id, FILE* f, UINT sz);

	void StreamReAlloc(d912pxy_vfs_packer_stream* stream, UINT newOffset);

	void* PackStreams(UINT* sz, UINT* oSz);

	void ReadVFS();
		
	d912pxy_vfs_packer_stream streams[VFS_PCK_STREAM_CNT] = {};
};
