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

d912pxy_config::d912pxy_config()
{
}


d912pxy_config::~d912pxy_config()
{
}

void d912pxy_config::Init()
{
	FILE* f = fopen(d912pxy_helper::GetFilePath(FP_CONFIG)->s, "rb");

	if (!f) {
		SaveConfig();
		return;
	}

	wchar_t section[256];
	wchar_t param[256];
	wchar_t val[256];

	fseek(f, 0, SEEK_END);
	int fsz = ftell(f);
	fseek(f, 0, SEEK_SET);

	if (fsz <= 0)
	{
		if (f)
			fclose(f);
		return;
	}

	int fptr = 0;

	wchar_t* fileContent = NULL;

	//megai2: config loaded when nothing initialized
	fileContent = (wchar_t*)malloc(fsz);

	fread(fileContent, 1, fsz, f);

	fsz = fsz / sizeof(wchar_t);

	while (fptr != fsz)
	{
		UINT dlmt = 0;
		UINT valf = 0;

		wchar_t* buf = &fileContent[fptr];

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

			if (((buf[i] == L'\r') || (buf[i] == L'\n')))
			{
				if (!valf)
				{
					++fptr;
					if ((fptr < fsz) && (buf[i + 1] == L'\n'))
						++fptr;
					break;
				}

				memcpy(val, &buf[dlmt], sizeof(wchar_t)*(i - dlmt));
				val[i - dlmt] = 0;

				++fptr;
				if ((fptr < fsz) && (buf[i + 1] == L'\n'))
					++fptr;

				break;
			}

			if (buf[i] == 0)
			{
				++fptr;
				break;
			}

			++fptr;

			if (fptr == fsz)
				break;

		}

		if (valf)
		{
			for (auto &&d : data)
			{
				if (lstrcmpW(section, d.section))
					continue;

				if (lstrcmpW(param, d.name))
					continue;

				lstrcpyW(d.value, val);

				break;
			}
		}
	}

	free(fileContent);

	fclose(f);
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

bool d912pxy_config::GetValueB(d912pxy_config_value val)
{
	return GetValueUI32(val) >= 1;
}

wchar_t * d912pxy_config::GetValueRaw(d912pxy_config_value val)
{
	return &data[val].value[0];
}

void d912pxy_config::InitNewValueBuffers()
{
	for(int i = 0; i != PXY_CFG_CNT; ++i)
	{
		PXY_MALLOC(data[i].newValue, 255, char*);
	}
	ValueToNewValueBuffers();
}

void d912pxy_config::UnInitNewValueBuffers()
{
	for (int i = 0; i != PXY_CFG_CNT; ++i)
	{
		PXY_FREE(data[i].newValue);
		data[i].newValue = nullptr;
	}
}

void d912pxy_config::ValueToNewValueBuffers() 
{
	for (int i = 0; i != PXY_CFG_CNT; ++i)
	{
		wcstombs(data[i].newValue, data[i].value, 255);
	}
}

void d912pxy_config::SaveConfig()
{
	FILE* f = fopen(d912pxy_helper::GetFilePath(FP_CONFIG)->s, "wb");

	if (!f)
	{
		char buf[4096];
		char cwd[4096];
		GetCurrentDirectoryA(4096, cwd);

		sprintf_s(buf, "Can't save config to %s, check folder write permissions! (cwd: %s)", d912pxy_helper::GetFilePath(FP_CONFIG)->s, cwd);
		MessageBoxA(0, buf, "d912pxy error", MB_ICONERROR);
		return;
	}

	wchar_t csection[256] = L"0";

	bool writeNewValues = data[0].newValue != nullptr;

	for (int i = 0; i != PXY_CFG_CNT; ++i)
	{

		if (lstrcmpW(csection, data[i].section))
		{
			fwprintf(f, L"\r\n[%s]\r\n", data[i].section);
			lstrcpyW(csection, data[i].section);
		}

		if (writeNewValues)
		{
			wchar_t conversion_buffer[256] = {0};
			mbstowcs(conversion_buffer, data[i].newValue, 255);
			fwprintf(f, L"%s=%s\r\n", data[i].name, conversion_buffer);
		}
		else 
		{
			fwprintf(f, L"%s=%s\r\n", data[i].name, data[i].value);
		}
	}

	fwprintf(f, L"\r\n[end] \r\n");

	fflush(f);
	fclose(f);
}

d912pxy_config_value_dsc * d912pxy_config::GetEntryRaw(d912pxy_config_value val)
{
	return &data[val];
}
