#pragma once
#include "stdafx.h"

typedef enum d912pxy_config_value {
	PXY_CFG_POOLING_UPLOAD_ALLOC_STEP = 0,
	PXY_CFG_POOLING_UPLOAD_LIMITS = 1,
	PXY_CFG_POOLING_VSTREAM_LIMITS,
	PXY_CFG_CNT
} d912pxy_config_value;

typedef struct d912pxy_config_value_dsc {
	wchar_t section[256];
	wchar_t name[256];
	wchar_t value[256];
} d912pxy_config_value_dsc;

#define PXY_CFG_FILE_NAME "d912pxy/config.ini"

class d912pxy_config 
{
public:
	d912pxy_config();
	~d912pxy_config();

	UINT64 GetValueUI64(d912pxy_config_value val);
	wchar_t* GetValueRaw(d912pxy_config_value val);

private:

	d912pxy_config_value_dsc data[PXY_CFG_CNT] = {
		{L"pooling", L"upload_alloc_step", L"128"},//PXY_CFG_POOLING_UPLOAD_ALLOC_STEP
		{L"pooling", L"upload_limits", L"0x0100 0050 0050 0010 0010 0005 0005 0005 0000 0000 0000 0000 L0"},//PXY_CFG_POOLING_UPLOAD_LIMITS		
		{L"pooling", L"vstream_limits", L"0x0100 0050 0050 0010 0010 0005 0005 0005 0000 0000 0000 0000 L0"}//PXY_CFG_POOLING_VSTREAM_LIMITS
	};
};

