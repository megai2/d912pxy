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

using namespace d912pxy;
using namespace d912pxy::KeyValue;

Reader::Reader(const MemoryBlock& source)
{	
	Element newElem = {};

	wchar_t* curString = source.c_arr<wchar_t>();

	if (*curString != 0xFEFF)
		return;
	else
		++curString;

	wchar_t* varr[3] = {};
	varr[0] = curString;
	int curVal = 0;
	bool skipNextDelimiter = false;

	while (curString < source.end<wchar_t>())
	{
		if ((*curString == L'=') || (*curString == L'\n') || (*curString == L'\r'))
		{
			if (skipNextDelimiter)
			{
				++curString;
				varr[curVal] = curString;
				continue;
			}
			*curString = 0;
			++curVal;
			varr[curVal] = curString + 1;
			skipNextDelimiter = true;
		}
		else
			skipNextDelimiter = false;

		if (curVal == 2)
		{
			newElem.key = varr[0];
			newElem.value = varr[1];

			data.push(newElem);

			varr[0] = varr[2];
			curVal = 0;
		}

		++curString;
	}
}
