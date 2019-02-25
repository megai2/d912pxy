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
	
	fwrite(szText, 2, lstrlen(buf)-1, f);

	const wchar_t* nlv = L"\r\n";
	fwrite(nlv, 2, 2, f);

	fclose(f);
}
