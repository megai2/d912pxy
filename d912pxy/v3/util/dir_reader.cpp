#include "stdafx.h"
#include "dir_reader.h"

using namespace d912pxy;

DirReader::DirReader(const wchar_t* root, const wchar_t* path)
{
	WIN32_FIND_DATA fdFile;
	HANDLE hFind = NULL;

	wchar_t sPath[2048];
	wsprintf(sPath, L"%s%s/*", root, path);

	if ((hFind = FindFirstFile(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
		return;

	do
	{
		if (wcscmp(fdFile.cFileName, L".") != 0
			&& wcscmp(fdFile.cFileName, L"..") != 0)
		{
			if (fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				continue;

			wchar_t fPath[2048];
			wsprintf(fPath, L"%s%s/%s", root, path, fdFile.cFileName);

			FILE* f = _wfopen(fPath, L"rb+");

			if (!f)
				continue;

			fseek(f, 0, SEEK_END);
			int fsz = ftell(f);
			fseek(f, 0, SEEK_SET);

			if (!fsz)
			{
				fclose(f);
				continue;
			}

			MemoryBlock& newBlock = dataArray[dataArray.next()];
			newBlock.alloc(fsz);
			fread(newBlock.getPtr(), 1, fsz, f);
			fclose(f);
		}
	} while (FindNextFile(hFind, &fdFile));

	if (dataArray.headIdx())
		cur = 1;
}

d912pxy::DirReader::~DirReader()
{
	for (intptr_t i = 1; i < dataArray.headIdx(); ++i)	
		dataArray[i].~MemoryBlock();	
}

bool DirReader::empty()
{
	return cur > dataArray.headIdx();
}

MemoryBlock& DirReader::next()
{
	return dataArray[cur++];
}

intptr_t d912pxy::DirReader::readed()
{
	return dataArray.headIdx();
}
