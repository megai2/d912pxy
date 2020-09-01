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

const char* extraSymSearchPath =
	"./addons/d912pxy/dll/debug;"
	"./addons/d912pxy/dll/release;"
	"./addons/d912pxy/dll/release_avx2;"
	"./d912pxy/dll/debug;"
	"./d912pxy/dll/release;"
	"./d912pxy/dll/release_avx2;"
  ;

d912pxy_StackWalker::d912pxy_StackWalker(UINT32 opts, UINT32 saveCaller) : StackWalker(opts | StackWalkOptions::SymBuildPath, extraSymSearchPath)
{
	if (!saveCaller)
		d912pxy_s.log.text.SyncCrashWrite(1);

	saveCallerToBuffer = saveCaller;
	callerLineNr = 0;
	caller = 0;
}

d912pxy_StackWalker::~d912pxy_StackWalker()
{
	if (!saveCallerToBuffer)
		d912pxy_s.log.text.SyncCrashWrite(0);
}

void d912pxy_StackWalker::OnOutput(LPCSTR szText)
{
	if (!saveCallerToBuffer)
	{
		wchar_t buf[4096];
		wsprintf(buf, L"%S", szText);

		//LOG_ERR_DTDM("%s", buf);

		d912pxy_s.log.text.WriteCrashLogLine(buf);
	}
	else if (saveCallerToBuffer == callerLineNr)
	{
		caller = _strdup(szText);
	}

}

void d912pxy_StackWalker::OnCallstackEntry(CallstackEntryType eType, CallstackEntry & entry)
{
	if (saveCallerToBuffer)
		++callerLineNr;
	
	StackWalker::OnCallstackEntry(eType, entry);
}

char * d912pxy_StackWalker::ReturnCaller()
{
	callerLineNr = 0;
	return caller;
}
