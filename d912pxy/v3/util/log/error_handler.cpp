/*
MIT License

Copyright(c) 2020 megai2

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

void d912pxy::error::fatal(const wchar_t* reason, ...)
{
	d912pxy_s.log.text.SyncCrashWrite(1);

	constexpr int bufSize = 1024 * 10;
	wchar_t formattedReason[bufSize];
	va_list arg;
	va_start(arg, reason);
	vswprintf(formattedReason, bufSize, reason, arg);
	va_end(arg);

	d912pxy_s.log.text.WriteCrashLogLine((wchar_t*)formattedReason);

	d912pxy_s.log.text.SyncCrashWrite(0);

	MessageBoxW(0, formattedReason, L"d912pxy fatal error", MB_OK);

	//here should be fatal / abort / crash / exit / whatever but we do belive in miracles, a'nt we? ;)
}