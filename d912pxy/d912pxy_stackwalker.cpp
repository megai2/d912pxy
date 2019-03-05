/*
MIT License

Copyright(c) 2018-2019 megai2

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
#include "d912pxy_stackwalker.h"

d912pxy_StackWalker::d912pxy_StackWalker() : StackWalker(), d912pxy_noncom(NULL, L"StkWalk")
{
}

d912pxy_StackWalker::~d912pxy_StackWalker()
{
}

void d912pxy_StackWalker::OnOutput(LPCSTR szText)
{
	wchar_t buf[4096];
	wsprintf(buf, L"%S", szText);

	m_log->P7_ERROR(LGC_DEFAULT, L"%s", buf);

	FILE* f = fopen("d912pxy_crash.txt", "ab");
	
	fwrite(buf, 2, lstrlenW(buf)-1, f);

	const wchar_t* nlv = L"\r\n";
	fwrite(nlv, 2, 2, f);

	fclose(f);
}
