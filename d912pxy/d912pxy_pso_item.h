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

class d912pxy_pso_item : public d912pxy_vtable, public d912pxy_comhandler
{

public:
	static d912pxy_pso_item* d912pxy_pso_item_com(d912pxy_trimmed_pso_desc* sDsc);
	~d912pxy_pso_item();

	void Compile();
	void MarkPushedToCompile();

	bool RetryDerivedPresence();
	void DerivedCompile();
	void PSOCompile();

	char* GetDerivedName() { return derivedName; }

	ID3D12PipelineState* GetPtr();

	static size_t GetTotalPendingItems();

private:	
	void CreatePSO(D3D12_GRAPHICS_PIPELINE_STATE_DESC& fullDesc);
	void CreatePSODerived(char* alias, D3D12_GRAPHICS_PIPELINE_STATE_DESC& fullDesc);
	void RealtimeIntegrityCheck(D3D12_GRAPHICS_PIPELINE_STATE_DESC& fullDesc);

	char* GetDerivedNameByAlias(char* alias);	
	bool RCELinkDerivedCSO(d912pxy_mem_block* src, char* alias);
	bool RCECompileDerivedCSO(d912pxy_mem_block* src, char* derivedName);
	bool RCEIsDerivedPresent(char* derivedName);

	//RCE part that work with HLSL code
	bool PerformRCE(char* alias, D3D12_GRAPHICS_PIPELINE_STATE_DESC& fullDesc);
	void RCELoadIOBlock(char* source, const char* marker, char** out, UINT& outCnt);
	void RCEUpdateVSInputByVDecl(char* source, D3D12_GRAPHICS_PIPELINE_STATE_DESC& fullDesc);
	void RCEUpdateIOBlock(char* source, const char* marker, char** data, UINT elements);
	void RCEFilterUnusedRegs(char** ioBlock, UINT elements);
	void RCEFixIOBlocksOrdering(char** vsOut, char** psIn, UINT vsOutCnt, UINT psInCnt);
	void RCEApplyPCFSampler(char* source, UINT stage);

	void AfterCompileRelease();

	bool ValidateFullDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC& fullDesc);

	d912pxy_pso_item(d912pxy_trimmed_pso_desc* inDesc);

	//MT transfers
	void MT_DerivedCompile();
	void MT_PSOCompile();
	
	std::atomic<ID3D12PipelineState*> psoPtr;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC* dx12Desc;
	d912pxy_trimmed_pso_desc* desc;
	d912pxy_mem_block HLSLsource[2];
	char derivedAlias[255];
	char* derivedName = nullptr;

	bool fallbacktoNonDerived = false;

	static std::atomic<size_t> itemsInCompile;
};