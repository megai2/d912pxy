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

d912pxy_vfs_entry::d912pxy_vfs_entry(UINT id) : d912pxy_noncom(L"vfs_entry")
{
	m_Id = id;
	chunkTree = new d912pxy_memtree2(8, 100, 2);
}

d912pxy_vfs_entry::~d912pxy_vfs_entry()
{
	chunkTree->Begin();

	while (!chunkTree->IterEnd())
	{
		d912pxy_vfs_pck_chunk* dtCh = (d912pxy_vfs_pck_chunk*)chunkTree->CurrentCID();

		if (dtCh)
			PXY_FREE(dtCh);

		chunkTree->Next();
	}

	delete chunkTree;
}

UINT64 d912pxy_vfs_entry::IsPresentH(UINT64 fnHash)
{
	chunkTree->PointAtMem(&fnHash, 8);
	return chunkTree->CurrentCID();
}

void * d912pxy_vfs_entry::GetFileDataH(UINT64 namehash, UINT * sz)
{
	d912pxy_vfs_pck_chunk* dtCh = (d912pxy_vfs_pck_chunk*)IsPresentH(namehash);

	if (!dtCh)
		return 0;

	*sz = dtCh->dsc.size - PXY_VFS_PCK_CHUNK_DSC_SIZE;

	return &dtCh->data.rawData;
}

void d912pxy_vfs_entry::WriteFileH(UINT64 namehash, void * data, UINT sz)
{
	d912pxy_vfs_pck_chunk* dtCh = (d912pxy_vfs_pck_chunk*)IsPresentH(namehash);

	dtCh = d912pxy_s.vfs.WriteFileToPck(dtCh, m_Id, namehash, data, sz);

	chunkTree->SetValue((UINT64)dtCh);
}

void d912pxy_vfs_entry::ReWriteFileH(UINT64 namehash, void * data, UINT sz)
{
	WriteFileH(namehash, data, sz);
}

void d912pxy_vfs_entry::AddFileInfo(d912pxy_vfs_pck_chunk * fileInfo)
{
	d912pxy_vfs_pck_chunk* fiCh = (d912pxy_vfs_pck_chunk*)IsPresentH(fileInfo->data.file_info.name);

	if (fiCh)
		PXY_FREE(fiCh);

	chunkTree->SetValue((UINT64)fileInfo);
}

void d912pxy_vfs_entry::LoadFilesFromDisk()
{
	UINT32 filesLoaded = 0;

	chunkTree->Begin();

	while (!chunkTree->IterEnd())
	{
		d912pxy_vfs_pck_chunk* fiCh = (d912pxy_vfs_pck_chunk*)chunkTree->CurrentCID();

		if (fiCh)
		{
			d912pxy_vfs_pck_chunk* dtCh = fiCh->parent->ReadFileFromPck(fiCh);
			PXY_FREE(fiCh);

			if (dtCh)
				++filesLoaded;

			chunkTree->SetValue((UINT64)dtCh);
		}

		chunkTree->Next();
	}

	LOG_INFO_DTDM("Loaded %u files in %s ", filesLoaded, d912pxy_vfs_entry_name_str[m_Id]);
}
