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
#pragma once
#include "stdafx.h"

typedef enum d912pxy_config_value {
	PXY_CFG_POOLING_UPLOAD_ALLOC_STEP = 0,
	PXY_CFG_POOLING_UPLOAD_LIMITS = 1,
	PXY_CFG_POOLING_VSTREAM_LIMITS,
	PXY_CFG_POOLING_SURFACE_LIMITS,
	PXY_CFG_POOLING_LIFETIME,
	PXY_CFG_SAMPLERS_MIN_LOD,	
	PXY_CFG_CLEANUP_PERIOD,
	PXY_CFG_CLEANUP_SUBSLEEP,	
	PXY_CFG_SDB_KEEP_PAIRS,
	PXY_CFG_SDB_USE_PSO_PRECOMPILE,
	PXY_CFG_SDB_USE_PSO_KEY_CACHE,
	PXY_CFG_REPLAY_MT_BEHAIVOUR,
	PXY_CFG_REPLAY_THREADS,
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

	UINT64 GetValueXI64(d912pxy_config_value val);
	UINT64 GetValueUI64(d912pxy_config_value val);
	wchar_t* GetValueRaw(d912pxy_config_value val);

private:

	d912pxy_config_value_dsc data[PXY_CFG_CNT] = {
		{L"pooling", L"upload_alloc_step", L"0"},//PXY_CFG_POOLING_UPLOAD_ALLOC_STEP
		{L"pooling", L"upload_limits", L"0x0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 L00"},//PXY_CFG_POOLING_UPLOAD_LIMITS		
		{L"pooling", L"vstream_limits", L"0x0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 L00"},//PXY_CFG_POOLING_VSTREAM_LIMITS
		{L"pooling", L"surface_limits",L"00000"},//PXY_CFG_POOLING_SURFACE_LIMITS
		{L"pooling", L"lifetime",L"10000"},//PXY_CFG_POOLING_LIFETIME
		{L"samplers", L"min_lod", L"0"},//PXY_CFG_SAMPLERS_MIN_LOD		
		{L"cleanup", L"period",L"10000"},//PXY_CFG_CLEANUP_PERIOD
		{L"cleanup", L"subsleep",L"250"},//PXY_CFG_CLEANUP_SUBSLEEP	
		{L"sdb", L"keep_pairs", L"0"},//PXY_CFG_SDB_KEEP_PAIRS
		{L"sdb", L"use_pso_precompile", L"0"},//PXY_CFG_SDB_USE_PSO_PRECOMPILE
		{L"sdb", L"use_pso_key_cache", L"0"},//PXY_CFG_SDB_USE_PSO_KEY_CACHE
		{L"replay", L"mt", L"1"},//PXY_CFG_REPLAY_MT_BEHAIVOUR
		{L"replay", L"threads", L"1"}//PXY_CFG_REPLAY_THREADS
	};
};

