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

#define PXY_SDB_PSO_PRECOMPILE_SAVE 1
#define PXY_SDB_PSO_PRECOMPILE_LOAD 2

class d912pxy_shader_db : public d912pxy_noncom
{
public:
	d912pxy_shader_db();
	~d912pxy_shader_db();

	void Init();
	void UnInit();

	d912pxy_shader_uid GetUID(DWORD* code, UINT32* len);		

	d912pxy_shader_pair_hash_type GetPairUID(d912pxy_shader* vs, d912pxy_shader* ps);

	d912pxy_shader_pair* GetPair(d912pxy_shader* vs, d912pxy_shader* ps);
	void DeletePair(d912pxy_shader_pair_hash_type ha);

private:
	typedef d912pxy::Memtree<d912pxy_shader_pair_hash_type, d912pxy_shader_pair*, d912pxy::RawHash<d912pxy_shader_pair_hash_type>> ShaderPairStorage;
	ShaderPairStorage shaderPairs;
};

