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
#pragma once
#include "stdafx.h"

typedef struct d912pxy_shader_pair_cache_entry {
	d912pxy_shader_uid ps;
	d912pxy_shader_uid vs;
	d912pxy_trimmed_pso_desc::StorageKey pso;
} d912pxy_shader_pair_cache_entry;

class d912pxy_shader_pair : public d912pxy_noncom
{
public:
	d912pxy_shader_pair(d912pxy_shader_pair_hash_type hash, d912pxy_shader_uid* shd);
	~d912pxy_shader_pair();

	d912pxy_pso_item** PrecompilePSO(UINT32 idx, d912pxy_trimmed_pso_desc* dsc);
	void CheckArrayAllocation(UINT32 idx);

	d912pxy_pso_item* GetPSOItem(UINT32 idx, d912pxy_trimmed_pso_desc* dsc);
	d912pxy_pso_item* GetPSOItemMT(UINT32 idx, d912pxy_trimmed_pso_desc* dsc);

private:
	
	d912pxy_pso_item** psoItems;
	UINT32 maxPsoId;

	d912pxy_shader_pair_hash_type node;
	d912pxy_shader_uid shdUID[2];

	d912pxy_thread_lock lock;

};

