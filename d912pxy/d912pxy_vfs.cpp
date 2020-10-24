/*
MIT License

Copyright(c) 2018-2020 megai2

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

d912pxy_thread_lock d912pxy_vfs_locked_entry::itemLocks[PXY_VFS_MAX_BID];

d912pxy_vfs::d912pxy_vfs()
{	
}


d912pxy_vfs::~d912pxy_vfs()
{	
}

void d912pxy_vfs::Init(const char * lockPath)
{	
	NonCom_Init(L"vfs");

	lockFile = CreateFileA(lockPath, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_DELETE_ON_CLOSE, NULL);
	if (lockFile == INVALID_HANDLE_VALUE)
	{
		writeAllowed = 0;
	}
	else {
		DWORD pid = GetProcessId(GetCurrentProcess());
		DWORD ret = 0;
		::WriteFile(lockFile, &pid, 4, &ret, NULL);
		writeAllowed = 1;
	}

	if (cuWriteMask == ((1 << (UINT)d912pxy_vfs_bid::end) - 1))
	{
		TakeOutWriteAccess();
	}
	
	for (int i = 0; i != (UINT)d912pxy_vfs_bid::end; ++i)
		items[i] = new d912pxy_vfs_entry(i);
}

void d912pxy_vfs::UnInit()
{
	for (int i = 0; i != (UINT)d912pxy_vfs_bid::end; ++i)
		delete items[i];

	if (writeAllowed)
	{
		CloseHandle(lockFile);
	}

	cuPck->Close(0);
	delete cuPck;

	d912pxy_noncom::UnInit();
}

void d912pxy_vfs::SetRoot(wchar_t * rootPath)
{
	sprintf(m_rootPath, "%s/%ws", d912pxy_helper::GetFilePath(FP_VFS_PREFIX)->s, rootPath);
}

void d912pxy_vfs::LoadVFS()
{
	LoadPckFromRootPath();

	if (!cuPck)
	{
		wchar_t newPck[4096];
		wsprintf(newPck, L"%S/%s", m_rootPath, PXY_VFS_LATEST_PCK);

		//try to remove old file before creating if it somehow corrupted
		DeleteFile(newPck);

		cuPck = new d912pxy_vfs_pck(newPck, writeAllowed);
		if (cuPck->GetStatus())
		{
			LOG_ERR_DTDM("Latest pck file is broken, nothing will be saved");

			TakeOutWriteAccess();
		}		
	}	
}

bool d912pxy_vfs::ReadFile(d912pxy_vfs_path path, d912pxy_mem_block to)
{
	UINT64 sz;
	void* ptr = GetBidLocked(path)->GetFileDataH(path.pathHash(), &sz);

	if (ptr == nullptr)
		return false;
	
	if (to.size() != sz)
		return false;

	memcpy(to.ptr(), ptr, sz);

	return true;
}

d912pxy_mem_block d912pxy_vfs::ReadFile(d912pxy_vfs_path path)
{
	UINT64 sz;
	void* ptr = GetBidLocked(path)->GetFileDataH(path.pathHash(), &sz);
	
	return d912pxy_mem_block::from(ptr, sz);
}

void d912pxy_vfs::WriteFile(d912pxy_vfs_path path, d912pxy_mem_block data)
{
	if (((1 << path.bidIndex()) & cuWriteMask) || (!writeAllowed))
		return;

	GetBidLocked(path)->WriteFileH(path.pathHash(), data.ptr(), data.size());
}

d912pxy_ringbuffer<d912pxy_vfs_path_hash>* d912pxy_vfs::GetFileList(d912pxy_vfs_bid bid)
{
	d912pxy_ringbuffer<d912pxy_vfs_path_hash>* ret = new d912pxy_ringbuffer<d912pxy_vfs_path_hash>(20, 2);

	d912pxy_vfs_entry::ChunkTree* cuTree = GetBidLocked(bid)->GetChunkTree();

	for (auto i = cuTree->begin(); i < cuTree->end(); ++i)
	{
		d912pxy_vfs_pck_chunk* chunk = i.value();

		if (chunk && (chunk->dsc.type == CHU_FILE_INFO))
			ret->WriteElement(chunk->data.file_info.name);
	}

	return ret;
}

bool d912pxy_vfs::IsFilePresent(d912pxy_vfs_path path)
{
	return GetBidLocked(path)->IsPresentH(path.pathHash());
}

d912pxy_vfs_locked_entry d912pxy_vfs::GetBidLocked(d912pxy_vfs_path path)
{
	return d912pxy_vfs_locked_entry(path.bid(), items);	
}

d912pxy_vfs_locked_entry d912pxy_vfs::GetBidLocked(d912pxy_vfs_bid bid)
{
	return d912pxy_vfs_locked_entry(bid, items);
}

d912pxy_vfs_pck_chunk * d912pxy_vfs::WriteFileToPck(d912pxy_vfs_pck_chunk* prevChunk, UINT id, UINT64 namehash, void * data, UINT sz)
{
	if (prevChunk)
	{
		if (prevChunk->dsc.type == CHU_FILE_INFO)
		{
			if (prevChunk->parent == cuPck)
			{
				items[id]->LoadFileFromDisk(prevChunk);
				prevChunk = (d912pxy_vfs_pck_chunk*)items[id]->GetLastChunk();

				return WriteFileToPck(prevChunk, id, namehash, data, sz);
			}
			else {
				prevChunk->parent->ModRef(-1);							
			}
		}
		else {
			if ((prevChunk->parent == cuPck) && (sz == (prevChunk->dsc.size - PXY_VFS_PCK_CHUNK_DSC_SIZE)))
			{
				if (data != &prevChunk->data.rawData)
					memcpy(&prevChunk->data.rawData, data, sz);
				cuPck->UpdateChunk(prevChunk);
				return prevChunk;
			}
		}
	}

	d912pxy_vfs_pck_chunk * newChunk = cuPck->WriteFileToPck(id, namehash, sz, data);

	if (prevChunk)
		PXY_FREE(prevChunk);

	return newChunk;
}

void d912pxy_vfs::SetWriteMask(UINT32 val)
{
	cuWriteMask = val;
}

void d912pxy_vfs::TakeOutWriteAccess()
{
	if (writeAllowed)
		CloseHandle(lockFile);

	writeAllowed = 0;
}

void d912pxy_vfs::LoadPckFromRootPath()
{
	WIN32_FIND_DATA fdFile;
	HANDLE hFind = NULL;

	wchar_t sPath[2048];
	wsprintf(sPath, L"%S\\*.pck", m_rootPath);

	if ((hFind = FindFirstFile(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
		return;

	std::wstring FileName;
	std::vector<std::wstring> ListOfFileNames;

	do
	{
		if (wcscmp(fdFile.cFileName, L".") != 0
			&& wcscmp(fdFile.cFileName, L"..") != 0)
		{			
			//Is the entity a File or Folder? 
			if (fdFile.dwFileAttributes &FILE_ATTRIBUTE_DIRECTORY)
			{
				;
			}
			else {
				FileName = fdFile.cFileName;
				ListOfFileNames.push_back(FileName);
			}
		}
	} while (FindNextFile(hFind, &fdFile)); 

	FindClose(hFind); 

	std::sort(ListOfFileNames.begin(), ListOfFileNames.end());
	
	for (std::vector<std::wstring>::iterator it = ListOfFileNames.begin(); it != ListOfFileNames.end(); ++it)
	{
		wsprintf(sPath, L"%S/%s", m_rootPath, it->c_str());

		d912pxy_vfs_pck* pck = new d912pxy_vfs_pck(sPath, writeAllowed);
		
		if (pck->GetStatus())
		{
			delete pck;
		}
		else {
			d912pxy_ringbuffer<d912pxy_vfs_pck_chunk*>* fileList = pck->GetFileList();
		
			while (fileList->HaveElements())
			{
				d912pxy_vfs_pck_chunk* fileInfo = fileList->GetElement();

				items[fileInfo->data.file_info.category]->AddFileInfo(fileInfo);

				fileList->Next();
			}

			delete fileList;

			//megai2: never unload latest pck file on ref changes				
			if (!wcscmp(it->c_str(), PXY_VFS_LATEST_PCK))
			{
				cuPck = pck;				
			} else 
				pck->ModRef(-1);
		}		
	}	

	UINT64 memCache = d912pxy_s.config.GetValueXI64(PXY_CFG_VFS_MEMCACHE_MASK);

	//megai2: load actual data from disk after we collect/overwrite listing from all pck files
	//if precache specified
	for (int i = 0; i != (UINT)d912pxy_vfs_bid::end; ++i)
		//precache all data if write is disabled to allow multi instance runs
		if ((memCache & (1ULL << i)) || !writeAllowed)
			items[i]->LoadFilesFromDisk();
}

d912pxy_vfs_path_hash d912pxy_vfs_path::HashFromName(const char * fnpath)
{
	UINT len = (UINT32)strlen(fnpath);
	d912pxy_vfs_path_hash ret = d912pxy::Hash64(d912pxy::MemoryArea((void*)fnpath, len)).value;

	auto checkPath = d912pxy_vfs_path(ret, d912pxy_vfs_bid::vfs_paths);
	if (d912pxy_s.vfs.IsFilePresent(checkPath))
	{
		auto savedPath = d912pxy_s.vfs.ReadFile(checkPath);
		if ((savedPath.size() != len) || (memcmp(savedPath.ptr(), fnpath, savedPath.size()) != 0))
		{
			abort();
		}
		savedPath.Delete();
	}
	else
		d912pxy_s.vfs.WriteFile(checkPath, d912pxy_mem_block::use((void*)fnpath, len));

	//TODO also add check for string based path & hash based path conflicts

	return ret;
}
