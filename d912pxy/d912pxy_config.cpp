#include "stdafx.h"

d912pxy_config::d912pxy_config()
{
	d912pxy_s(config) = this;

	FILE* f = fopen(PXY_CFG_FILE_NAME, "r");

	if (!f)
		return;

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

			if (buf[i] == L'=')
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

wchar_t * d912pxy_config::GetValueRaw(d912pxy_config_value val)
{
	return &data[val].value[0];
}
