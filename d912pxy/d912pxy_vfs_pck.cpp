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

d912pxy_vfs_pck::d912pxy_vfs_pck(wchar_t * fn, UINT in_allowWrite)
{	
	NonCom_Init(L"vfs_pck");

	fs_write_allowed = in_allowWrite;
	refs = 1;

	if (!FS_Open(fn))
	{
		if (fs_write_allowed)
		{
			if (!CreateNewPckFile(fn))
			{
				LOG_ERR_DTDM("Error opening & creating %s file", fn);
				cuStatus = 1;
				return;
			}
		}
		else {
			LOG_ERR_DTDM("Can't open %s for read", fn);
			cuStatus = 1;
			return;
		}
	}

	cuHeader = ReadHeader();

	if (!cuHeader)
	{
		LOG_ERR_DTDM("Error reading %s header", fn);
		FS_Close();
		cuStatus = 1;
		return;
	}

	if (!LoadChunkIndex())
	{
		LOG_ERR_DTDM("Error reading %s chunk index", fn);
		FS_Close();
		cuStatus = 1;
		return;
	}

	//always write new data to file's end
	fs_write_offset = FS_Size();

	cuStatus = 0;

	LOG_INFO_DTDM("Opened %s file as %016llX", fn, this);	
}

d912pxy_vfs_pck::~d912pxy_vfs_pck()
{
	if (cuStatus)
	{
		return;
	}
	else {
		LOG_WARN_DTDM("pck %016llX are not closed!", this);
		Close(0);
	}
}

UINT d912pxy_vfs_pck::GetStatus()
{
	return cuStatus;
}

UINT d912pxy_vfs_pck::Close(UINT compress)
{
	cuHeader->data.header.fileSize = FS_Size();

	if (!UpdateChunk(cuHeader))	
		LOG_ERR_DTDM("Error updating header on pck %016llX", this);		

	FS_Close();

	FreeChunk(cuHeader);
	FreeChunk(cuChunkIndex);
	delete cuChunkList;

	cuStatus = 1;

	if (compress)
	{
		//megai2: TODO
	}

	return 1;
}

d912pxy_vfs_pck_chunk* d912pxy_vfs_pck::WriteFileToPck(UINT8 cat, UINT64 name, UINT32 size, void * data)
{
	d912pxy_vfs_pck_chunk* fiCh = AllocateChunk(CHU_FILE_INFO);
	d912pxy_vfs_pck_chunk* dtCh = AllocateDataChunk(CHU_FILE_DATA, size, 1);

	memcpy(&dtCh->data.rawData, data, size);
	if (!WriteChunk(dtCh))
		return 0;

	fiCh->data.file_info.category = cat;
	fiCh->data.file_info.name = name;	
	fiCh->data.file_info.dataChunk = dtCh->dsc.id;	
	fiCh->data.file_info.dataCheckSum = dtCh->dsc.checksum;

	if (!WriteChunk(fiCh))
		return 0;

	FreeChunk(fiCh);
	
	return dtCh;
}

d912pxy_vfs_pck_chunk* d912pxy_vfs_pck::ReadFileFromPck(d912pxy_vfs_pck_chunk * file)
{	
	d912pxy_vfs_pck_chunk* dtCh = ReadChunk(GetChunkOffsetFromIndexById(file->data.file_info.dataChunk));

	if (!dtCh)
		return 0;

	if (dtCh->dsc.checksum != file->data.file_info.dataCheckSum)
	{				
		LOG_DBG_DTDM3("Data of file %u / %016llX in %016llX are corrupted or rewrited (checksum %08lX != %08lX)", file->data.file_info.category, file->data.file_info.name, this, dtCh->dsc.checksum, file->data.file_info.dataCheckSum);				
	}
	
	return dtCh;
}

d912pxy_ringbuffer<d912pxy_vfs_pck_chunk*>* d912pxy_vfs_pck::GetFileList()
{
	d912pxy_ringbuffer<d912pxy_vfs_pck_chunk*>* ret = new d912pxy_ringbuffer<d912pxy_vfs_pck_chunk*>(512, 2);

	for (auto i = cuChunkList->begin(); i < cuChunkList->end(); ++i)
	{
		UINT64 packedData = i.value();

		if (UnpackChunkTypeFromPackedData(packedData) == CHU_FILE_INFO)
		{
			d912pxy_vfs_pck_chunk* fiCh = ReadChunk(UnpackChunkOffsetFromPackedData(packedData));

			if (fiCh)
				ret->WriteElement(fiCh);
		}
	}

	LOG_INFO_DTDM("Found %u files in %016llX", ret->TotalElements(), this);

	return ret;
}

d912pxy_vfs_pck_chunk * d912pxy_vfs_pck::ReadHeader()
{	
	d912pxy_vfs_pck_chunk* ret = ReadChunk(0);

	if (!ret)
		return 0;

	if (ret->data.header.ver != PXY_VFS_PCK_VER)
	{
		FreeChunk(ret);
		return 0;
	}

	UINT64 actualFsz = FS_Size();

	if (ret->data.header.fileSize != actualFsz)
	{
		LOG_WARN_DTDM("%016llX header file size is incorrect, updating from %u to %u", this, ret->data.header.fileSize, actualFsz);
		ret->data.header.fileSize = actualFsz;
	}

	return ret;
}

UINT d912pxy_vfs_pck::CreateNewPckFile(wchar_t * fn)
{
	if (!FS_CreateNew(fn))
		return 0;

	d912pxy_vfs_pck_chunk* headerChunk = AllocateDataChunk(CHU_HEADER, 0, 0);
	d912pxy_vfs_pck_chunk* indexChunk = AllocateDataChunk(CHU_CHUNK_INDEX, 0, 0);

	headerChunk->dsc.id = 0;
	headerChunk->data.header.compressed = 0;
	headerChunk->data.header.fileSize = 0x1845;
	headerChunk->data.header.freeSpace = 0;
	headerChunk->data.header.maxId = 1;
	headerChunk->data.header.ver = PXY_VFS_PCK_VER;

	memset(&indexChunk->data.chunk_index, 0, PXY_VFS_PCK_CHUNK_DATA_SIZE_INDEX);
	indexChunk->dsc.id = 1;
	indexChunk->data.chunk_index.usedIndexes = 2;
	indexChunk->data.chunk_index.nextIndexChunk = 0;

	indexChunk->data.chunk_index.data[0].id = 0;
	indexChunk->data.chunk_index.data[0].packedInfo = (UINT64)CHU_HEADER << 56ULL;
	indexChunk->data.chunk_index.data[1].id = 1;
	indexChunk->data.chunk_index.data[1].packedInfo = ((UINT64)CHU_CHUNK_INDEX << 56ULL) | (headerChunk->dsc.size);

	UpdateChunkChecksum(headerChunk);
	UpdateChunkChecksum(indexChunk);

	FS_IO(0, 1, headerChunk->dsc.size, &headerChunk->dsc);
	FS_IO(headerChunk->dsc.size, 1, indexChunk->dsc.size, &indexChunk->dsc);

	FreeChunk(headerChunk);
	FreeChunk(indexChunk);

	//megai2: close and reopen file to make sure that critical data is saved
	if (!FS_Close())
		return 0;

	if (!FS_Open(fn))
		return 0;

	return 1;
}

UINT d912pxy_vfs_pck::FS_CreateNew(wchar_t * fn)
{
	fs_file = CreateFile(fn, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

	return fs_file != INVALID_HANDLE_VALUE;		
}

UINT d912pxy_vfs_pck::FS_Open(wchar_t * fn)
{
	//megai2: works this way, i'l keep writes OFF in FS_IO
	fs_file = CreateFile(fn, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	return fs_file != INVALID_HANDLE_VALUE;
}

UINT d912pxy_vfs_pck::FS_Close()
{
	if (!FlushFileBuffers(fs_file))
		return 0;

	if (!CloseHandle(fs_file))
		return 0;

	return 1;
}

UINT d912pxy_vfs_pck::FS_IO(UINT64 offset, UINT rw, UINT32 size, void * buf)
{
	if (rw && !fs_write_allowed)
		return 1;

	LARGE_INTEGER liOfs;

	liOfs.QuadPart = offset;

	if (!SetFilePointerEx(fs_file, liOfs, NULL, FILE_BEGIN))
	{
		LOG_ERR_DTDM("SetFilePointerEx failed for %016llX with params %llu %u %u", this, offset, rw, size);
		return 0;
	}
	
	DWORD processedBytes;

	if (rw)
	{
		if (!WriteFile(fs_file, buf, size, &processedBytes, 0))
		{
			LOG_ERR_DTDM("IO failed for %016llX with params %llu %u %u", this, offset, rw, size);
			return 0;
		}
	}
	else {
		if (!ReadFile(fs_file, buf, size, &processedBytes, 0))
		{
			LOG_ERR_DTDM("IO failed for %016llX with params %llu %u %u", this, offset, rw, size);
			return 0;
		}
	}

	if (processedBytes != size)
	{
		LOG_ERR_DTDM("IO count wrong for %016llX with params %llu %u %u", this, offset, rw, size);
		return 0;
	}

	return 1;
}

UINT64 d912pxy_vfs_pck::FS_Size()
{
	LARGE_INTEGER ret;
	if (!GetFileSizeEx(fs_file, &ret))
	{
		LOG_ERR_DTDM("GetFileSize failed for %016llX", this);
		return 0;
	}

	return ret.QuadPart;
}

UINT d912pxy_vfs_pck::Cleanup()
{
	//megai2: TODO scan file for free space / unused chunks and relocate data
	return 0;
}

void d912pxy_vfs_pck::ModRef(INT dlt)
{
	refs += dlt;

	if (!refs)
	{
		Close(d912pxy_s.config.GetValueUI32(PXY_CFG_VFS_PACK_DATA));
		delete this;
	}
}

UINT32 d912pxy_vfs_pck::CalcCRC32(intptr_t data, UINT32 length)
{
	int j;
	unsigned int byte, crc, mask;
	static unsigned int table[256];

	/* Set up the table, if necessary. */

	if (table[1] == 0) {
		for (byte = 0; byte <= 255; byte++) {
			crc = byte;
			for (j = 7; j >= 0; j--) {    // Do eight times.
				mask = -(signed int)(crc & 1);
				crc = (crc >> 1) ^ (0xEDB88320 & mask);
			}
			table[byte] = crc;
		}
	}

	/* Through with table setup, now calculate the CRC. */

	crc = 0xFFFFFFFF;
	intptr_t endpoint = data + length;

	while ((data + 4) < endpoint) {
		crc = crc ^ *(unsigned int *)data;
		crc = (crc >> 8) ^ table[crc & 0xFF];
		crc = (crc >> 8) ^ table[crc & 0xFF];
		crc = (crc >> 8) ^ table[crc & 0xFF];
		crc = (crc >> 8) ^ table[crc & 0xFF];
		data += 4;		
	}

	return ~crc;
}

UINT d912pxy_vfs_pck::LoadChunkIndex()
{
	UINT64 fsz = FS_Size();

	UINT64 indexOffset = d912pxy_vfs_pck_on_disk_chunk_sizes[CHU_HEADER];

	cuChunkList = new d912pxy::Memtree<uint32_t, uint64_t, d912pxy::RawHash<uint32_t>>();
	cuChunkIndex = 0;
	
	d912pxy_vfs_pck_chunk* chunkIndex = ReadChunk(indexOffset);

	UINT totalChunks = 0;
	UINT maxChunkId = 0;

	while (chunkIndex)
	{
		if (chunkIndex->dsc.type != CHU_CHUNK_INDEX)
		{
			LOG_ERR_DTDM("Next chunk index in %016llX pck are corrupted", this);
			delete cuChunkList;
			return 0;
		}

		d912pxy_vfs_pck_chunk_index_data* indexData = chunkIndex->data.chunk_index.data;
		for (UINT32 i = 0; i != chunkIndex->data.chunk_index.usedIndexes; ++i)
		{			
			AddToChunkList(&indexData[i]);

			if (indexData[i].id > maxChunkId)
				maxChunkId = indexData[i].id;
			++totalChunks;				
		}

		if (chunkIndex->data.chunk_index.nextIndexChunk)
		{
			d912pxy_vfs_pck_chunk* oldChunkIdx = chunkIndex;
			chunkIndex = ReadChunk(chunkIndex->data.chunk_index.nextIndexChunk);

			FreeChunk(oldChunkIdx);
		}
		else {
			cuChunkIndex = chunkIndex;
			chunkIndex = 0;
		}
	}
	
	if (!cuChunkIndex)
	{
		LOG_ERR_DTDM("Chunk index in %016llX pck are corrupted", this);
		delete cuChunkList;
		return 0;
	}

	LOG_DBG_DTDM3("Readed %lu chunks from %016llX pck index", totalChunks, this);

	if (cuHeader->data.header.maxId != maxChunkId)
	{
		LOG_WARN_DTDM("Max chunk id of %016llX are corrupted, updating %u to %u from chunk index", this, cuHeader->data.header.maxId, maxChunkId);
		cuHeader->data.header.maxId = maxChunkId;
	}

	return 1;
}

UINT d912pxy_vfs_pck::AddToChunkIndex(d912pxy_vfs_pck_chunk * chunk, UINT64 offset)
{
	if (cuChunkIndex->data.chunk_index.usedIndexes >= PXY_VFS_PCK_INDEX_ROOM)
	{
		cuChunkIndex->data.chunk_index.nextIndexChunk = fs_write_offset;
		UpdateChunk(cuChunkIndex);
		FreeChunk(cuChunkIndex);

		cuChunkIndex = AllocateChunk(CHU_CHUNK_INDEX);

		cuChunkIndex->data.chunk_index.usedIndexes = 0;
		cuChunkIndex->data.chunk_index.nextIndexChunk = 0;
	
		if (!WriteChunk(cuChunkIndex))
			return 0;
	}

	AddToChunkList(NewChunkIndexElement(chunk, offset));
	
	return UpdateChunk(cuChunkIndex);	
}

void d912pxy_vfs_pck::AddToChunkList(d912pxy_vfs_pck_chunk_index_data* indexDt)
{
	cuChunkList->find(indexDt->id) = indexDt->packedInfo;
}

d912pxy_vfs_pck_chunk_index_data* d912pxy_vfs_pck::NewChunkIndexElement(d912pxy_vfs_pck_chunk* chunk, UINT64 offset)
{
	auto& indexInfo = cuChunkIndex->data.chunk_index;

	auto ret = &indexInfo.data[indexInfo.usedIndexes++];

	ret->id = chunk->dsc.id;
	ret->packedInfo = PackChunkIndexInfo(chunk->dsc.type, offset);

	return ret;
}

UINT64 d912pxy_vfs_pck::PackChunkIndexInfo(UINT8 type, UINT64 offset)
{
	return ((UINT64)type << 56ULL) | offset;
}

UINT8 d912pxy_vfs_pck::GetChunkTypeFromIndex(d912pxy_vfs_pck_chunk* chunk)
{
	return UnpackChunkTypeFromPackedData(cuChunkList->find(chunk->dsc.id));
}

UINT64 d912pxy_vfs_pck::GetChunkOffsetFromIndex(d912pxy_vfs_pck_chunk* chunk)
{
	return UnpackChunkOffsetFromPackedData(cuChunkList->find(chunk->dsc.id));
}

UINT64 d912pxy_vfs_pck::GetChunkOffsetFromIndexById(UINT32 id)
{
	return UnpackChunkOffsetFromPackedData(cuChunkList->find(id));
}

UINT8 d912pxy_vfs_pck::UnpackChunkTypeFromPackedData(UINT64 packedData)
{
	return (packedData >> 56ULL) & 0xFF;
}

UINT64 d912pxy_vfs_pck::UnpackChunkOffsetFromPackedData(UINT64 packedData)
{	
	return packedData & 0x00FFFFFFFFFFFFFFULL;
}

d912pxy_vfs_pck_chunk * d912pxy_vfs_pck::AllocateChunk(d912pxy_vfs_pck_chunk_type type)
{
	return AllocateDataChunk(type, 0, 1);
}

d912pxy_vfs_pck_chunk * d912pxy_vfs_pck::AllocateDataChunk(d912pxy_vfs_pck_chunk_type type, UINT32 size, UINT newChunk)
{
	d912pxy_vfs_pck_chunk * ret = 0;
	PXY_MALLOC(ret, d912pxy_vfs_pck_in_mem_chunk_sizes[type]+size, d912pxy_vfs_pck_chunk *);

	ret->parent = this;
	ret->dsc.type = type;
	ret->dsc.signature = PXY_VFS_PCK_SIGNATURE;
	ret->dsc.size = d912pxy_vfs_pck_on_disk_chunk_sizes[type] + size;

	if (newChunk)
		ret->dsc.id = ++cuHeader->data.header.maxId;

	return ret;
}

void d912pxy_vfs_pck::FreeChunk(d912pxy_vfs_pck_chunk * chunk)
{
	PXY_FREE(chunk);
}

void d912pxy_vfs_pck::UpdateChunkChecksum(d912pxy_vfs_pck_chunk * chunk)
{
	chunk->dsc.checksum = CalcCRC32((intptr_t)&chunk->dsc.signature, chunk->dsc.size-4);
}

UINT d912pxy_vfs_pck::UpdateChunk(d912pxy_vfs_pck_chunk * chunk)
{
	UpdateChunkChecksum(chunk);

	return FS_IO(GetChunkOffsetFromIndex(chunk), 1, chunk->dsc.size, &chunk->dsc);
}

UINT d912pxy_vfs_pck::WriteChunk(d912pxy_vfs_pck_chunk * chunk)
{
	UpdateChunkChecksum(chunk);

	UINT64 chunkOffset = fs_write_offset;

	if (!FS_IO(chunkOffset, 1, chunk->dsc.size, &chunk->dsc))
		return 0;

	fs_write_offset += chunk->dsc.size;

	return AddToChunkIndex(chunk, chunkOffset);
}

d912pxy_vfs_pck_chunk * d912pxy_vfs_pck::ReadChunk(UINT64 offset)
{
	UINT8 chunkDscBuffer[PXY_VFS_PCK_CHUNK_DSC_SIZE];
	d912pxy_vfs_pck_chunk_dsc* chunkDsc = (d912pxy_vfs_pck_chunk_dsc*)chunkDscBuffer;
	
	if (!FS_IO(offset, 0, PXY_VFS_PCK_CHUNK_DSC_SIZE, chunkDsc))
		return nullptr;

	if (chunkDsc->signature != PXY_VFS_PCK_SIGNATURE)
	{
		LOG_ERR_DTDM("chunk with no signature at offset %llu in %016llX", offset, this);		
		return nullptr;
	}

	d912pxy_vfs_pck_chunk* ret = AllocateDataChunk((d912pxy_vfs_pck_chunk_type)chunkDsc->type, chunkDsc->size - d912pxy_vfs_pck_on_disk_chunk_sizes[chunkDsc->type], 0);

	memcpy(&ret->dsc, chunkDsc, PXY_VFS_PCK_CHUNK_DSC_SIZE);
	if (!FS_IO(offset + PXY_VFS_PCK_CHUNK_DSC_SIZE, 0, chunkDsc->size - PXY_VFS_PCK_CHUNK_DSC_SIZE, &ret->data.rawData))
	{
		FreeChunk(ret);
		return nullptr;
	}

	UINT32 readedChecksum = ret->dsc.checksum;

	UpdateChunkChecksum(ret);

	if (readedChecksum != ret->dsc.checksum)
	{
		LOG_ERR_DTDM("chunk %u (type %u size %u) of %016llX is corrupted", ret->dsc.id, ret->dsc.type, ret->dsc.size, this);
		FreeChunk(ret);
		return nullptr;
	}
			   
	return ret;
}
