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

#define PXY_VFS_PCK_SIGNATURE 0x7670636b
#define PXY_VFS_PCK_INDEX_ROOM 512
#define PXY_VFS_PCK_VER 3

typedef enum d912pxy_vfs_pck_chunk_type {
	CHU_UNDEF,
	CHU_HEADER,	
	CHU_CHUNK_INDEX,
	CHU_FILE_INFO,
	CHU_FILE_DATA
} d912pxy_vfs_pck_chunk_type;

class d912pxy_vfs_pck;

#pragma pack(push, 1)

typedef struct d912pxy_vfs_pck_chunk_index_data {
	UINT32 id;
	UINT64 packedInfo;
} d912pxy_vfs_pck_chunk_index_data;

typedef struct d912pxy_vfs_pck_chunk_dsc {	
	UINT32 checksum;
	UINT32 signature;
	UINT8 type;
	UINT32 id;	
	UINT32 size;
} d912pxy_vfs_pck_chunk_dsc;

typedef struct d912pxy_vfs_pck_chunk_header_data {
	UINT32 ver;
	UINT64 fileSize;
	UINT64 freeSpace;
	UINT32 maxId;
	UINT8 compressed;
} d912pxy_vfs_pck_chunk_header_data;

typedef struct d912pxy_vfs_pck_chunk_chunk_index {
	UINT16 usedIndexes;
	d912pxy_vfs_pck_chunk_index_data data[PXY_VFS_PCK_INDEX_ROOM];
	UINT64 nextIndexChunk;
} d912pxy_vfs_pck_chunk_chunk_index;

typedef struct d912pxy_vfs_pck_chunk_file_info {
	UINT8 category;
	UINT64 name;
	UINT32 dataChunk;
	UINT32 dataCheckSum;	
} d912pxy_vfs_pck_chunk_file_info;

typedef struct d912pxy_vfs_pck_chunk {
	d912pxy_vfs_pck* parent;
	d912pxy_vfs_pck_chunk_dsc dsc;
	union {
		d912pxy_vfs_pck_chunk_header_data header;
		d912pxy_vfs_pck_chunk_chunk_index chunk_index;
		d912pxy_vfs_pck_chunk_file_info file_info;		
		UINT64 rawData;
	} data;	
} d912pxy_vfs_pck_chunk;

#pragma pack(pop)

#define PXY_VFS_PCK_CHUNK_DSC_SIZE sizeof(d912pxy_vfs_pck_chunk_dsc)
#define PXY_VFS_PCK_CHUNK_DATA_SIZE_HEADER sizeof(d912pxy_vfs_pck_chunk_header_data)
#define PXY_VFS_PCK_CHUNK_DATA_SIZE_INDEX sizeof(d912pxy_vfs_pck_chunk_chunk_index)
#define PXY_VFS_PCK_CHUNK_DATA_SIZE_FILE_INFO sizeof(d912pxy_vfs_pck_chunk_file_info)
#define PXY_VFS_PCK_CHUNK_DATA_SIZE_FILE_INFO_EX sizeof(d912pxy_vfs_pck_chunk_file_info_ex)

static UINT d912pxy_vfs_pck_on_disk_chunk_sizes[] = {
	0,//undef
	PXY_VFS_PCK_CHUNK_DSC_SIZE + PXY_VFS_PCK_CHUNK_DATA_SIZE_HEADER,//header
	PXY_VFS_PCK_CHUNK_DSC_SIZE + PXY_VFS_PCK_CHUNK_DATA_SIZE_INDEX,//index
	PXY_VFS_PCK_CHUNK_DSC_SIZE + PXY_VFS_PCK_CHUNK_DATA_SIZE_FILE_INFO,//file info
	PXY_VFS_PCK_CHUNK_DSC_SIZE//file data
};

static UINT d912pxy_vfs_pck_in_mem_chunk_sizes[] = {
	0,//undef
	PXY_VFS_PCK_CHUNK_DSC_SIZE + PXY_VFS_PCK_CHUNK_DATA_SIZE_HEADER + 8,//header
	PXY_VFS_PCK_CHUNK_DSC_SIZE + PXY_VFS_PCK_CHUNK_DATA_SIZE_INDEX + 8,//index
	PXY_VFS_PCK_CHUNK_DSC_SIZE + PXY_VFS_PCK_CHUNK_DATA_SIZE_FILE_INFO + 8,//file info
	PXY_VFS_PCK_CHUNK_DSC_SIZE + 8//file data
};

class d912pxy_vfs_pck : public d912pxy_noncom
{
public:
	d912pxy_vfs_pck(wchar_t* fn, UINT in_allowWrite);
	~d912pxy_vfs_pck(); 

	UINT GetStatus();

	UINT Close(UINT compress);

	d912pxy_vfs_pck_chunk* WriteFileToPck(UINT8 cat, UINT64 name, UINT32 size, void* data);
	d912pxy_vfs_pck_chunk* ReadFileFromPck(d912pxy_vfs_pck_chunk* file);
	UINT UpdateChunk(d912pxy_vfs_pck_chunk* chunk);

	d912pxy_ringbuffer<d912pxy_vfs_pck_chunk*>* GetFileList();

	UINT Cleanup();

	void ModRef(INT dlt);
		   
private:
	UINT32 CalcCRC32(intptr_t data, UINT32 length);

	UINT LoadChunkIndex();
	UINT AddToChunkIndex(d912pxy_vfs_pck_chunk* chunk, UINT64 offset);
	void AddToChunkList(d912pxy_vfs_pck_chunk_index_data* indexDt);
	d912pxy_vfs_pck_chunk_index_data* NewChunkIndexElement(d912pxy_vfs_pck_chunk* chunk, UINT64 offset);
	
	UINT64 PackChunkIndexInfo(UINT8 type, UINT64 offset);
	UINT8 GetChunkTypeFromIndex(d912pxy_vfs_pck_chunk* chunk);
	UINT64 GetChunkOffsetFromIndex(d912pxy_vfs_pck_chunk* chunk);
	UINT64 GetChunkOffsetFromIndexById(UINT32 id);

	UINT8 UnpackChunkTypeFromPackedData(UINT64 packedData);
	UINT64 UnpackChunkOffsetFromPackedData(UINT64 packedData);
	
	d912pxy_vfs_pck_chunk* AllocateChunk(d912pxy_vfs_pck_chunk_type type);
	d912pxy_vfs_pck_chunk* AllocateDataChunk(d912pxy_vfs_pck_chunk_type type, UINT32 size, UINT newChunk);
	void FreeChunk(d912pxy_vfs_pck_chunk* chunk);
	
	void UpdateChunkChecksum(d912pxy_vfs_pck_chunk* chunk);
	
	UINT WriteChunk(d912pxy_vfs_pck_chunk* chunk);
	d912pxy_vfs_pck_chunk* ReadChunk(UINT64 offset);

	d912pxy_vfs_pck_chunk* ReadHeader();
	
	UINT CreateNewPckFile(wchar_t* fn);

	UINT FS_CreateNew(wchar_t* fn);
	UINT FS_Open(wchar_t* fn);
	UINT FS_Close();
	UINT FS_IO(UINT64 offset, UINT rw, UINT32 size, void* buf);	
	UINT64 FS_Size();


	////
	d912pxy_vfs_pck_chunk* cuHeader;
	d912pxy_vfs_pck_chunk* cuChunkIndex;	
	d912pxy::Memtree<uint32_t, uint64_t, d912pxy::RawHash<uint32_t>>* cuChunkList;

	HANDLE fs_file;
	UINT64 fs_write_offset;
	UINT fs_write_allowed;

	UINT cuStatus;
	UINT refs;
};

