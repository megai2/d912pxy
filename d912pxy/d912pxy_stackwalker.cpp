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
	m_log->P7_ERROR(LGC_DEFAULT, L"%S", szText);

	FILE* f = fopen("d912pxy_crash.txt", "ab");
	fwrite(szText, 1, lstrlenA(szText), f);
	fclose(f);
}
