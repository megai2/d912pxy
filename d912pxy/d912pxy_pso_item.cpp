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

std::atomic<size_t> d912pxy_pso_item::itemsInCompile { 0 };

d912pxy_pso_item* d912pxy_pso_item::d912pxy_pso_item_com(d912pxy_trimmed_pso_desc* sDsc)
{
	d912pxy_com_object* ret = d912pxy_s.com.AllocateComObj(PXY_COM_OBJ_PSO_ITEM);

	new (&ret->pso_item)d912pxy_pso_item(sDsc);
	ret->vtable = d912pxy_com_route_get_vtable(PXY_COM_ROUTE_EMPTY);

	return &ret->pso_item;
}

d912pxy_pso_item::~d912pxy_pso_item()
{
	if (psoPtr)
	{
		psoPtr.load()->Release();
	}
}

void d912pxy_pso_item::Compile()
{
	//0 full PSO desc: translate trimmed PSO desc to full dx12 PSO desc

	d912pxy_mem_block::alloc(&dx12Desc);
	*dx12Desc = *desc->GetPSODesc();

	//1 RCE: generates primary HLSL from DXBC bytecode & patches things that need a change based on PSO desc (i.e. not tied to DXBC)

	RealtimeIntegrityCheck(*dx12Desc);

	//2 DXC: compile final HLSL codes to DXBC in DXC

	if (fallbacktoNonDerived)
	{
		CreatePSO(*dx12Desc);
		AfterCompileRelease();
	} else {			   
		if (derivedName)	
			MT_PSOCompile();		
		else
		{
			if (!RCELinkDerivedCSO(HLSLsource, derivedAlias))
				MT_DerivedCompile();
			else
			{
				//cleanup hlsl code as it is not longer needed
				HLSLsource[0].Delete();
				HLSLsource[1].Delete();

				MT_PSOCompile();
			}
		}
	} 
		
}

void d912pxy_pso_item::MarkPushedToCompile()
{
	++itemsInCompile;
	AddRef();
	desc->HoldRefs(true);
}

bool d912pxy_pso_item::RetryDerivedPresence()
{
	if (!RCEIsDerivedPresent(derivedName))
		return false;
	
	//cleanup hlsl code as it is not longer needed
	HLSLsource[0].Delete();
	HLSLsource[1].Delete();

	MT_PSOCompile();
	
	return true;
}

void d912pxy_pso_item::DerivedCompile()
{
	//triggers DXC compilation from HLSL source
	if (!RCECompileDerivedCSO(HLSLsource, derivedName))
	{
		PXY_FREE(derivedName);
		derivedName = nullptr;
	}
	
	HLSLsource[0].Delete();
	HLSLsource[1].Delete();

	MT_PSOCompile();
}

void d912pxy_pso_item::PSOCompile()
{
	//3 PSO: make PSO in dx12 using RCE data or fallback to raw hlsl (latter one should be "rare")

	if (derivedName)
	{
		CreatePSODerived(derivedName, *dx12Desc);
		PXY_FREE(derivedName);
	}
	else {
		LOG_ERR_DTDM("RCE failed to generate derived hlsl for %S", derivedAlias);
		CreatePSO(*dx12Desc);
	}

	//4 cleanup

	AfterCompileRelease();
}

void d912pxy_pso_item::CreatePSO(D3D12_GRAPHICS_PIPELINE_STATE_DESC& fullDesc)
{
	if (!fullDesc.VS.pShaderBytecode)
		fullDesc.VS = *desc->ref.VS->GetCode();

	if (!fullDesc.PS.pShaderBytecode)
		fullDesc.PS = *desc->ref.PS->GetCode();

	if (!ValidateFullDesc(fullDesc))
		return;

	LOG_DBG_DTDM("Compiling PSO with vs = %016llX , ps = %016llX", desc->ref.VS->GetID(), desc->ref.PS->GetID());

	ID3D12PipelineState* obj;
	HRESULT psoHRet = d912pxy_s.dx12.dev->CreateGraphicsPipelineState(&fullDesc, IID_PPV_ARGS(&obj));

	if (FAILED(psoHRet))
	{
		LOG_ERR_DTDM("CreateGraphicsPipelineState error %lX for VS %016llX PS %016llX", psoHRet, desc->ref.VS->GetID(), desc->ref.PS->GetID());

		char dumpString[sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC) * 2 + 1];
		dumpString[0] = 0;

		for (int i = 0; i != sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC); ++i)
		{
			char tmp[3];
			sprintf(tmp, "%02X", ((UINT8*)&fullDesc)[i]);
			dumpString[i * 2] = tmp[0];
			dumpString[i * 2 + 1] = tmp[1];
		}

		dumpString[sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC) * 2] = 0;

		LOG_ERR_DTDM("full pso desc dump %S", dumpString);
	}
	else {
		if (d912pxy_s.render.db.pso.IsCacheSavingEnabled())
		{
			d912pxy_shader_pair_cache_entry entryData 
			{ 
				desc->ref.PS->GetID(), 
				desc->ref.VS->GetID(), 
				d912pxy_trimmed_pso_desc::StorageKey(desc->GetValuePart()) 
			};


			char fullPsoName[255];
			sprintf(fullPsoName, "%016llX_%016llX_%08lX", entryData.vs, entryData.ps, entryData.pso.data());

			auto cacheFn = d912pxy_vfs_path(fullPsoName, d912pxy_vfs_bid::pso_precompile_list);

			if (!d912pxy_s.vfs.IsFilePresent(cacheFn))
				d912pxy_s.vfs.WriteFile(cacheFn, d912pxy_mem_block::use(&entryData));
		}
	}

	psoPtr = obj;
}

void d912pxy_pso_item::CreatePSODerived(char* alias, D3D12_GRAPHICS_PIPELINE_STATE_DESC& fullDesc)
{
	d912pxy_mem_block derCSO[2] = {
		d912pxy_s.vfs.ReadFile(d912pxy_vfs_path(alias, d912pxy_vfs_bid::derived_cso_vs)),
		d912pxy_s.vfs.ReadFile(d912pxy_vfs_path(alias, d912pxy_vfs_bid::derived_cso_ps)),
	};

	fullDesc.VS.BytecodeLength = derCSO[0].size();
	fullDesc.PS.BytecodeLength = derCSO[1].size();
	fullDesc.VS.pShaderBytecode = derCSO[0].ptr();
	fullDesc.PS.pShaderBytecode = derCSO[1].ptr();

	CreatePSO(fullDesc);

	derCSO[0].Delete();
	derCSO[1].Delete();
}

char* d912pxy_pso_item::GetDerivedNameByAlias(char* alias)
{
	d912pxy_vfs_path derivedNamePath = d912pxy_vfs_path(alias, d912pxy_vfs_bid::derived_cso_refs);

	if (d912pxy_s.vfs.IsFilePresent(derivedNamePath))
		return d912pxy_s.vfs.ReadFile(derivedNamePath).c_arr<char>();
	else
		return nullptr;
}

bool d912pxy_pso_item::PerformRCE(char* alias, D3D12_GRAPHICS_PIPELINE_STATE_DESC& fullDesc)
{
	HLSLsource[0] = d912pxy_s.vfs.ReadFile(d912pxy_vfs_path(desc->ref.VS->GetID(), d912pxy_vfs_bid::shader_sources));
	HLSLsource[1] = d912pxy_s.vfs.ReadFile(d912pxy_vfs_path(desc->ref.PS->GetID(), d912pxy_vfs_bid::shader_sources));

	if (HLSLsource[0].isNullptr())
		HLSLsource[0] = desc->ref.VS->GetHLSLSource();

	if (HLSLsource[1].isNullptr())
		HLSLsource[1] = desc->ref.PS->GetHLSLSource();
		
	if (HLSLsource[0].isNullptr() || HLSLsource[1].isNullptr())
	{
		LOG_ERR_DTDM("No HLSL source available to perfrom PSO RCE for alias %S (VS %016llX PS %016llX)", alias, desc->ref.VS->GetID(), desc->ref.PS->GetID());
		return false;
	}

	//megai2: pass 0 - vdecl to vs input signature typecheck
	LOG_DBG_DTDM("PSO RCE P0");

	RCEUpdateVSInputByVDecl(HLSLsource[0].c_arr<char>(), fullDesc);

	//megai2: pass 1 - vs output to ps input signature ordering check
	LOG_DBG_DTDM("PSO RCE P1");

	char* vsOut[256] = { NULL };
	char* psIn[256] = { NULL };

	UINT vsOutCnt = 0;
	UINT psInCnt = 0;

	RCELoadIOBlock(HLSLsource[0].c_arr<char>(), "VS_OUTPUT", vsOut, vsOutCnt);
	RCELoadIOBlock(HLSLsource[1].c_arr<char>(), "PS_INPUT", psIn, psInCnt);

	//filter ps unused regs
	RCEFilterUnusedRegs(psIn, psInCnt);

	//find inputs in outputs and reorder last one to input sequence
	RCEFixIOBlocksOrdering(vsOut, psIn, vsOutCnt, psInCnt);

	//write declaration back to VS
	RCEUpdateIOBlock(HLSLsource[0].c_arr<char>(), "VS_OUTPUT", vsOut, vsOutCnt);
	//write declaration back to PS due to unused reg filtering
	RCEUpdateIOBlock(HLSLsource[1].c_arr<char>(), "PS_INPUT", psIn, psInCnt);

	//pass 2 - change tex2d lookup to pcf lookup if needed
	if (desc->val.compareSamplerStage != d912pxy_trimmed_pso_desc::NO_COMPARE_SAMPLERS)
		RCEApplyPCFSampler(HLSLsource[1].c_arr<char>(), desc->val.compareSamplerStage);

	return true;
}

void d912pxy_pso_item::RCELoadIOBlock(char* source, const char* marker, char** out, UINT& outCnt)
{
	char* sdeclLine = strstr(source, marker);
	char* structDclEmt = d912pxy_helper::StrNextLine(sdeclLine);

	structDclEmt = d912pxy_helper::StrNextLine(structDclEmt);

	while (structDclEmt[0] != '}')
	{
		char* lnStart = structDclEmt;
		structDclEmt = d912pxy_helper::StrNextLine(structDclEmt);

		intptr_t lSz = (intptr_t)structDclEmt - (intptr_t)lnStart;

		PXY_MALLOC(out[outCnt], lSz + 1, char*);

		memcpy(out[outCnt], lnStart, lSz);
		out[outCnt][lSz] = 0;
		++outCnt;
	}
}

void d912pxy_pso_item::RCEUpdateVSInputByVDecl(char* source, D3D12_GRAPHICS_PIPELINE_STATE_DESC& fullDesc)
{
	for (int i = 0; i != fullDesc.InputLayout.NumElements; ++i)
	{
		char* semDefPlace = strstr(source, fullDesc.InputLayout.pInputElementDescs[i].SemanticName);

		if (!semDefPlace)
		{
			LOG_DBG_DTDM("semantic %S not used in vs", fullDesc.InputLayout.pInputElementDescs[i].SemanticName);
			continue;
		}

		char* defLine = d912pxy_helper::StrGetCurrentLineStart(semDefPlace);
		char* replPos = strchr(defLine, '4') - 20;

		const char* newType = "/*default*/    float4";

		switch (fullDesc.InputLayout.pInputElementDescs[i].Format)
		{
		case DXGI_FORMAT_R32_FLOAT:
		case DXGI_FORMAT_R32G32_FLOAT:
		case DXGI_FORMAT_R32G32B32_FLOAT:
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
			break;
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_R16G16_UNORM:
		case DXGI_FORMAT_R16G16B16A16_UNORM:
			newType = "/*RCE*/  unorm float4";
			break;
		case DXGI_FORMAT_R16G16_SNORM:
		case DXGI_FORMAT_R16G16B16A16_SNORM:
			newType = "/*RCE*/  snorm float4";
			break;
		case DXGI_FORMAT_R16G16_FLOAT:
		case DXGI_FORMAT_R16G16B16A16_FLOAT:
			newType = "/*RCE*/         half4";
			break;
		case DXGI_FORMAT_R16G16_SINT:
		case DXGI_FORMAT_R16G16B16A16_SINT:
			newType = "/*RCE*/          int4";
			break;
		case DXGI_FORMAT_R8G8B8A8_UINT:
			newType = "/*RCE*/         uint4";
			break;
		default:
			newType = "/*RCE unk*/    float4";
			break;
		}

		memcpy(replPos, newType, 21);
	}
}

void d912pxy_pso_item::RCEUpdateIOBlock(char* source, const char* marker, char** data, UINT elements)
{
	char* sdeclLine = strstr(source, marker);
	char* structDclEmt = d912pxy_helper::StrNextLine(sdeclLine);
	structDclEmt = d912pxy_helper::StrNextLine(structDclEmt);

	for (int j = 0; j != elements; ++j)
	{
		size_t tStrLen = strlen(data[j]);
		memcpy(structDclEmt, data[j], tStrLen);
		structDclEmt += tStrLen;
		PXY_FREE(data[j]);
	}
}

void d912pxy_pso_item::RCEFilterUnusedRegs(char** ioBlock, UINT elements)
{
	int filterTgt = elements - 1;

	for (int i = 0; i != elements; ++i)
	{
		while (strstr(ioBlock[i], "unused_ireg_"))
		{
			if (filterTgt >= i)
				break;

			char* tSwp = ioBlock[i];
			ioBlock[i] = ioBlock[filterTgt];
			ioBlock[filterTgt] = tSwp;
			--filterTgt;
		}
	}
}

void d912pxy_pso_item::RCEFixIOBlocksOrdering(char** vsOut, char** psIn, UINT vsOutCnt, UINT psInCnt)
{
	for (int i = 0; i != psInCnt; ++i)
	{
		char* inputSemantic = strstr(psIn[i], ": ") + 2;

		for (int j = 0; j != vsOutCnt; ++j)
		{
			if (strstr(vsOut[j], inputSemantic))
			{
				char* strSwp = vsOut[i];
				vsOut[i] = vsOut[j];
				vsOut[j] = strSwp;
			}
		}
	}
}

bool d912pxy_pso_item::RCELinkDerivedCSO(d912pxy_mem_block* src, char* alias)
{
	//megai2: link alias with derived CSO
	UINT64 derivedUID[2] = {
		d912pxy::Hash64(d912pxy::MemoryArea(src[0].ptr(), src[0].size())).value,
		d912pxy::Hash64(d912pxy::MemoryArea(src[1].ptr(), src[1].size())).value
	};

	char buf[255];
	sprintf(buf, "%016llX_%016llX", derivedUID[0], derivedUID[1]);
	d912pxy_mem_block ret = d912pxy_mem_block::from(buf, strlen(buf) + 1);
	derivedName = ret.c_arr<char>();
	d912pxy_s.vfs.WriteFile(d912pxy_vfs_path(alias, d912pxy_vfs_bid::derived_cso_refs), ret);

	return RCEIsDerivedPresent(derivedName);
}

bool d912pxy_pso_item::RCECompileDerivedCSO(d912pxy_mem_block* src, char* derivedName)
{
	auto replVS = d912pxy_shader_replacer(0, 0, desc->ref.VS->GetID(), 1);
	auto replPS = d912pxy_shader_replacer(0, 0, desc->ref.PS->GetID(), 0);

	d912pxy_shader_code bcVS = replVS.CompileFromHLSL_MEM(d912pxy_helper::GetFilePath(FP_SHADER_DB_HLSL_DIR)->w, src[0].ptr(), (UINT)src[0].size(), 0);
	d912pxy_shader_code bcPS = replPS.CompileFromHLSL_MEM(d912pxy_helper::GetFilePath(FP_SHADER_DB_HLSL_DIR)->w, src[1].ptr(), (UINT)src[1].size(), 0);

	if ((!bcVS.blob) || (!bcPS.blob))
	{
		LOG_ERR_DTDM("RCE derived code %S failed to compile", derivedName);
		return false;
	}
	else {
		d912pxy_s.vfs.WriteFile(d912pxy_vfs_path(derivedName, d912pxy_vfs_bid::derived_cso_vs), d912pxy_mem_block::use(bcVS.code, bcVS.sz));
		d912pxy_s.vfs.WriteFile(d912pxy_vfs_path(derivedName, d912pxy_vfs_bid::derived_cso_ps), d912pxy_mem_block::use(bcPS.code, bcPS.sz));
	}

	return true;
}

bool d912pxy_pso_item::RCEIsDerivedPresent(char* derivedName)
{
	return (
		d912pxy_s.vfs.IsFilePresent(d912pxy_vfs_path(derivedName, d912pxy_vfs_bid::derived_cso_ps)) &&
		d912pxy_s.vfs.IsFilePresent(d912pxy_vfs_path(derivedName, d912pxy_vfs_bid::derived_cso_vs))
		);
}

void d912pxy_pso_item::RCEApplyPCFSampler(char* source, UINT stage)
{
	char buf[256];
	sprintf_s(buf, "%u_deftype tex2d", stage);
	char* targetSamplerDef = strstr(source, buf);

	if (targetSamplerDef)
	{
		sprintf_s(buf, "%u_deftype depth/*M*/", stage);
		memcpy(targetSamplerDef, buf, strlen(buf));
	}
}

void d912pxy_pso_item::AfterCompileRelease()
{
	delete dx12Desc;

	desc->HoldRefs(false);
	delete desc;

	Release();

	--itemsInCompile;
}

bool d912pxy_pso_item::ValidateFullDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC& fullDesc)
{
	if (!fullDesc.VS.pShaderBytecode || !fullDesc.PS.pShaderBytecode)
	{
		LOG_ERR_DTDM("Can't compile pso with shader pair VS %016llX PS %016llX", desc->ref.VS->GetID(), desc->ref.PS->GetID());
		return false;
	}

	return true;
}

ID3D12PipelineState* d912pxy_pso_item::GetPtr()
{
	return psoPtr;
}

size_t d912pxy_pso_item::GetTotalPendingItems()
{
	return itemsInCompile.load();
}

void d912pxy_pso_item::RealtimeIntegrityCheck(D3D12_GRAPHICS_PIPELINE_STATE_DESC& fullDesc)
{
	d912pxy_shader_pair_hash_type pairUID = desc->GetShaderPairUID();
	d912pxy_trimmed_pso_desc::StorageKey psoKey(desc->GetValuePart());	
	sprintf(derivedAlias, "%016llX_%08lX", pairUID, psoKey.val.value);
	LOG_DBG_DTDM("DX9 PSO realtime check emulation for alias %s", derivedAlias);

	derivedName = GetDerivedNameByAlias(derivedAlias);
	
	if (derivedName && RCEIsDerivedPresent(derivedName))		
		return;//megai2: both derived cso files are present, just load them to pso and compile on dx12 side

	if (derivedName)
	{
		PXY_FREE(derivedName);
		derivedName = nullptr;
	}

	fallbacktoNonDerived = !PerformRCE(derivedAlias, fullDesc);
}

d912pxy_pso_item::d912pxy_pso_item(d912pxy_trimmed_pso_desc* inDesc) : d912pxy_comhandler(PXY_COM_OBJ_PSO_ITEM, L"PSO item"),
	psoPtr(nullptr)
{
	desc = new d912pxy_trimmed_pso_desc(*inDesc);
}

void d912pxy_pso_item::MT_DerivedCompile()
{
	d912pxy_s.render.db.psoMTCompiler.queueCompileDXC(this);
}

void d912pxy_pso_item::MT_PSOCompile()
{
	d912pxy_s.render.db.psoMTCompiler.queueCompilePSO(this);
}
