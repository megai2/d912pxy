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

d912pxy_shader_replacer::d912pxy_shader_replacer(DWORD * fun, UINT len, d912pxy_shader_uid UID, UINT isVs) : d912pxy_noncom( L"shader replacer")
{
	mUID = UID;
	oCode = fun;
	oLen = len;

	if (fun)
		vsSig = CheckTypeSignature();
	else
		vsSig = isVs;
}

d912pxy_shader_replacer::~d912pxy_shader_replacer()
{

}

d912pxy_shader_code d912pxy_shader_replacer::CompileFromHLSL_CS(const wchar_t* bfolder)
{
	wchar_t replFn[1024];

	//megai2: %016llX bugged out
	wsprintf(replFn, L"%s/%08lX%08lX.hlsl", bfolder, (int)(mUID >> 32), (int)(mUID & 0xFFFFFFFF));

	char targetCompiler[] = "cs_5_1";

	ComPtr<ID3DBlob> ret, eret;

#ifdef _DEBUG
	HRESULT compRet = d912pxy_s.imports.d3d_compiler.CompileFromFile(replFn, 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", targetCompiler, D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES | D3DCOMPILE_DEBUG, 0, &ret, &eret);
#else
	HRESULT compRet = d912pxy_s.imports.d3d_compiler.CompileFromFile(replFn, 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", targetCompiler, D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES, 0, &ret, &eret);
#endif

	if ((compRet != S_OK) && (eret == NULL))
	{
		LOG_ERR_DTDM("shd compiler err = %08lX", compRet);

		d912pxy_shader_code ret2;

		ret2.code = 0;
		ret2.sz = 0;
		ret2.blob = nullptr;

		return ret2;

	}
	else if (eret != NULL && ret == NULL)
	{
		LOG_ERR_DTDM("shd compile err = %S", eret->GetBufferPointer());

		d912pxy_shader_code ret2;

		ret2.code = 0;
		ret2.sz = 0;
		ret2.blob = nullptr;

		return ret2;
	}
	else {

		if (eret != NULL)
		{
			LOG_WARN_DTDM("shd compile warning = %S", eret->GetBufferPointer());
		}

		d912pxy_shader_code ret2;

		ret2.code = ret->GetBufferPointer();
		ret2.sz = ret->GetBufferSize();
		ret2.blob = ret;
		
		return ret2;
	}
}

d912pxy_shader_code d912pxy_shader_replacer::CompileFromHLSL(const wchar_t* bfolder, UINT keepSource)
{
	wchar_t replFn[1024];

	//megai2: %016llX bugged out
	wsprintf(replFn, L"%s/%08lX%08lX.hlsl", bfolder, (int)(mUID >> 32), (int)(mUID & 0xFFFFFFFF));

	char targetCompiler[] = "ps_5_1";

	if (vsSig)
		targetCompiler[0] = L'v';

	ComPtr<ID3DBlob> ret, eret;
	
#ifdef _DEBUG
	HRESULT compRet = d912pxy_s.imports.d3d_compiler.CompileFromFile(replFn, 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", targetCompiler, D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES | D3DCOMPILE_DEBUG, 0, &ret, &eret);
#else
 	HRESULT compRet = d912pxy_s.imports.d3d_compiler.CompileFromFile(replFn, 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", targetCompiler, D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES, 0, &ret, &eret);
#endif

	if ((compRet != S_OK) && (eret == NULL))
	{
		//megai2: that should be converted via hresult processing, but nah
		if (compRet != 0x80070002)//ERROR_FILE_NOT_FOUND
			LOG_ERR_DTDM("shd compiler err = %08lX", compRet);

		d912pxy_shader_code ret2;

		ret2.code = 0;
		ret2.sz = 0;
		ret2.blob = nullptr;
		
		return ret2;

	} else if (eret != NULL && ret == NULL)
	{
		LOG_ERR_DTDM("shd compile err = %S", eret->GetBufferPointer());

		d912pxy_shader_code ret2;

		ret2.code = 0;
		ret2.sz = 0;
		ret2.blob = nullptr;

		return ret2;
	}
	else {

		if (eret != NULL)
		{
			LOG_DBG_DTDM3("shd compile warning = %S", eret->GetBufferPointer());
		}

		d912pxy_shader_code ret2;

		ret2.code = ret->GetBufferPointer();
		ret2.sz = ret->GetBufferSize();
		ret2.blob = ret;

#ifndef _DEBUG
		if (!keepSource)
			DeleteFile(replFn);
#endif
		return ret2;
	}
}

d912pxy_shader_code d912pxy_shader_replacer::CompileFromHLSL_MEM(const wchar_t* bfolder, void* imem, UINT size, UINT saveSource)
{
	char replFn[1024];

	//megai2: %016llX bugged out
	sprintf(replFn, "%S/%08lX%08lX.hlsl", bfolder, (int)(mUID >> 32), (int)(mUID & 0xFFFFFFFF));

	char targetCompiler[] = "ps_5_1";

	if (vsSig)
		targetCompiler[0] = L'v';

	ComPtr<ID3DBlob> ret, eret;

#ifdef _DEBUG
	HRESULT compRet = d912pxy_s.imports.d3d_compiler.Compile(imem, size, replFn, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", targetCompiler, D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES | D3DCOMPILE_DEBUG, 0, &ret, &eret);
#else
	HRESULT compRet = d912pxy_s.imports.d3d_compiler.Compile(imem, size, replFn, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", targetCompiler, D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES, 0, &ret, &eret);
#endif

	if ((compRet != S_OK) && (eret == NULL))
	{
		//megai2: that should be converted via hresult processing, but nah
		if (compRet != 0x80070002)//ERROR_FILE_NOT_FOUND
			LOG_ERR_DTDM("shd compiler err = %08lX", compRet);

		d912pxy_shader_code ret2;

		ret2.code = 0;
		ret2.sz = 0;
		ret2.blob = nullptr;

		return ret2;

	}
	else if (eret != NULL && ret == NULL)
	{
		LOG_ERR_DTDM("shd compile err = %S", eret->GetBufferPointer());

		d912pxy_shader_code ret2;

		ret2.code = 0;
		ret2.sz = 0;
		ret2.blob = nullptr;

		return ret2;
	}
	else {

		if (eret != NULL)
		{
			LOG_DBG_DTDM("shd compile warning = %S", eret->GetBufferPointer());
		}

		d912pxy_shader_code ret2;

		ret2.code = ret->GetBufferPointer();
		ret2.sz = ret->GetBufferSize();
		ret2.blob = ret;

		if (saveSource)
		{
			d912pxy_s.vfs.WriteFile(d912pxy_vfs_path(mUID, d912pxy_vfs_bid::shader_sources), d912pxy_mem_block::use(imem, size));			
		}

		return ret2;
	}
}

d912pxy_shader_code d912pxy_shader_replacer::LoadFromCSO(const char* bfolder)
{		
	d912pxy_mem_block mem = d912pxy_s.vfs.ReadFile(d912pxy_vfs_path(mUID, d912pxy_vfs_bid::cso));

	d912pxy_shader_code ret;
	ret.blob = nullptr;
	ret.code = mem.ptr();
	ret.sz = mem.size();
		
	return ret;

}

void d912pxy_shader_replacer::SaveCSO(d912pxy_shader_code code, const char * bfolder)
{
	d912pxy_s.vfs.WriteFile(d912pxy_vfs_path(mUID, d912pxy_vfs_bid::cso), d912pxy_mem_block::use(code.code, code.sz));	
}

d912pxy_hlsl_generator_memout* d912pxy_shader_replacer::GenerateHLSL(const wchar_t * bfolder)
{
	d912pxy_hlsl_generator_memout* ret = 0;

	wchar_t replFn[4096];
	wsprintf(replFn, L"%s/%08lX%08lX.hlsl", bfolder, (UINT32)(mUID >> 32), (UINT32)(mUID & 0xFFFFFFFF));

	d912pxy_hlsl_generator* gen = new d912pxy_hlsl_generator(oCode, oLen, replFn, mUID);

	try {
		ret = gen->Process(1);
	}
	catch (...)
	{		
		ret = 0;
	}

	if (!ret)
	{
		wsprintf(replFn, L"%s/%08lX%08lX.dxbc", bfolder, (UINT32)(mUID >> 32), (UINT32)(mUID & 0xFFFFFFFF));
		LOG_ERR_DTDM("hlsl generator failed, dumping original bytecode to %s", replFn);

		FILE* dumpFile = _wfopen(replFn, L"wb+");
		if (dumpFile)
		{
			fwrite(oCode, 4, oLen, dumpFile);
			fclose(dumpFile);
		}
		else {
			LOG_ERR_DTDM("failed to dump bytecode");
		}
	}

	delete gen;

	return ret;
}

d912pxy_mem_block d912pxy_shader_replacer::GetHLSL()
{
	wchar_t replFn[1024];

	wsprintf(replFn, L"%s/%08lX%08lX.hlsl", d912pxy_helper::GetFilePath(FP_SHADER_DB_HLSL_CUSTOM_DIR)->w, (int)(mUID >> 32), (int)(mUID & 0xFFFFFFFF));

	FILE* customHLSL = _wfopen(replFn, L"rb+");
	if (customHLSL)
	{
		fseek(customHLSL, 0, SEEK_END);
		int fsz = ftell(customHLSL);
		fseek(customHLSL, 0, SEEK_SET);
		
		if (!fsz)
			fclose(customHLSL);
		else
		{
			auto ret = d912pxy_mem_block::alloc(fsz);
			fread(ret.ptr(), 1, fsz, customHLSL);
			fclose(customHLSL);
			return ret;
		}
	}

	d912pxy_hlsl_generator_memout* genRet = GenerateHLSL(d912pxy_helper::GetFilePath(FP_SHADER_DB_HLSL_DIR)->w);
	if (!genRet)
		return d912pxy_mem_block::null();

	auto ret = d912pxy_mem_block::from(genRet->data, genRet->size);
	PXY_FREE(genRet);

	d912pxy_s.vfs.WriteFile(d912pxy_vfs_path(mUID, d912pxy_vfs_bid::shader_sources), ret);
	return ret;
}

d912pxy_shader_code d912pxy_shader_replacer::GetCode()
{	
	d912pxy_shader_code ret = LoadFromCSO(d912pxy_helper::GetFilePath(FP_SHADER_DB_CSO_DIR)->s);

	if (!ret.code)
	{
		ret = CompileFromHLSL(d912pxy_helper::GetFilePath(FP_SHADER_DB_HLSL_CUSTOM_DIR)->w, 1);

		if ((oCode == NULL) && (ret.code == NULL))			
			return ret;		

		if (!ret.code)
		{
			d912pxy_hlsl_generator_memout* genRet = GenerateHLSL(d912pxy_helper::GetFilePath(FP_SHADER_DB_HLSL_DIR)->w);
			if (!genRet)
				return ret;

			ret = CompileFromHLSL_MEM(d912pxy_helper::GetFilePath(FP_SHADER_DB_HLSL_DIR)->w, genRet->data, genRet->size, 1);			

			if (!ret.code)
			{
				wchar_t replFn[4096];
				wsprintf(replFn, L"%s/err_%08lX%08lX.hlsl", d912pxy_helper::GetFilePath(FP_SHADER_DB_HLSL_DIR)->w, (UINT32)(mUID >> 32), (UINT32)(mUID & 0xFFFFFFFF));

				LOG_ERR_DTDM("Can't compile generated shader, dumping HLSL code to %s", replFn);				

				FILE* dumpFile = _wfopen(replFn, L"wb+");
				if (dumpFile)
				{
					fwrite(genRet->data, 1, genRet->size, dumpFile);
					fwrite(oCode, 4, oLen, dumpFile);
					fclose(dumpFile);
				}
				else {
					LOG_ERR_DTDM("failed to dump generated code");
				}
			}
			else {
				SaveCSO(ret, d912pxy_helper::GetFilePath(FP_SHADER_DB_CSO_DIR)->s);
			}

			PXY_FREE(genRet);
		} else 
			SaveCSO(ret, d912pxy_helper::GetFilePath(FP_SHADER_DB_CSO_DIR)->s);
	}

	return ret;
}

d912pxy_shader_code d912pxy_shader_replacer::GetCodeCS()
{
	d912pxy_shader_code ret = LoadFromCSO(d912pxy_helper::GetFilePath(FP_CS_CSO_DIR)->s);

	if (!ret.code)
	{
		ret = CompileFromHLSL_CS(d912pxy_helper::GetFilePath(FP_CS_HLSL_DIR)->w);
		if (!ret.code)
			LOG_ERR_THROW2(-1, "cs code error");
		else
			SaveCSO(ret, d912pxy_helper::GetFilePath(FP_CS_CSO_DIR)->s);
	}

	return ret;
}

UINT d912pxy_shader_replacer::GetMaxVars()
{
/*	wchar_t replFn[1024];
	wsprintf(replFn, L"%s_v/%08lX%08lX.vf", d912pxy_shader_db_hlsl_dir, mUID >> 32, mUID & 0xFFFFFFFF);

	FILE* f = _wfopen(replFn, L"rb");

	UINT maxVars = 256;

	if (f)
	{		
		fread(&maxVars, 4, 1, f);
		fclose(f);

		maxVars = ((((maxVars & 0xF) != 0) * 0x10) + (maxVars & (~0xF)));

		if (maxVars > 256)
			maxVars = 256;
		
	}
		
	return maxVars*16;*/
	return 4096;
}

UINT d912pxy_shader_replacer::CheckTypeSignature()
{
	//Each individual shader code is formatted with a general token layout. The first token must be a version token. 
	//[31:16] Bits 16 through 31 specify whether the code is for a pixel or vertex shader. For a pixel shader, the value is 0xFFFF. For a vertex shader, the value is 0xFFFE.
	
	return ((oCode[0] & (1 << 16)) == 0);
}