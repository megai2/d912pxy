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

class d912pxy_vfs_entry : public d912pxy_noncom
{
public:
	typedef d912pxy::Memtree<d912pxy::MemoryArea, d912pxy_vfs_pck_chunk*, d912pxy::Hash64> ChunkTree;

	d912pxy_vfs_entry(UINT id);
	~d912pxy_vfs_entry();
	d912pxy_vfs_entry(const d912pxy_vfs_entry&) = delete;

	d912pxy_vfs_pck_chunk* IsPresentH(UINT64 fnHash);
	void* GetFileDataH(UINT64 namehash, UINT64* sz);

	void WriteFileH(UINT64 namehash, void* data, UINT64 sz);
	void ReWriteFileH(UINT64 namehash, void* data, UINT64 sz);

	void AddFileInfo(d912pxy_vfs_pck_chunk* fileInfo);
	void LoadFileFromDisk(d912pxy_vfs_pck_chunk* fileInfo);
	void LoadFilesFromDisk();

	ChunkTree* GetChunkTree() { return chunkTree; };
	d912pxy_vfs_pck_chunk* GetLastChunk() { return *lastFind; }

private:	
	ChunkTree* chunkTree;
	d912pxy_vfs_pck_chunk** lastFind;

	
	UINT m_Id;
};