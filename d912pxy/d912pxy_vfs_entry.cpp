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

d912pxy_vfs_entry::d912pxy_vfs_entry(UINT id) 
	: d912pxy_noncom(L"vfs_entry")
	, chunkTree(new ChunkTree())
	, lastFind(nullptr)
	, m_Id(id)
{
}

d912pxy_vfs_entry::~d912pxy_vfs_entry()
{
	for (auto i = chunkTree->begin(); i < chunkTree->end(); ++i)
	{
		d912pxy_vfs_pck_chunk* dtCh = i.value();
		if (dtCh)
		{
			if (dtCh->dsc.type == CHU_FILE_INFO)
				dtCh->parent->ModRef(-1);
			PXY_FREE(dtCh);
		}
	}

	delete chunkTree;
}

d912pxy_vfs_pck_chunk* d912pxy_vfs_entry::IsPresentH(UINT64 fnHash)
{
	lastFind = &chunkTree->findPrepared(ChunkTree::PreparedKey::fromRawData(fnHash));

	return *lastFind;
}

void * d912pxy_vfs_entry::GetFileDataH(UINT64 namehash, UINT64 * sz)
{
	d912pxy_vfs_pck_chunk* dtCh = (d912pxy_vfs_pck_chunk*)IsPresentH(namehash);

	if (!dtCh)
		return 0;

	//megai2: file is not precached
	if (dtCh->dsc.type == CHU_FILE_INFO)
	{
		LoadFileFromDisk(dtCh);
		dtCh = *lastFind;

		if (!dtCh)
			return 0;
	}

	*sz = dtCh->dsc.size - PXY_VFS_PCK_CHUNK_DSC_SIZE;

	return &dtCh->data.rawData;
}

void d912pxy_vfs_entry::WriteFileH(UINT64 namehash, void * data, UINT64 sz)
{
	d912pxy_vfs_pck_chunk* dtCh = (d912pxy_vfs_pck_chunk*)IsPresentH(namehash);

	dtCh = d912pxy_s.vfs.WriteFileToPck(dtCh, m_Id, namehash, data, (UINT)sz);

	*lastFind = dtCh;
}

void d912pxy_vfs_entry::ReWriteFileH(UINT64 namehash, void * data, UINT64 sz)
{
	WriteFileH(namehash, data, sz);
}

void d912pxy_vfs_entry::AddFileInfo(d912pxy_vfs_pck_chunk * fileInfo)
{
	d912pxy_vfs_pck_chunk* fiCh = IsPresentH(fileInfo->data.file_info.name);
	
	if (fiCh)
	{
		fiCh->parent->ModRef(-1);
		PXY_FREE(fiCh);
	}

	fileInfo->parent->ModRef(1);

	*lastFind = fileInfo;
}

void d912pxy_vfs_entry::LoadFileFromDisk(d912pxy_vfs_pck_chunk * fiCh)
{
	*lastFind = nullptr;

	if (fiCh)
	{		
		d912pxy_vfs_pck_chunk* dtCh = fiCh->parent->ReadFileFromPck(fiCh);

		if (dtCh)
		{
			if (dtCh->dsc.type != CHU_FILE_DATA)
			{
				LOG_ERR_DTDM("VFS %u / %016llX file info points to non data chunk", fiCh->data.file_info.category, fiCh->data.file_info.name);
				PXY_FREE(dtCh);
			}
			else
				*lastFind = dtCh;
		}

		fiCh->parent->ModRef(-1);
		PXY_FREE(fiCh);
	} 	
}

void d912pxy_vfs_entry::LoadFilesFromDisk()
{
	UINT32 filesLoaded = 0;

	for (auto i = chunkTree->begin(); i < chunkTree->end(); ++i)
	{
		lastFind = &i.value();
		LoadFileFromDisk(i.value());

		if (*lastFind != 0)
			++filesLoaded;

	}

	LOG_INFO_DTDM("Loaded %u files in %s ", filesLoaded, d912pxy_vfs_entry_name_str[m_Id]);
}
