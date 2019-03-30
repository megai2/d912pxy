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

d912pxy_config::d912pxy_config()
{
	d912pxy_s(config) = this;

	FILE* f = fopen(PXY_CFG_FILE_NAME, "r");

	if (!f) {
		f = fopen(PXY_CFG_FILE_NAME, "w");

		wchar_t csection[256] = L"0";


		for (int i = 0; i != PXY_CFG_CNT; ++i)
		{
			if (lstrcmpW(csection, data[i].section))
			{
				fwprintf(f, L"\r\n[%s]\r\n", data[i].section);
				lstrcpyW(csection, data[i].section);
			}

			fwprintf(f, L"%s=%s\r\n", data[i].name, data[i].value);		
		}

		fwprintf(f, L"\r\n[end] \r\n");

		fflush(f);
		fclose(f);
		
		return;
	}

	wchar_t section[256];
	wchar_t param[256];
	wchar_t val[256];

	while (!feof(f))
	{
		wchar_t buf[256];
		fgetws(buf, 256, f);
		
		UINT dlmt = 0;
		UINT valf = 0;

		for (int i = 0; i != 256; ++i)
		{
			if (buf[i] == L'[')
			{
				dlmt = i + 1;
			}

			if (buf[i] == L']')
			{
				memcpy(section, &buf[dlmt], sizeof(wchar_t)*(i - dlmt));
				section[i - dlmt] = 0;
			}

			if ((buf[i] == L'=') && (valf == 0))
			{
				memcpy(param, buf, sizeof(wchar_t)*i);
				param[i] = 0;
				valf = 1;
				dlmt = i + 1;
			}

			if (((buf[i] == L'\r') || (buf[i] == L'\n')) && valf)
			{
				memcpy(val, &buf[dlmt], sizeof(wchar_t)*(i - dlmt));
				val[i - dlmt] = 0;
				break;
			}

			if (buf[i] == 0)
				break;
		}

		if (valf)
		{
			for (int i = 0; i != PXY_CFG_CNT; ++i)
			{
				if (lstrcmpW(section, data[i].section))
					continue;

				if (lstrcmpW(param, data[i].name))
					continue;

				lstrcpyW(data[i].value, val);

				break;
			}
		}
	}

	fclose(f);
}


d912pxy_config::~d912pxy_config()
{
}

UINT64 d912pxy_config::GetValueXI64(d912pxy_config_value val)
{
	UINT64 ret = 0;
	swscanf(&data[val].value[0], L"%llX", &ret);
	return ret;
}

UINT64 d912pxy_config::GetValueUI64(d912pxy_config_value val)
{
	return _wtoi64(&data[val].value[0]);
}

UINT32 d912pxy_config::GetValueUI32(d912pxy_config_value val)
{
	return (UINT32)GetValueUI64(val);
}

wchar_t * d912pxy_config::GetValueRaw(d912pxy_config_value val)
{
	return &data[val].value[0];
}

d912pxy_config_value_dsc * d912pxy_config::GetEntryRaw(d912pxy_config_value val)
{
	return &data[val];
}
