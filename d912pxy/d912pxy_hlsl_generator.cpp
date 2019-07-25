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

struct ID3DXBuffer : public IUnknown {
	virtual LPVOID STDMETHODCALLTYPE GetBufferPointer(void) = 0;
	virtual DWORD  STDMETHODCALLTYPE GetBufferSize(void) = 0;
};

typedef HRESULT
(WINAPI *PD3DXDISASSEMBLESHADER)(
	CONST DWORD *pShader,
	BOOL EnableColorCode,
	LPCSTR pComments,
	ID3DXBuffer **ppDisassembly
	);

typedef HRESULT 
(WINAPI *PD3DXAssembleShaderFromFile) (
	_In_        LPCTSTR       pSrcFile,
	_In_		void* pDefines,
	_In_        void* pInclude,
	_In_        DWORD         Flags,
	_Out_       ID3DXBuffer** ppShader,
	_Out_       ID3DXBuffer** ppErrorMsgs
);

HRESULT
AssembleShader(const wchar_t* file)
{
	static BOOL firsttime = TRUE;

	/*
	* TODO: Consider using d3dcompile_xx.dll per
	* http://msdn.microsoft.com/en-us/library/windows/desktop/ee663275.aspx
	*/

	static HMODULE hD3DXModule = NULL;
	static PD3DXAssembleShaderFromFile pfnD3DXAssembleShaderFromFile = NULL;

	if (firsttime) {
		if (!hD3DXModule) {
			unsigned release;
			int version;
			for (release = 0; release <= 1; ++release) {
				/* Version 41 corresponds to Mar 2009 version of DirectX Runtime / SDK */
				for (version = 41; version >= 0; --version) {
					char filename[256];
					_snprintf(filename, sizeof(filename),
						"d3dx9%s%s%u.dll", release ? "" : "d", version ? "_" : "", version);
					hD3DXModule = LoadLibraryA(filename);
					if (hD3DXModule)
						goto found;
				}
			}
		found:
			;
		}

		if (hD3DXModule) {
			if (!pfnD3DXAssembleShaderFromFile) {
				pfnD3DXAssembleShaderFromFile = (PD3DXAssembleShaderFromFile)GetProcAddress(hD3DXModule, "D3DXAssembleShaderFromFileW");
			}
		}

		firsttime = FALSE;
	}

	ID3DXBuffer* pAssembly;
	ID3DXBuffer** ppAssembly = &pAssembly;

	HRESULT hr = E_FAIL;
	if (pfnD3DXAssembleShaderFromFile) {
		hr = pfnD3DXAssembleShaderFromFile(file, NULL, NULL, 0, reinterpret_cast<ID3DXBuffer **>(ppAssembly), NULL);

		int len = pAssembly->GetBufferSize();

		FILE* f = fopen("tmp.dxbc", "wb");

		fwrite(pAssembly->GetBufferPointer(), len, 1, f);

		fflush(f);
		fclose(f);

		pAssembly->Release();
	}
	return hr;
}


HRESULT
DisassembleShader(const DWORD *tokens, ID3DXBuffer**ppDisassembly)
{
	static BOOL firsttime = TRUE;

	/*
	* TODO: Consider using d3dcompile_xx.dll per
	* http://msdn.microsoft.com/en-us/library/windows/desktop/ee663275.aspx
	*/

	static HMODULE hD3DXModule = NULL;
	static PD3DXDISASSEMBLESHADER pfnD3DXDisassembleShader = NULL;

	if (firsttime) {
		if (!hD3DXModule) {
			unsigned release;
			int version;
			for (release = 0; release <= 1; ++release) {
				/* Version 41 corresponds to Mar 2009 version of DirectX Runtime / SDK */
				for (version = 41; version >= 0; --version) {
					char filename[256];
					_snprintf(filename, sizeof(filename),
						"d3dx9%s%s%u.dll", release ? "" : "d", version ? "_" : "", version);
					hD3DXModule = LoadLibraryA(filename);
					if (hD3DXModule)
						goto found;
				}
			}
		found:
			;
		}

		if (hD3DXModule) {
			if (!pfnD3DXDisassembleShader) {
				pfnD3DXDisassembleShader = (PD3DXDISASSEMBLESHADER)GetProcAddress(hD3DXModule, "D3DXDisassembleShader");
			}
		}

		firsttime = FALSE;
	}

	HRESULT hr = E_FAIL;
	if (pfnD3DXDisassembleShader) {
		hr = pfnD3DXDisassembleShader(tokens, FALSE, NULL,
			reinterpret_cast<ID3DXBuffer **>(ppDisassembly));
	}
	return hr;
}

const char* d912pxy_hlsl_generator_reg_names[20] = {
	"reg_t", // Temporary Register File
	"reg_inp", // Input Register File
	"reg_const", // Constant Register File
	"reg_at",
	"reg_rastout",
	"reg_attrout",
	"reg_tc_vo",
	"reg_iconst",
	"reg_clr_out",
	"reg_depth_out",
	"reg_sampler",
	"reg_const2",
	"reg_const3",
	"reg_const4",
	"reg_constb",
	"reg_loopc",
	"reg_t16",
	"reg_misc",
	"reg_lab",
	"reg_pred"
};

const char* d912pxy_hlsl_generator_reg_names_proc_ps[20] = {
	"reg_t", // Temporary Register File
	"inp.reg_inp", // Input Register File
	"reg_const", // Constant Register File
	"inp.reg_at",
	"ret.reg_rastout",
	"reg_attrout",
	"ret.reg_tc_vo",
	"reg_iconst",
	"ret.reg_clr_out",
	"reg_depth_out",
	"reg_sampler",
	"reg_const2",
	"reg_const3",
	"reg_const4",
	"reg_constb",
	"reg_loopc",
	"reg_t16",
	"reg_misc",
	"reg_lab",
	"reg_pred"
};

const char* d912pxy_hlsl_generator_reg_names_proc_vs[20] = {
	"reg_t", // Temporary Register File
	"inp.reg_inp", // Input Register File
	"reg_const", // Constant Register File
	"reg_at",
	"ret.reg_rastout",
	"reg_attrout",
	"ret.reg_tc_vo",
	"reg_iconst",
	"ret.reg_clr_out",
	"reg_depth_out",
	"reg_sampler",
	"reg_const2",
	"reg_const3",
	"reg_const4",
	"reg_constb",
	"reg_loopc",
	"reg_t16",
	"reg_misc",
	"reg_lab",
	"reg_pred"
};

d912pxy_hlsl_generator_sio_handler d912pxy_hlsl_generator::SIOhandlers[d912pxy_hlsl_generator_op_handler_group_size*d912pxy_hlsl_generator_op_handler_cnt] = { 0 };

UINT d912pxy_hlsl_generator::allowPP_suffix = 0;
UINT32 d912pxy_hlsl_generator::NaNguard_flag = 0;
UINT32 d912pxy_hlsl_generator::sRGB_alphatest_bits = 0;

d912pxy_hlsl_generator::d912pxy_hlsl_generator(DWORD * src, UINT len, wchar_t * ofn, d912pxy_shader_uid uid) : d912pxy_noncom(L"hlsl generator")
{
	oCode = src;
	oLen = len;
	PSpositionUsed = 0;
	procIdent = 0;
	mUID = uid;

	LoadGenProfile();

#ifdef _DEBUG
	LOG_DBG_DTDM("generating hlsl file %s", ofn);
	of = _wfopen(ofn, L"wb");
#endif

	procOffsetPredef = d912pxy_hlsl_generator_proc_predef_offset;
	headerOffsetI = 0;
	headerOffsetO = d912pxy_hlsl_generator_heado_offset;
	procOffset = d912pxy_hlsl_generator_proc_offset;
	ZeroMemory(lines, sizeof(char*) * d912pxy_hlsl_generator_max_code_lines);
	ZeroMemory(regDefined, 8 * ((D3DSPR_PREDICATE + 1) * HLSL_MAX_REG_FILE_LEN));
	ZeroMemory(regDefinedAsC, 8 * ((D3DSPR_PREDICATE + 1) * HLSL_MAX_REG_FILE_LEN));
	relLookupDefined = 0;
}

d912pxy_hlsl_generator::~d912pxy_hlsl_generator()
{
#ifdef _DEBUG
	fflush(of);
	fclose(of);
#endif
}

d912pxy_hlsl_generator_memout* d912pxy_hlsl_generator::Process(UINT toMemory)
{
	if (!of && !toMemory)
		return 0;
		
#ifdef _DEBUG
	DumpDisassembly();
#endif

	UINT sioTableOffset = LoadSMBlock();

	WriteShaderHeadData();
	WriteExtraUnusedRegs();
	
	UINT ocIdx=1;
	while (ocIdx != oLen)
	{
		UINT16 sioID = oCode[ocIdx] & 0xFFFF;

		//skip comments
		if (sioID == 0xFFFE)
		{
			ocIdx += ((oCode[ocIdx] >> 16) & 0xFFFF) + 1;
			continue;
		}
		else if (sioID == 0xFFFF)
		{
			break;
		}

		(this->*SIOhandlers[sioID + sioTableOffset])(&oCode[ocIdx]);

		//[27:24] For pixel and vertex shader versions earlier than 2_0, bits 24 through 27 are reserved and set to 0x0.
		//For pixel and vertex shader versions 2_0 and later, bits 24 through 27 specify the size in DWORDs of the instruction 
		//excluding the instruction token itself(that is, the number of tokens that comprise the instruction excluding the instruction token).

		if (majVer < 2)
			ocIdx += 1 + SM_1_X_SIO_SIZE[sioID];
		else
			ocIdx += 1 + (oCode[ocIdx] >> 24) & 0xF;
	}

	WriteShaderTailData();
	return WriteOutput(toMemory);
}

UINT d912pxy_hlsl_generator::GetRegType(DWORD dst)
{
	return ((dst & D3DSP_REGTYPE_MASK) >> D3DSP_REGTYPE_SHIFT) | ((dst & D3DSP_REGTYPE_MASK2) >> D3DSP_REGTYPE_SHIFT2);
}

UINT d912pxy_hlsl_generator::GetRegNumber(DWORD op)
{
	return op & 0x3FF;
}

UINT d912pxy_hlsl_generator::GetWriteMask(DWORD op)
{
	return (0xF0000 & op) >> 16;
}

D3DSHADER_PARAM_SRCMOD_TYPE d912pxy_hlsl_generator::GetSrcMod(DWORD op)
{
	return (D3DSHADER_PARAM_SRCMOD_TYPE)(op & D3DSP_SRCMOD_MASK);
}

d912pxy_hlsl_generator_regtext d912pxy_hlsl_generator::FormatDstRegister(DWORD* regA)
{
	DWORD reg = regA[1];

	CheckRegDefinition(reg, 1);

	UINT64 writeMask = GetWriteMaskStr(reg);

	d912pxy_hlsl_generator_regtext ret;

	if (reg & 0x2000)
	{
		LOG_ERR_THROW2(-1, "hlsl generator got dst relative addresing, which is not done yet");
	}
	else {
		sprintf(ret.t, "%s%u%s",
			GetRegTypeStr(reg, 1),
			GetRegNumber(reg),
			(char*)&writeMask
		);
	}

	return ret;
}

d912pxy_hlsl_generator_regtext d912pxy_hlsl_generator::FormatSrcRegister(DWORD* regA, UINT8 wm, UINT id, UINT haveDst, UINT allowFmtConvert)
{
	UINT wOfs = 1;

	if (haveDst)
	{
		wOfs = 2;
		if (regA[1] & 0x2000)
			wOfs = 3;
	}

	for (int i = 0; i != id; ++i)
	{
		if (regA[wOfs] & 0x2000)
			wOfs += 2;
		else
			wOfs += 1;
	}

	DWORD reg = regA[wOfs];
	
	CheckRegDefinition(reg, 0);

	UINT64 swizzle = GetSwizzleStr(reg, wm);
		
	d912pxy_hlsl_generator_regtext ret;

	if (reg & 0x2000)
	{
		if (GetRegType(reg) != D3DSPR_CONST)
		{
			LOG_ERR_THROW2(-1, "relative adressing for registers other then constants are not supported yet");
		}

		char constType[2] = "V";
		if (isPS)
			constType[0] = 'P';

		DWORD regAdr = regA[wOfs+1];
		UINT64 swizzleAdr = GetSwizzleStr(regAdr, 1);

		//megai2: base index of relative adressed register is defined in constants
		//so we need to make copy of constants from zero rel. index to max found in code (at least)
		if (IsRegDefined(reg, 0) == 2)
		{	
			UINT baseRelNum = GetRegNumber(reg);
			if (!relLookupDefined)
			{				
				UINT relArrSz = 0;
				for (int i = 0; i != 255 - baseRelNum; ++i)
				{
					if (IsRegDefined(reg, i))
						++relArrSz;
					else
						break;//megai2: stop on first fail for now
				}

				HLSL_GEN_WRITE_PROC("float4 reg_consts_rel[%u] = { ", relArrSz);

				for (int i = 0; i != relArrSz; ++i)
				{
					if (i + 1 == relArrSz)
						HLSL_GEN_WRITE_PROC("	%s%u };", GetRegTypeStr(reg, 1), i + baseRelNum);
					else
						HLSL_GEN_WRITE_PROC("	%s%u,", GetRegTypeStr(reg, 1), i + baseRelNum);
				}

				relLookupDefined = baseRelNum;
			}			

			if (relLookupDefined != baseRelNum)
				LOG_ERR_THROW2(-1, "reg_consts_rel go wrong 1");

			switch (GetSrcMod(reg))
			{
			case D3DSPSM_NONE:
				//c23[a0.x].swizzle => getPassedVSFv(23 + a0.x).swizzle
				sprintf(ret.t, "reg_consts_rel[%s%u%s]%s",
					GetRegTypeStr(regAdr, 1),
					GetRegNumber(regAdr),
					(char*)&swizzleAdr,
					(char*)&swizzle
				);
				break;
			case D3DSPSM_NEG:
				sprintf(ret.t, "-reg_consts_rel[%s%u%s]%s",
					GetRegTypeStr(regAdr, 1),
					GetRegNumber(regAdr),
					(char*)&swizzleAdr,
					(char*)&swizzle
				);
				break;
			case D3DSPSM_ABS:
				sprintf(ret.t, "abs(reg_consts_rel[%s%u%s]%s)",
					GetRegTypeStr(regAdr, 1),
					GetRegNumber(regAdr),
					(char*)&swizzleAdr,
					(char*)&swizzle
				);
				break;
			case D3DSPSM_ABSNEG:
				sprintf(ret.t, "-abs(reg_consts_rel[%s%u%s]%s)",
					GetRegTypeStr(regAdr, 1),
					GetRegNumber(regAdr),
					(char*)&swizzleAdr,
					(char*)&swizzle
				);
				break;
			default:
				LOG_ERR_DTDM("hlsl generator not support %08lX src modifier", GetSrcMod(reg));
				LOG_ERR_THROW2(-1, "hlsl generator not support passed src modifier");
				break;
			}
		}
		else {

			switch (GetSrcMod(reg))
			{
			case D3DSPSM_NONE:
				//c23[a0.x].swizzle => getPassedVSFv(23 + a0.x).swizzle
				sprintf(ret.t, "getPassed%sSFv(%u+%s%u%s)%s",
					constType,
					GetRegNumber(reg),
					GetRegTypeStr(regAdr, 1),
					GetRegNumber(regAdr),
					(char*)&swizzleAdr,
					(char*)&swizzle
				);
				break;
			case D3DSPSM_NEG:
				sprintf(ret.t, "-getPassed%sSFv(%u+%s%u%s)%s",
					constType,
					GetRegNumber(reg),
					GetRegTypeStr(regAdr, 1),
					GetRegNumber(regAdr),
					(char*)&swizzleAdr,
					(char*)&swizzle
				);
				break;
			case D3DSPSM_ABS:
				sprintf(ret.t, "abs(getPassed%sSFv(%u+%s%u%s)%s)",
					constType,
					GetRegNumber(reg),
					GetRegTypeStr(regAdr, 1),
					GetRegNumber(regAdr),
					(char*)&swizzleAdr,
					(char*)&swizzle
				);
				break;
			case D3DSPSM_ABSNEG:
				sprintf(ret.t, "-abs(getPassed%sSFv(%u+%s%u%s)%s)",
					constType,
					GetRegNumber(reg),
					GetRegTypeStr(regAdr, 1),
					GetRegNumber(regAdr),
					(char*)&swizzleAdr,
					(char*)&swizzle
				);
				break;
			default:
				LOG_ERR_DTDM("hlsl generator not support %08lX src modifier", GetSrcMod(reg));
				LOG_ERR_THROW2(-1, "hlsl generator not support passed src modifier");
				break;
			}
		}
	}
	else {

		switch (GetSrcMod(reg))
		{
		case D3DSPSM_NONE:
			sprintf(ret.t, "%s%u%s",
				GetRegTypeStr(reg, 1),
				GetRegNumber(reg),
				(char*)&swizzle
			);
			break;
		case D3DSPSM_NEG:
			sprintf(ret.t, "(-%s%u%s)",
				GetRegTypeStr(reg, 1),
				GetRegNumber(reg),
				(char*)&swizzle
			);
			break;
		case D3DSPSM_ABS:
			sprintf(ret.t, "abs(%s%u%s)",
				GetRegTypeStr(reg, 1),
				GetRegNumber(reg),
				(char*)&swizzle
			);
			break;
		case D3DSPSM_ABSNEG:
			sprintf(ret.t, "(-abs(%s%u%s))",
				GetRegTypeStr(reg, 1),
				GetRegNumber(reg),
				(char*)&swizzle
			);
			break;
		default:
			LOG_ERR_DTDM("hlsl generator not support %08lX src modifier", GetSrcMod(reg));
			LOG_ERR_THROW2(-1, "hlsl generator not support passed src modifier");
			break;
		}
	}

	if (haveDst & allowFmtConvert)
		ret = FormatDstModifierForSrc(ret, regA[1], GetDstLenByWriteMask2(wm));

	return ret;
}

d912pxy_hlsl_generator_regtext d912pxy_hlsl_generator::FormatDstModifier(d912pxy_hlsl_generator_regtext statement, DWORD dstOp, UINT8 dstLen)
{
	d912pxy_hlsl_generator_regtext ret;

	switch (GetDstModifier(dstOp))
	{
	case 0x0:
		sprintf(ret.t, "%s", statement.t);
		break;
	case 0x1:
		sprintf(ret.t, "saturate(%s)", statement.t);
		break;
	case 0x2:
		if (dstLen > 1)
			sprintf(ret.t, "(half%u)(%s)", dstLen, statement.t);
		else
			sprintf(ret.t, "(half)(%s)", statement.t);
		break;
	case 0x3:
		if (dstLen > 1)
			sprintf(ret.t, "saturate((half%u)(%s))", dstLen, statement.t);
		else
			sprintf(ret.t, "saturate((half)(%s))", statement.t);
		break;
	default:
		LOG_ERR_DTDM("hlsl generator not support %08lX dst mod", (dstOp >> 20) & 0xF);
		LOG_ERR_THROW2(-1, "hlsl generator not support passed dst modifier");
	}
	return ret;
}

d912pxy_hlsl_generator_regtext d912pxy_hlsl_generator::FormatDstModifierForSrc(d912pxy_hlsl_generator_regtext statement, DWORD dstOp, UINT8 dstLen)
{
	d912pxy_hlsl_generator_regtext ret;

	switch (GetDstModifier(dstOp))
	{
	case 0x0:
	case 0x1:
		sprintf(ret.t, "%s", statement.t);
		break;	
	case 0x3:
	case 0x2:
		if (dstLen > 1)
			sprintf(ret.t, "((half%u)(%s))", dstLen, statement.t);
		else
			sprintf(ret.t, "((half)(%s))", statement.t);
		break;	
	default:
		LOG_ERR_DTDM("hlsl generator not support %08lX dst mod", (dstOp >> 20) & 0xF);
		LOG_ERR_THROW2(-1, "hlsl generator not support passed dst modifier");
	}
	return ret;
}

d912pxy_hlsl_generator_regtext d912pxy_hlsl_generator::FormatRightSide1(DWORD dstOp, const char * pre, const char * post, d912pxy_hlsl_generator_regtext op1, UINT8 dstLen)
{
	d912pxy_hlsl_generator_regtext ret;

	sprintf(ret.t, "%s%s%s", pre, op1.t, post);

	return FormatDstModifier(ret, dstOp, dstLen);
}

d912pxy_hlsl_generator_regtext d912pxy_hlsl_generator::FormatRightSide2(DWORD dstOp, const char * pre, const char * post, const char * mid, d912pxy_hlsl_generator_regtext op1, d912pxy_hlsl_generator_regtext op2, UINT8 dstLen)
{
	d912pxy_hlsl_generator_regtext ret;

	sprintf(ret.t, "%s%s%s%s%s", pre, op1.t, mid, op2.t, post);

	return FormatDstModifier(ret, dstOp, dstLen);
}

d912pxy_hlsl_generator_regtext d912pxy_hlsl_generator::FormatRightSide3(DWORD dstOp, const char * pre, const char * post, const char * mid[2], d912pxy_hlsl_generator_regtext op1, d912pxy_hlsl_generator_regtext op2, d912pxy_hlsl_generator_regtext op3, UINT8 dstLen)
{
	d912pxy_hlsl_generator_regtext ret;

	sprintf(ret.t, "%s%s%s%s%s%s%s", pre, op1.t, mid[0], op2.t, mid[1], op3.t, post);

	return FormatDstModifier(ret, dstOp, dstLen);
}

UINT64 d912pxy_hlsl_generator::FormatCmpString(DWORD op)
{
	UINT64 retV = 0;

	char* ret = (char*)&retV;

	/*D3DSPC_GT = 1, // 0 0 1
	D3DSPC_EQ = 2, // 0 1 0
	D3DSPC_GE = 3, // 0 1 1
	D3DSPC_LT = 4, // 1 0 0
	D3DSPC_NE = 5, // 1 0 1
	D3DSPC_LE = 6, // 1 1 0*/

	switch ((op & D3DSHADER_COMPARISON_MASK) >> D3DSHADER_COMPARISON_SHIFT)
	{
	case 1://GT
		sprintf(ret, " > ");
		break;
	case 2://EQ
		sprintf(ret, " == ");
		break;
	case 3://GE
		sprintf(ret, " >= ");
		break;
	case 4://LT
		sprintf(ret, " < ");
		break;
	case 5://NE
		sprintf(ret, " != ");
		break;
	case 6://LE
		sprintf(ret, " <= ");
		break;
	}

	return retV;
}

UINT d912pxy_hlsl_generator::GetDstModifier(DWORD op)
{
	UINT ret = (op >> 20) & 0xF;

	if (!allowPP_suffix)	
		ret &= 1;	

	return ret;
}

int d912pxy_hlsl_generator::WriteProcLinePredef(const char * fmt, ...)
{
	va_list args;

	char tb[d912pxy_hlsl_generator_max_line_length];

	PXY_MALLOC(lines[procOffsetPredef], d912pxy_hlsl_generator_max_line_length, char*);

	va_start(args, fmt);
	vsprintf(tb, fmt, args);
	va_end(args);

	if (procIdent)
		sprintf(lines[procOffsetPredef], "	%s", tb);
	else 
		sprintf(lines[procOffsetPredef], "%s", tb);

	++procOffsetPredef;

	return procOffsetPredef - 1;
}

void d912pxy_hlsl_generator::WriteProcLine(const char * fmt, ...)
{
	va_list args;

	char tb[d912pxy_hlsl_generator_max_line_length];

	PXY_MALLOC(lines[procOffset], d912pxy_hlsl_generator_max_line_length, char*);

	va_start(args, fmt);
	vsprintf(tb,fmt,args); 
	va_end(args);
	
	sprintf(lines[procOffset], "%*s%s", procIdent*4, "", tb);
	++procOffset;
}

void d912pxy_hlsl_generator::WriteHeadILine(UINT prio, const char * fmt, ...)
{
	va_list args;

	UINT idx = headerOffsetI;

	if (isPS)
	{
		idx += prio;
	}

	if (lines[idx]) {
		PXY_FREE(lines[idx]);
	}

	PXY_MALLOC(lines[idx], d912pxy_hlsl_generator_max_line_length, char*);

	va_start(args, fmt);
	vsprintf(lines[idx], fmt, args);
	va_end(args);

	if (!isPS || prio == 0)
		++headerOffsetI;	
}

void d912pxy_hlsl_generator::WriteHeadOLine(UINT prio, const char * fmt, ...)
{
	va_list args;

	UINT idx = headerOffsetO;

	if (!isPS)
	{
		idx += prio;
	}

	if (lines[idx]) {
		PXY_FREE(lines[idx]);
	}

	PXY_MALLOC(lines[idx], d912pxy_hlsl_generator_max_line_length, char*);

	va_start(args, fmt);
	vsprintf(lines[idx], fmt, args);
	va_end(args);

	if (isPS || prio == 0)
		++headerOffsetO;
}

UINT d912pxy_hlsl_generator::GetDstLenByWriteMask(DWORD op)
{
	UINT wm = GetWriteMask(op);

	return GetDstLenByWriteMask2(wm);
}

UINT d912pxy_hlsl_generator::GetDstLenByWriteMask2(DWORD wmask)
{
	UINT ret = 0;
	for (int i = 0; i != 4; ++i)
	{
		if (wmask & (1 << i))
		{
			++ret;
		}
	}
	return ret;
}

const char * d912pxy_hlsl_generator::GetRegTypeStr(DWORD op, UINT8 proc)
{
	if (proc)
		if (isPS)
			return d912pxy_hlsl_generator_reg_names_proc_ps[GetRegType(op)];
		else
			return d912pxy_hlsl_generator_reg_names_proc_vs[GetRegType(op)];
	else
		return d912pxy_hlsl_generator_reg_names[GetRegType(op)];
}

const char * d912pxy_hlsl_generator::GetUsageString(UINT usage, UINT type)
{
	if (!type)
	{
		switch (usage)
		{
		case D3DDECLUSAGE_POSITION:
			return "POSN";
			break;
		case D3DDECLUSAGE_BLENDWEIGHT:
			return "BLWE";
			break;
		case D3DDECLUSAGE_BLENDINDICES:
			return "BLIN"; 
			break;
		case D3DDECLUSAGE_NORMAL:
			return "NORM";
			break;
		case D3DDECLUSAGE_PSIZE:
			return "PSIZ"; 
			break;
		case D3DDECLUSAGE_TEXCOORD:
			return "TEXC"; 
			break;
		case D3DDECLUSAGE_TANGENT:
			return "TANG";
			break;
		case D3DDECLUSAGE_BINORMAL:
			return "BINO";
			break;
		case D3DDECLUSAGE_TESSFACTOR:
			return "TESF"; 
			break;
		case D3DDECLUSAGE_POSITIONT:
			return "POST"; 
			break;
		case D3DDECLUSAGE_COLOR:
			return "COLR"; 
			break;
		case D3DDECLUSAGE_FOG:
			return "FOGG";
			break;
		case D3DDECLUSAGE_DEPTH:
			return "DEPH";
			break;
		case D3DDECLUSAGE_SAMPLE:
			return "SAPL"; 
			break;
		default:
			LOG_ERR_THROW(-1/*vdecl usage unk*/);
		}
	}
	else {
		switch (usage)
		{
		case D3DDECLUSAGE_POSITION:
			return "POSITION";
			break;
		case D3DDECLUSAGE_BLENDWEIGHT:
			return "BLENDWEIGHT";
			break;
		case D3DDECLUSAGE_BLENDINDICES:
			return "BLENDINDICES";
			break;
		case D3DDECLUSAGE_NORMAL:
			return "NORMAL";
			break;
		case D3DDECLUSAGE_PSIZE:
			return "PSIZE";
			break;
		case D3DDECLUSAGE_TEXCOORD:
			return "TEXCOORD";
			break;
		case D3DDECLUSAGE_TANGENT:
			return "TANGENT";
			break;
		case D3DDECLUSAGE_BINORMAL:
			return "BINORMAL";
			break;
		case D3DDECLUSAGE_TESSFACTOR:
			return "TESSFACTOR";
			break;
		case D3DDECLUSAGE_POSITIONT:
			return "POSITIONT";
			break;
		case D3DDECLUSAGE_COLOR:
			return "COLOR";
			break;
		case D3DDECLUSAGE_FOG:
			return "FOG";
			break;
		case D3DDECLUSAGE_DEPTH:
			return "DEPTH";
			break;
		case D3DDECLUSAGE_SAMPLE:
			return "SAMPLE";
			break;
		default:
			LOG_ERR_THROW(-1/*vdecl usage unk*/);
		}
	}
	return "ERRUSAGE";
}

UINT64 d912pxy_hlsl_generator::GetWriteMaskStr(DWORD op)
{
	UINT64 retV;
	char* ret = (char*)&retV;

	ret[0] = '.';
	int wp = 1;
	UINT wm = GetWriteMask(op);

	if (wm == 0xF)	
		return 0;

	char maskCh[4] = { 'x', 'y', 'z', 'w' };
	
	for (int i = 0; i != 4; ++i)
	{
		if ((1 << i) & wm)
		{
			ret[wp] = maskCh[i];
			++wp;
		}
	}
	ret[wp] = 0;

	return retV;
}

UINT64 d912pxy_hlsl_generator::GetSwizzleStr(DWORD opSrc, DWORD wm)
{
	UINT64 retV;
	char* ret = (char*)&retV;

	char swCh[4] = {
		(char)((opSrc >> 16) & 0x3),//x
		(char)((opSrc >> 18) & 0x3),//y
		(char)((opSrc >> 20) & 0x3),//z
		(char)((opSrc >> 22) & 0x3)//w
	};

	ret[0] = '.';
	int wp = 1;

	for (int i = 0; i != 4; ++i)
	{		
		if (!((1 << i) & wm))
			continue;
		
		switch (swCh[i])
		{
			case 0: //x
				swCh[i] = 'x';
				break;
			case 1: //y
				swCh[i] = 'y';
				break;
			case 2: //z
				swCh[i] = 'z';
				break;
			case 3: //w
				swCh[i] = 'w';
				break;
		}

		ret[wp] = swCh[i];
		++wp;
	}
	ret[wp] = 0;

	//megai2: means ".xyzw", so we can skip this safely
	if ((retV & 0x000000777a79782e) == 0x000000777a79782e)
	{
		return 0;
	}

	return retV;
}

void d912pxy_hlsl_generator::CheckRegDefinition(DWORD op, UINT isDst)
{
	UINT type = GetRegType(op);
	UINT num = GetRegNumber(op);

	UINT64 ai = type * (D3DSPR_PREDICATE + 1) + (num >> 6);
	UINT64 rm = 1ULL << (num & 0x3F);

	//HLSL_GEN_WRITE_PROC("//check reg %u - %u on %u - %016llX by %016llX", type, num, ai, rm, regDefined[ai]);
	if ((regDefined[ai] & rm) == 0ULL)
	{
		switch (type)
		{
			case D3DSPR_CONST:
			{
				if (isPS)
				{		
					HLSL_GEN_WRITE_PROC_PD("float4 %s%u = getPassedPSFv(%u);",
						GetRegTypeStr(op, 1),
						num, 
						num
					);
				}
				else {
					HLSL_GEN_WRITE_PROC_PD("float4 %s%u = getPassedVSFv(%u);",
						GetRegTypeStr(op, 1),
						num,
						num
					);
				}
				break;
			}
			case D3DSPR_CONSTINT:
			{
				HLSL_GEN_WRITE_PROC_PD("uint4 %s%u = { 0, 0, 0, 0 };",
					GetRegTypeStr(op, 1),
					num
				);
				break;
			}
			case D3DSPR_COLOROUT:
			{
				HLSL_GEN_WRITE_HEADO(0, "	float4 %s%u: SV_TARGET;",
					GetRegTypeStr(op, 0),
					num
				);
				HLSL_GEN_WRITE_PROC_PD("#define dx9_ret_color_reg_ac %s%u",
					GetRegTypeStr(op, 1),
					num
				);
				/*HLSL_GEN_WRITE_PROC_PD("%s%u = float4(1,1,1,1);",
					GetRegTypeStr(op, 1),
					num
				);*/
				break;
			}
			case D3DSPR_TEXCRDOUT:
			{
				if (majVer <= 2)
				{
					HLSL_GEN_WRITE_HEADO(HLSL_HIO_PRIORITY(HLSL_HIO_PRIOG_TEXC, num), "	float4 %s%u: TEXCOORD%u;",
						GetRegTypeStr(op, 0),
						num,
						num
					);
				}
				break;
			}
			case D3DSPR_RASTOUT:
			{
				if (majVer <= 2)
				{
					HLSL_GEN_WRITE_HEADO(HLSL_HIO_PRIORITY(HLSL_HIO_PRIOG_POS, 0),"	float4 %s%u: SV_POSITION;",
						GetRegTypeStr(op, 0),
						num
					);
					HLSL_GEN_WRITE_PROC_PD("#define dx9_halfpixel_pos_reg_ac %s%u", 
						GetRegTypeStr(op, 1),
						num
					);
				}
				break;
			}
			default:
				if (isDst)
				{
					switch (GetDstModifier(op))
					{
						case 0x0:
						case 0x1:
						{
							HLSL_GEN_WRITE_PROC_PD("float4 %s%u = { 0, 0, 0, 0 };",
								GetRegTypeStr(op, 1),
								num
							);
						}
						break;
						case 0x3:
						case 0x2:
						{
							HLSL_GEN_WRITE_PROC_PD("half4 %s%u = { 0, 0, 0, 0 };",
								GetRegTypeStr(op, 1),
								num
							);
						}
						break;
						default:
							LOG_ERR_DTDM("hlsl generator not support %08lX dst mod", GetDstModifier(op));
							LOG_ERR_THROW2(-1, "hlsl generator not support passed dst modifier");						
					}
				}
				else {
					HLSL_GEN_WRITE_PROC_PD("float4 %s%u = { 0, 0, 0, 0 };",
						GetRegTypeStr(op, 1),
						num
					);
				}
				break;
		}
		regDefined[ai] |= rm;		
	}
}

void d912pxy_hlsl_generator::DefineIOReg(DWORD op, UINT asConstant)
{
	UINT type = GetRegType(op);
	UINT num = GetRegNumber(op);

	UINT ai = type * (D3DSPR_PREDICATE + 1) + (num >> 6);
	UINT64 rm = 1ULL << (num & 0x3F);
	regDefined[ai] |= rm;
	regDefinedAsC[ai] |= (rm * asConstant);
}

int d912pxy_hlsl_generator::IsRegDefined(DWORD op, UINT numOffset)
{
	UINT type = GetRegType(op);
	UINT num = GetRegNumber(op) + numOffset;

	UINT ai = type * (D3DSPR_PREDICATE + 1) + (num >> 6);
	UINT64 rm = 1ULL << (num & 0x3F);
	return ((regDefined[ai] & rm) != 0) * (1 + ((regDefinedAsC[ai] & rm) != 0));
}

void d912pxy_hlsl_generator::LoadGenProfile()
{
	UINT32 sz;
	UINT32* data = (UINT32*)d912pxy_s.vfs.LoadFileH(mUID, &sz, PXY_VFS_BID_SHADER_PROFILE);

	if (data)
	{
		for (int i = 0; i != PXY_INNER_SHDR_BUG_COUNT; ++i)
		{
			UINT32 bva = data[i];
			if (!bva)
				continue;

			genProfile[i] = bva;
		}

		PXY_FREE(data);
	}
	else {
		ZeroMemory(genProfile, sizeof(UINT)*PXY_INNER_SHDR_BUG_COUNT);
	}

	genProfile[PXY_INNER_SHDR_BUG_SRGB_READ] |= ((sRGB_alphatest_bits & PXY_SDB_HLSL_SRGB_ALPHATEST_FORCE_SRGBR) != 0);
	genProfile[PXY_INNER_SHDR_BUG_ALPHA_TEST] |= ((sRGB_alphatest_bits & PXY_SDB_HLSL_SRGB_ALPHATEST_FORCE_ALPHATEST) != 0);
	genProfile[PXY_INNER_SHDR_BUG_SRGB_WRITE] |= ((sRGB_alphatest_bits & PXY_SDB_HLSL_SRGB_ALPHATEST_FORCE_SRGBW) != 0);
}

void d912pxy_hlsl_generator::ProcSIO_DOTX(DWORD * op, UINT sz)
{
	UINT64 writeMask = GetWriteMaskStr(op[1]);

	UINT32 srcComps = (1 << sz)-1;

	d912pxy_hlsl_generator_regtext sDst = FormatDstRegister(op);
	d912pxy_hlsl_generator_regtext sSrc1 = FormatSrcRegister(op, srcComps, 0, 1, 1);
	d912pxy_hlsl_generator_regtext sSrc2 = FormatSrcRegister(op, srcComps, 1, 1, 1);
	d912pxy_hlsl_generator_regtext rSide = FormatRightSide2(op[1], "dot(", ")", ",", sSrc1, sSrc2, 1);

	HLSL_GEN_WRITE_PROC("%s = %s;",
		sDst.t,
		rSide.t
	);
}

void d912pxy_hlsl_generator::ProcSIO_3OP(DWORD * op, const char * pre, const char * mid[2], const char * post)
{
	UINT64 writeMask = GetWriteMaskStr(op[1]);

	d912pxy_hlsl_generator_regtext sDst = FormatDstRegister(op);
	d912pxy_hlsl_generator_regtext sSrc1 = FormatSrcRegister(op, GetWriteMask(op[1]),0, 1, 1);
	d912pxy_hlsl_generator_regtext sSrc2 = FormatSrcRegister(op, GetWriteMask(op[1]),1, 1, 1);
	d912pxy_hlsl_generator_regtext sSrc3 = FormatSrcRegister(op, GetWriteMask(op[1]),2, 1, 1);
	d912pxy_hlsl_generator_regtext rSide = FormatRightSide3(op[1], pre, post, mid, sSrc1, sSrc2, sSrc3, GetDstLenByWriteMask(op[1]));

	HLSL_GEN_WRITE_PROC("%s = %s;",
		sDst.t,
		rSide.t
	);
}

void d912pxy_hlsl_generator::ProcSIO_2OP(DWORD * op, const char * pre, const char * mid, const char * post)
{
	UINT64 writeMask = GetWriteMaskStr(op[1]);

	d912pxy_hlsl_generator_regtext sDst = FormatDstRegister(op);
	d912pxy_hlsl_generator_regtext sSrc1 = FormatSrcRegister(op, GetWriteMask(op[1]),0, 1, 1);
	d912pxy_hlsl_generator_regtext sSrc2 = FormatSrcRegister(op, GetWriteMask(op[1]),1, 1, 1);
	d912pxy_hlsl_generator_regtext rSide = FormatRightSide2(op[1], pre, post, mid, sSrc1, sSrc2, GetDstLenByWriteMask(op[1]));

	HLSL_GEN_WRITE_PROC("%s = %s;",
		sDst.t,
		rSide.t
	);
}

void d912pxy_hlsl_generator::ProcSIO_1OP(DWORD * op, const char * pre, const char * post)
{
	UINT64 writeMask = GetWriteMaskStr(op[1]);
	d912pxy_hlsl_generator_regtext sDst = FormatDstRegister(op);
	d912pxy_hlsl_generator_regtext sSrc1 = FormatSrcRegister(op, GetWriteMask(op[1]),0, 1, 1);
	d912pxy_hlsl_generator_regtext rSide = FormatRightSide1(op[1],pre, post, sSrc1, GetDstLenByWriteMask(op[1]));

	HLSL_GEN_WRITE_PROC("%s = %s;",
		sDst.t,
		rSide.t
	);
}

void d912pxy_hlsl_generator::ProcSIO_ADD(DWORD * op)
{
	ProcSIO_2OP(op, "", " + ", "");
}

UINT d912pxy_hlsl_generator::LoadSMBlock()
{
	majVer = (oCode[0] & 0xFF00) >> 8;
	minVer = oCode[0] & 0xFF;
	isPS = (oCode[0] & 0x10000) != 0;


	UINT sioTableOffset = 0;
	if (majVer == 3)
		sioTableOffset = d912pxy_hlsl_generator_op_handler_group_size * d912pxy_hlsl_generator_op_handler_3_x;
	else if (majVer == 2)
		sioTableOffset = d912pxy_hlsl_generator_op_handler_group_size * d912pxy_hlsl_generator_op_handler_2_x;
	else if (majVer == 1)
		sioTableOffset = d912pxy_hlsl_generator_op_handler_group_size * d912pxy_hlsl_generator_op_handler_1_x;
	else {
		LOG_ERR_DTDM("hlsl generator not support %u_%u shader model", majVer, minVer);
		LOG_ERR_THROW2(-1, "hlsl generator not support shader model specified");
	}

	return sioTableOffset;
}

void d912pxy_hlsl_generator::DumpDisassembly()
{
//code for asm verification
	const char* copener = "/*\n";
	const char* ccloser = "\n*/\n";
	fwrite(copener, 3, 1, of);
	ID3DXBuffer * dasm;
	LOG_ERR_THROW2(DisassembleShader(oCode, &dasm), "shader not decompiled");
	fwrite(dasm->GetBufferPointer(), 1, dasm->GetBufferSize()-1,of);
	fwrite(ccloser, 4, 1, of);
	fflush(of);
	dasm->Release();
////////////////////////	
}

d912pxy_hlsl_generator_memout* d912pxy_hlsl_generator::WriteOutput(UINT toMemory)
{
	const char* newLine = "\r\n";
	size_t outputSz = 0;
	intptr_t outputMemPos;

	for (int i = 0; i != d912pxy_hlsl_generator_max_code_lines; ++i)
	{
		if (lines[i] != 0)
			outputSz += strlen(lines[i]) + 2;
	}

	d912pxy_hlsl_generator_memout* ret = NULL;
	PXY_MALLOC(ret, outputSz+4, d912pxy_hlsl_generator_memout*);

	ret->size = (UINT32)outputSz;

	outputMemPos = (intptr_t)(&ret->data[0]);	

	for (int i = 0; i != d912pxy_hlsl_generator_max_code_lines; ++i)
	{
		if (lines[i] != 0)
		{
			size_t linLen = strlen(lines[i]);
			memcpy((void*)outputMemPos, lines[i], linLen);
			outputMemPos += linLen;
			memcpy((void*)outputMemPos, newLine, 2);
			outputMemPos += 2;
			PXY_FREE(lines[i]);
		}
	}

	if (!toMemory)
	{
		fwrite(&ret->data[0], 1, outputSz, of);
		PXY_FREE(ret);
		return 0;
	}
	else {
#ifdef _DEBUG
		fwrite(&ret->data[0], 1, outputSz, of);
#endif

		return ret;
	}
}

void d912pxy_hlsl_generator::ProcSIO_DEF(DWORD * op)
{
	UINT reg = GetRegType(op[1]);
	UINT wm = GetWriteMask(op[1]);
	UINT num = GetRegNumber(op[1]);

	LOG_DBG_DTDM("def regt = %u wm = %X num = %u", reg, wm, num);
	
	if (wm != 0xF)
		LOG_ERR_THROW2(-1, "hlsl gen stuck");

	DefineIOReg(op[1], 1);

	switch (reg)
	{
		case D3DSPR_CONST:
		{
			float fv4[4];

			fv4[0] = *((float*)&op[1 + 1]);
			fv4[1] = *((float*)&op[1 + 2]);
			fv4[2] = *((float*)&op[1 + 3]);
			fv4[3] = *((float*)&op[1 + 4]);

			HLSL_GEN_WRITE_PROC(
				"float4 %s%u = { %.9f , %.9f , %.9f , %.9f };",
				d912pxy_hlsl_generator_reg_names[reg],
				num,
				fv4[0], fv4[1], fv4[2], fv4[3]
			);
		}
		break;
		case D3DSPR_CONSTINT:
		{
			INT32 iv4[4];

			iv4[0] = *((INT32*)&op[1 + 1]);
			iv4[1] = *((INT32*)&op[1 + 2]);
			iv4[2] = *((INT32*)&op[1 + 3]);
			iv4[3] = *((INT32*)&op[1 + 4]);

			HLSL_GEN_WRITE_PROC(
				"int4 %s%u = { %li , %li , %li , %li };",
				d912pxy_hlsl_generator_reg_names[reg],
				num,
				iv4[0], iv4[1], iv4[2], iv4[3]
			);
		}
		break;
		default:
			LOG_ERR_THROW2(-1, "hlsl gen stuck on def sio");
			break;
	}		
}

void d912pxy_hlsl_generator::ProcSIO_DCL(DWORD * op)
{
	//ProcSIO_UNK(op);

	//1 is DCL instruct token
	//2 is DST token

	UINT dstReg = GetRegType(op[2]);
	UINT regNum = GetRegNumber(op[2]);
	
	LOG_DBG_DTDM("DCL of %S%u", d912pxy_hlsl_generator_reg_names[dstReg], regNum);

	DefineIOReg(op[2], 0);
	
	if (isPS)
	{
		if (majVer >= 3)
		{
			switch (dstReg)
			{
				case D3DSPR_SAMPLER:
				{
					D3DSAMPLER_TEXTURE_TYPE texType = (D3DSAMPLER_TEXTURE_TYPE)(D3DSP_TEXTURETYPE_MASK & op[1]);

					UINT texTypeO = texType >> D3DSP_TEXTURETYPE_SHIFT;

					const char* samplerType[] = {
						"unk",
						"unk",
						"tex2d",
						"texCube",
						"texVolume",
						"depth"
					};

					UINT pcfFix = genProfile[PXY_INNER_SHDR_BUG_PCF_SAMPLER];
					if ((pcfFix != 0) && ((pcfFix - 1) == regNum))						
						texTypeO = 5;

					if ((texTypeO == 0) || (texTypeO > 5))
					{
						LOG_ERR_THROW2(-1, "hlsl gen wrong sampler type");
					}

					if (texTypeO == 3)
					{
						HLSL_GEN_WRITE_PROC(
							"TextureCube %s%ut = textureBindsCubed[texState.texture_s%u];",
							d912pxy_hlsl_generator_reg_names[dstReg], regNum, regNum
						);
					}
					else {
						HLSL_GEN_WRITE_PROC(
							"Texture2DArray %s%ut = textureBinds[texState.texture_s%u];",
							d912pxy_hlsl_generator_reg_names[dstReg], regNum, regNum
						);
					}
					HLSL_GEN_WRITE_PROC(
						"sampler %s%us = samplerBinds[texState.sampler_s%u];",
						d912pxy_hlsl_generator_reg_names[dstReg], regNum, regNum
					);
					HLSL_GEN_WRITE_PROC(
						"#define %s%u_deftype %s",
						d912pxy_hlsl_generator_reg_names[dstReg], regNum, samplerType[texTypeO]
					);
					HLSL_GEN_WRITE_PROC(
						"#define %s%u_srgb_flag %u",
						d912pxy_hlsl_generator_reg_names[dstReg], regNum, 1 << regNum
					);
					HLSL_GEN_WRITE_PROC(" ");
				}
				break;
				case D3DSPR_INPUT:
				case D3DSPR_TEXTURE:
				{
					UINT dclId = ((op[1] >> 16) & 0xF);
					UINT usageType = op[1] & 0x1F;
					HLSL_GEN_WRITE_HEADI(
						HLSL_HIO_PRIORITY(HLSL_HIO_PRIOG_FROM_D3DDECLUSAGE[usageType], dclId),
						"	float4 %s%u: %s%u;",
						d912pxy_hlsl_generator_reg_names[dstReg], regNum, GetUsageString(usageType,1), dclId
					);
					
					if ((dclId > 0) && (!d912pxy_pso_cache::allowRealtimeChecks))
					{
						UINT itr = (HLSL_HIO_PRIORITY(HLSL_HIO_PRIOG_FROM_D3DDECLUSAGE[usageType], dclId) + headerOffsetI);
						do
						{
							--itr;
							--dclId;

							if (lines[itr] == 0)
							{
								HLSL_GEN_WRITE_HEADI(
									itr - headerOffsetI,
									"	float4 %s%u_s%u: %s%u;",
									"unused_ireg_", regNum, dclId, GetUsageString(op[1] & 0x1F, 1), dclId
								);
							}							
						} while (dclId != 0);
					}
				}
				break;
				case D3DSPR_MISCTYPE:
				{
					if (regNum == D3DSMO_POSITION)
					{
						HLSL_GEN_WRITE_HEADI(
							HLSL_HIO_PRIORITY(HLSL_HIO_PRIOG_POS, regNum),
							"	float4 %s%u: SV_POSITION;",
							d912pxy_hlsl_generator_reg_names[dstReg], regNum
						);

						HLSL_GEN_WRITE_PROC_PD("float4 %s%u = inp.%s%u;", d912pxy_hlsl_generator_reg_names[dstReg], regNum, d912pxy_hlsl_generator_reg_names[dstReg], regNum);

						if (isPS)
						{
							PSpositionUsed = 1;
							HLSL_GEN_WRITE_PROC("%s%u = %s%u - 0.5f;", 
								GetRegTypeStr(op[2], 1),
								regNum,
								GetRegTypeStr(op[2], 1),
								regNum
							);

							HLSL_GEN_WRITE_PROC("#define ps_ros_reg_ac %s%u",
								GetRegTypeStr(op[2], 1),
								regNum
							);
						}
					}
					else if (regNum == D3DSMO_FACE)
					{
						lines[mainFunctionDeclStrIdx][strlen(lines[mainFunctionDeclStrIdx])-1] = 0;
						strcat(lines[mainFunctionDeclStrIdx], ", bool glob_isFrontFace: SV_IsFrontFace)");

						HLSL_GEN_WRITE_PROC_PD("float4 glob_vecOne = { 1, 1, 1, 1 };");
						HLSL_GEN_WRITE_PROC_PD("float4 glob_vecMinusOne = { -1, -1, -1, -1 };");

						HLSL_GEN_WRITE_PROC_PD("float4 %s%u = glob_isFrontFace ? glob_vecOne : glob_vecMinusOne;", d912pxy_hlsl_generator_reg_names[dstReg], regNum);						
					} else 
						LOG_ERR_THROW2(-1, "hlsl reg type misc unk");
				}
				break;
				default:
					LOG_ERR_DTDM("hlsl gen dcl ps_3 reg type is %u", dstReg);
					LOG_ERR_THROW2(-1, "hlsl reg type");
					break;
			}
		}
		else {
			switch (dstReg)
			{
				case D3DSPR_SAMPLER:
				{
					D3DSAMPLER_TEXTURE_TYPE texType = (D3DSAMPLER_TEXTURE_TYPE)(D3DSP_TEXTURETYPE_MASK & op[1]);

					UINT texTypeO = texType >> D3DSP_TEXTURETYPE_SHIFT;

					const char* samplerType[] = {
						"unk",
						"unk",
						"tex2d",
						"texCube",
						"texVolume",
						"depth"
					};

					UINT pcfFix = genProfile[PXY_INNER_SHDR_BUG_PCF_SAMPLER];
					if ((pcfFix != 0) && ((pcfFix - 1) == regNum))
						texTypeO = 5;

					if ((texTypeO == 0) || (texTypeO > 5))
					{
						LOG_ERR_THROW2(-1, "hlsl gen wrong sampler type");
					}

					if (texTypeO == 3)
					{
						HLSL_GEN_WRITE_PROC(
							"TextureCube %s%ut = textureBindsCubed[texState.texture_s%u];",
							d912pxy_hlsl_generator_reg_names[dstReg], regNum, regNum
						);
					}
					else {
						HLSL_GEN_WRITE_PROC(
							"Texture2DArray %s%ut = textureBinds[texState.texture_s%u];",
							d912pxy_hlsl_generator_reg_names[dstReg], regNum, regNum
						);
					}
					HLSL_GEN_WRITE_PROC(
						"sampler %s%us = samplerBinds[texState.sampler_s%u];",
						d912pxy_hlsl_generator_reg_names[dstReg], regNum, regNum
					);
					HLSL_GEN_WRITE_PROC(
						"#define %s%u_deftype %s",
						d912pxy_hlsl_generator_reg_names[dstReg], regNum, samplerType[texTypeO]
					);
					HLSL_GEN_WRITE_PROC(
						"#define %s%u_srgb_flag %u",
						d912pxy_hlsl_generator_reg_names[dstReg], regNum, 1 << regNum
					);
					HLSL_GEN_WRITE_PROC(" ");
				}
				break;
				case D3DSPR_INPUT:
				{
					HLSL_GEN_WRITE_HEADI(
						HLSL_HIO_PRIORITY(HLSL_HIO_PRIOG_NC, regNum),
						"	float4 %s%u: INPUT_REG%u;",
						d912pxy_hlsl_generator_reg_names[dstReg], regNum, regNum
					);
				}
				break;
				case D3DSPR_TEXTURE:
				{
					HLSL_GEN_WRITE_HEADI(
						HLSL_HIO_PRIORITY(HLSL_HIO_PRIOG_TEXC, regNum),
						"	float4 %s%u: TEXCOORD%u;",
						d912pxy_hlsl_generator_reg_names[dstReg], regNum, regNum
					);
				}
				break;
				case D3DSPR_MISCTYPE:
				{   
					if (regNum == D3DSMO_POSITION)
					{
						HLSL_GEN_WRITE_HEADI(
							HLSL_HIO_PRIORITY(HLSL_HIO_PRIOG_POS, regNum),
							"	float4 %s%u: SV_POSITION;",
							d912pxy_hlsl_generator_reg_names[dstReg], regNum
						);

						HLSL_GEN_WRITE_PROC_PD("float4 %s%u = inp.%s%u;", d912pxy_hlsl_generator_reg_names[dstReg], regNum, d912pxy_hlsl_generator_reg_names[dstReg], regNum);

						if (isPS)
						{
							PSpositionUsed = 1;
							HLSL_GEN_WRITE_PROC("%s%u = %s%u - 0.5f;",
								GetRegTypeStr(op[2], 1),
								regNum,
								GetRegTypeStr(op[2], 1),
								regNum
							);

							HLSL_GEN_WRITE_PROC("#define ps_ros_reg_ac %s%u",
								GetRegTypeStr(op[2], 1),
								regNum
							);
						}
					}
					else if (regNum == D3DSMO_FACE)
					{
						lines[mainFunctionDeclStrIdx][strlen(lines[mainFunctionDeclStrIdx])-1] = 0;
						strcat(lines[mainFunctionDeclStrIdx], ", bool glob_isFrontFace: SV_IsFrontFace)");

						HLSL_GEN_WRITE_PROC_PD("float4 glob_vecOne = { 1, 1, 1, 1 };");
						HLSL_GEN_WRITE_PROC_PD("float4 glob_vecMinusOne = { -1, -1, -1, -1 };");

						HLSL_GEN_WRITE_PROC_PD("float4 %s%u = glob_isFrontFace ? glob_vecOne : glob_vecMinusOne;", d912pxy_hlsl_generator_reg_names[dstReg], regNum);
					}
					else
						LOG_ERR_THROW2(-1, "hlsl reg type misc unk");
				}
				break;
				default:
					LOG_ERR_THROW2(-1, "hlsl reg type");
				break;
			}
		}
	}
	else {
		if (majVer >= 3)
		{
			switch (dstReg)
			{
				case D3DSPR_SAMPLER:
				{
					D3DSAMPLER_TEXTURE_TYPE texType = (D3DSAMPLER_TEXTURE_TYPE)(D3DSP_TEXTURETYPE_MASK & op[1]);

					UINT texTypeO = texType >> D3DSP_TEXTURETYPE_SHIFT;

					const char* samplerType[] = {
						"unk",
						"unk",
						"tex2d",
						"texCube",
						"texVolume",
						"depth"
					};

					UINT pcfFix = genProfile[PXY_INNER_SHDR_BUG_PCF_SAMPLER];
					if ((pcfFix != 0) && ((pcfFix - 1) == (regNum+HLSL_GEN_VTEXTURE_OFFSET)))
						texTypeO = 5;

					if ((texTypeO == 0) || (texTypeO > 5))
					{
						LOG_ERR_THROW2(-1, "hlsl gen wrong sampler type");
					}

					HLSL_GEN_WRITE_PROC(
						"Texture2DArray %s%ut = textureBinds[texState.texture_s%u];",
						d912pxy_hlsl_generator_reg_names[dstReg], regNum, regNum + HLSL_GEN_VTEXTURE_OFFSET
					);
					HLSL_GEN_WRITE_PROC(
						"sampler %s%us = samplerBinds[texState.sampler_s%u];",
						d912pxy_hlsl_generator_reg_names[dstReg], regNum, regNum + HLSL_GEN_VTEXTURE_OFFSET
					);
					HLSL_GEN_WRITE_PROC(
						"#define %s%u_deftype %s",
						d912pxy_hlsl_generator_reg_names[dstReg], regNum, samplerType[texTypeO]
					);
					HLSL_GEN_WRITE_PROC(
						"#define %s%u_srgb_flag %u",
						d912pxy_hlsl_generator_reg_names[dstReg], regNum, 1 << (regNum + HLSL_GEN_VTEXTURE_OFFSET)
					);
					HLSL_GEN_WRITE_PROC(" ");
				}
				break;
				case D3DSPR_OUTPUT:
				{
					if (((op[1] & 0x1F) == D3DDECLUSAGE_POSITION) && ((op[1] >> 16) & 0xF) == 0)
					{
						HLSL_GEN_WRITE_HEADO(
							HLSL_HIO_PRIORITY(HLSL_HIO_PRIOG_POS, 0),
							"	float4 %s%u: SV_POSITION;",
							d912pxy_hlsl_generator_reg_names[dstReg], regNum
						);
						HLSL_GEN_WRITE_PROC_PD("#define dx9_halfpixel_pos_reg_ac %s%u", GetRegTypeStr(op[2], 1), regNum);
					}
					else {
						UINT dclId = (op[1] >> 16) & 0xF;
						UINT usageType = op[1] & 0x1F;
						HLSL_GEN_WRITE_HEADO(							
							HLSL_HIO_PRIORITY(HLSL_HIO_PRIOG_FROM_D3DDECLUSAGE[usageType], dclId),
							"	float4 %s%u: %s%u;",
							d912pxy_hlsl_generator_reg_names[dstReg], regNum, GetUsageString(usageType, 1), dclId
						);

					/*	if (dclId > 0)
						{
							UINT itr = (HLSL_HIO_PRIORITY(HLSL_HIO_PRIOG_FROM_D3DDECLUSAGE[usageType], dclId) + headerOffsetO);
							do
							{
								--itr;
								--dclId;

								if (lines[itr] == 0)
								{
									HLSL_GEN_WRITE_HEADO(
										itr - headerOffsetO,
										"	float4 %s%u_s%u: %s%u;",
										"unused_oreg_", regNum, dclId, GetUsageString(op[1] & 0x1F, 1), dclId
									);
								}
							} while (dclId != 0);
						}*/
					}
				}
				break;
				case D3DSPR_INPUT:
				{
					if ((op[1] & 0x1F) == D3DDECLUSAGE_BLENDINDICES)
					{
						HLSL_GEN_WRITE_HEADI(
							0,
							"	    uint4 %s%u: %s%uE;",
							d912pxy_hlsl_generator_reg_names[dstReg], regNum, GetUsageString(op[1] & 0x1F, 0), (op[1] >> 16) & 0xF
						);
					} else {
						HLSL_GEN_WRITE_HEADI(
							0,
							"	float4 %s%u: %s%uE;",
							d912pxy_hlsl_generator_reg_names[dstReg], regNum, GetUsageString(op[1] & 0x1F, 0), (op[1] >> 16) & 0xF
						);
					}
				}
				break;
				default:
					LOG_ERR_THROW2(-1, "hlsl reg type");
					break;
			}
		}
		else {
			switch (dstReg)
			{
				case D3DSPR_SAMPLER:
				{
					LOG_ERR_THROW2(-1, "no samplers in vertex shaders for sm < 3");
				}
				break;
				case D3DSPR_INPUT:
				{
					HLSL_GEN_WRITE_HEADI(
						0,
						"	float4 %s%u: %s%uE;",
						d912pxy_hlsl_generator_reg_names[dstReg], regNum, GetUsageString(op[1] & 0x1F, 0), (op[1] >> 16) & 0xF
					);
				}
				break;
				default:
					LOG_ERR_THROW2(-1, "hlsl reg type");
					break;
			}
		}
	}
}

//FormatRightSide2(DWORD dstOp, char* pre, char* post, char* mid, d912pxy_hlsl_generator_regtext op1, d912pxy_hlsl_generator_regtext op2)

void d912pxy_hlsl_generator::ProcSIO_DP2(DWORD * op)
{
	ProcSIO_DOTX(op, 2);
}

void d912pxy_hlsl_generator::ProcSIO_DP3(DWORD * op)
{
	ProcSIO_DOTX(op, 3);
}

void d912pxy_hlsl_generator::ProcSIO_DP4(DWORD * op)
{
	ProcSIO_DOTX(op, 4);
}

void d912pxy_hlsl_generator::ProcSIO_TEXLD(DWORD * op)
{
	UINT64 writeMask = GetWriteMaskStr(op[1]);

	d912pxy_hlsl_generator_regtext sDst = FormatDstRegister(op);
	d912pxy_hlsl_generator_regtext sSrc1 = FormatSrcRegister(op, 0xF,0, 1, 1);
	d912pxy_hlsl_generator_regtext sSrc2 = FormatSrcRegister(op, 0xF,1, 1, 0);

	d912pxy_hlsl_generator_regtext pre;
	sprintf(pre.t, "dx9texld(%s_deftype, %s_srgb_flag, ", sSrc2.t, sSrc2.t);

	d912pxy_hlsl_generator_regtext mid;
	sprintf(mid.t, "t, %ss, ", sSrc2.t);
	d912pxy_hlsl_generator_regtext rSide = FormatRightSide2(op[1], pre.t, ")", mid.t, sSrc2, sSrc1, GetDstLenByWriteMask(op[1]));

	HLSL_GEN_WRITE_PROC("%s = %s;",
		sDst.t,
		rSide.t
	);
}

void d912pxy_hlsl_generator::ProcSIO_TEXLDL(DWORD * op)
{
	UINT64 writeMask = GetWriteMaskStr(op[1]);

	d912pxy_hlsl_generator_regtext sDst = FormatDstRegister(op);
	d912pxy_hlsl_generator_regtext sSrc1 = FormatSrcRegister(op, 0xF, 0, 1, 1);
	d912pxy_hlsl_generator_regtext sSrc1W = FormatSrcRegister(op, 0x8, 0, 1, 1);
	d912pxy_hlsl_generator_regtext sSrc2 = FormatSrcRegister(op, 0xF, 1, 1, 0);

	d912pxy_hlsl_generator_regtext pre;
	sprintf(pre.t, "dx9texldl(%s_deftype, %s_srgb_flag, ", sSrc2.t, sSrc2.t);

	const char* mid[2] = { 0, ", " };
	d912pxy_hlsl_generator_regtext mid0;
	sprintf(mid0.t, "t, %ss, ", sSrc2.t);
	mid[0] = mid0.t;
	
	d912pxy_hlsl_generator_regtext rSide = FormatRightSide3(op[1], pre.t, ")", mid, sSrc2, sSrc1, sSrc1W, GetDstLenByWriteMask(op[1]));
	
	/*if (!isPS)
		rSide = FormatRightSide3(op[1], "", ")", mid, sSrc2, sSrc1, sSrc1W, GetDstLenByWriteMask(op[1]));*/

	HLSL_GEN_WRITE_PROC("%s = %s;",
		sDst.t,
		rSide.t
	);
}

void d912pxy_hlsl_generator::ProcSIO_MUL(DWORD * op)
{
	ProcSIO_2OP(op, "", " * ", "");
}

void d912pxy_hlsl_generator::ProcSIO_MAD(DWORD * op)
{
	const char* mid[2] = { " * ", " ) + " };
	ProcSIO_3OP(op, "(", mid, "");
}

void d912pxy_hlsl_generator::ProcSIO_MOV(DWORD * op)
{
	ProcSIO_1OP(op, "", "");
}

void d912pxy_hlsl_generator::ProcSIO_REP(DWORD * op)
{
	d912pxy_hlsl_generator_regtext counter = FormatSrcRegister(op, 1, 0, 0, 1);
	HLSL_GEN_WRITE_PROC(" ");
	HLSL_GEN_WRITE_PROC("{ ");
	++procIdent;
	HLSL_GEN_WRITE_PROC("int loopStartCounter%u = %s;", procIdent, counter.t);
	HLSL_GEN_WRITE_PROC("[loop]");	
	HLSL_GEN_WRITE_PROC("while (loopStartCounter%u > 0)", procIdent);
	HLSL_GEN_WRITE_PROC("{");
	++procIdent;
	HLSL_GEN_WRITE_PROC("--loopStartCounter%u;", procIdent-1);
	HLSL_GEN_WRITE_PROC(" ");
}

void d912pxy_hlsl_generator::ProcSIO_ENDREP(DWORD * op)
{
	--procIdent;
	HLSL_GEN_WRITE_PROC("");
	HLSL_GEN_WRITE_PROC("}");
	--procIdent;
	HLSL_GEN_WRITE_PROC("");
	HLSL_GEN_WRITE_PROC("}");
}

void d912pxy_hlsl_generator::ProcSIO_MAX(DWORD * op)
{
	ProcSIO_2OP(op, "dx9_max(", ", ", ")");
}

void d912pxy_hlsl_generator::ProcSIO_MIN(DWORD * op)
{
	ProcSIO_2OP(op, "dx9_min(", ", ", ")");
}

void d912pxy_hlsl_generator::ProcSIO_RCP(DWORD * op)
{
	if ((NaNguard_flag >> (isPS * PXY_SDB_HLSL_NAN_GUARD_PS_SHIFT)) & PXY_SDB_HLSL_NAN_GUARD_RCP)
		ProcSIO_1OP(op, "dx9_rcp_guarded(", ")");
	else
		ProcSIO_1OP(op, "dx9_rcp(", ")");
}

void d912pxy_hlsl_generator::ProcSIO_CMP(DWORD * op)
{
	const char* mid[2] = { " >= tmpCmpVec) ? ", " : " };

	//megai2: ensure that all regs are defined, to not bug out their defs in subblock

	FormatDstRegister(op);
	FormatSrcRegister(op, 1, 0, 1, 1);
	FormatSrcRegister(op, 1, 1, 1, 1);
	FormatSrcRegister(op, 1, 2, 1, 1);

	HLSL_GEN_WRITE_PROC("{");
	++procIdent;

	UINT wLen = GetDstLenByWriteMask(op[1]);
	switch (wLen)
	{
		case 1:
			HLSL_GEN_WRITE_PROC("float tmpCmpVec = 0;");
			break;
		case 2:
			HLSL_GEN_WRITE_PROC("float2 tmpCmpVec = {0, 0};");
			break;
		case 3:
			HLSL_GEN_WRITE_PROC("float3 tmpCmpVec = {0, 0, 0};");
			break;
		case 4:
			HLSL_GEN_WRITE_PROC("float4 tmpCmpVec = {0, 0, 0, 0};");
			break;	
		default:
			LOG_ERR_THROW2(-1, "hlsl gen cmp write size not in [1,4] range");
			break;
	}

	ProcSIO_3OP(op, "(", mid, "");
	--procIdent;
	HLSL_GEN_WRITE_PROC("}");
}

void d912pxy_hlsl_generator::ProcSIO_DP2ADD(DWORD * op)
{
	UINT64 writeMask = GetWriteMaskStr(op[1]);

	UINT32 srcComps = 3;

	d912pxy_hlsl_generator_regtext sDst = FormatDstRegister(op);
	d912pxy_hlsl_generator_regtext sSrc1 = FormatSrcRegister(op, srcComps, 0, 1,1);
	d912pxy_hlsl_generator_regtext sSrc2 = FormatSrcRegister(op, srcComps, 1, 1,1);
	d912pxy_hlsl_generator_regtext sSrc3 = FormatSrcRegister(op, 1, 2, 1, 1);

	const char* mid[2] = { ", ", ") + " };
	d912pxy_hlsl_generator_regtext rSide = FormatRightSide3(op[1], "dot(", "", mid, sSrc1, sSrc2, sSrc3, 1);

	HLSL_GEN_WRITE_PROC("%s = %s;",
		sDst.t,
		rSide.t
	);
}

void d912pxy_hlsl_generator::ProcSIO_FRC(DWORD * op)
{
	ProcSIO_1OP(op, "dx9_frac(", ")");
}

void d912pxy_hlsl_generator::ProcSIO_POW(DWORD * op)
{
	ProcSIO_2OP(op, "dx9_pow(", ", ", ")");
}

void d912pxy_hlsl_generator::ProcSIO_RSQ(DWORD * op)
{
	if ((NaNguard_flag >> (isPS * PXY_SDB_HLSL_NAN_GUARD_PS_SHIFT)) & PXY_SDB_HLSL_NAN_GUARD_RSQ)
		ProcSIO_1OP(op, "dx9_rsqrt_guarded(", ")");
	else
		ProcSIO_1OP(op, "dx9_rsqrt(", ")");
}



void d912pxy_hlsl_generator::ProcSIO_NRM(DWORD * op)
{
	ProcSIO_1OP(op, "dx9_normalize(", ")");
}

void d912pxy_hlsl_generator::ProcSIO_LOG(DWORD * op)
{
	ProcSIO_1OP(op, "dx9_log(", ")");
}

void d912pxy_hlsl_generator::ProcSIO_EXP(DWORD * op)
{
	ProcSIO_1OP(op, "dx9_exp(", ")");
}

void d912pxy_hlsl_generator::ProcSIO_EXPP(DWORD * op)
{
	ProcSIO_1OP(op, "dx9_expp(", ")");
}

void d912pxy_hlsl_generator::ProcSIO_TEXKILL(DWORD * op)
{
	d912pxy_hlsl_generator_regtext sSrc1 = FormatDstRegister(op);
	
	HLSL_GEN_WRITE_PROC("clip(%s);",
		sSrc1.t
	);
}

void d912pxy_hlsl_generator::ProcSIO_IF(DWORD * op)
{

	UINT64 cmpStr = FormatCmpString(op[0]);
	d912pxy_hlsl_generator_regtext lOp = FormatSrcRegister(op, 1, 0, 0, 1);
	d912pxy_hlsl_generator_regtext rOp = FormatSrcRegister(op, 1, 1, 0, 1);

	HLSL_GEN_WRITE_PROC(" ");
	HLSL_GEN_WRITE_PROC("if (%s%s%s)",
		lOp.t,
		(char*)&cmpStr,
		rOp.t
	);
	HLSL_GEN_WRITE_PROC("{");
	HLSL_GEN_WRITE_PROC(" ");
	++procIdent;
}

void d912pxy_hlsl_generator::ProcSIO_ELSE(DWORD * op)
{
	--procIdent;
	HLSL_GEN_WRITE_PROC(" ");
	HLSL_GEN_WRITE_PROC("} else {");
	HLSL_GEN_WRITE_PROC(" ");
	++procIdent;
}

void d912pxy_hlsl_generator::ProcSIO_ENDIF(DWORD * op)
{
	--procIdent;
	HLSL_GEN_WRITE_PROC(" ");
	HLSL_GEN_WRITE_PROC("}");
	HLSL_GEN_WRITE_PROC(" ");
}

void d912pxy_hlsl_generator::ProcSIO_BREAK(DWORD * op)
{
	UINT64 cmpStr = FormatCmpString(op[0]);
	d912pxy_hlsl_generator_regtext lOp = FormatSrcRegister(op, 1, 0, 0, 1);
	d912pxy_hlsl_generator_regtext rOp = FormatSrcRegister(op, 1, 1, 0, 1);

	HLSL_GEN_WRITE_PROC(" ");
	HLSL_GEN_WRITE_PROC("if (%s%s%s)",
		lOp.t,
		(char*)&cmpStr,
		rOp.t
	);
	++procIdent;
	HLSL_GEN_WRITE_PROC("	break;");
	--procIdent;
	HLSL_GEN_WRITE_PROC(" ");	

}

void d912pxy_hlsl_generator::ProcSIO_LRP(DWORD * op)
{
//#define asm_lrp(ret,s0,s1,s2) ret = lerp(s2,s1,s0) 
	UINT64 writeMask = GetWriteMaskStr(op[1]);
	

	d912pxy_hlsl_generator_regtext sDst = FormatDstRegister(op);
	d912pxy_hlsl_generator_regtext sSrc0 = FormatSrcRegister(op, GetWriteMask(op[1]), 0, 1, 1);
	d912pxy_hlsl_generator_regtext sSrc1 = FormatSrcRegister(op, GetWriteMask(op[1]), 1, 1, 1);
	d912pxy_hlsl_generator_regtext sSrc2 = FormatSrcRegister(op, GetWriteMask(op[1]), 2, 1, 1);

	const char* mid[2] = { ", ", ", " };
	d912pxy_hlsl_generator_regtext rSide = FormatRightSide3(op[1], "dx9_lerp(", ")", mid, sSrc2, sSrc1, sSrc0, GetDstLenByWriteMask(op[1]));

	HLSL_GEN_WRITE_PROC("%s = %s;",
		sDst.t,
		rSide.t
	);
}

void d912pxy_hlsl_generator::ProcSIO_SLT(DWORD * op)
{
	ProcSIO_2OP(op, "(", " < ", ") ? 1.0f : 0.0f");
}

void d912pxy_hlsl_generator::ProcSIO_ABS(DWORD * op)
{
	ProcSIO_1OP(op, "abs(", ")");
}

void d912pxy_hlsl_generator::ProcSIO_SGE(DWORD * op)
{
	ProcSIO_2OP(op, "(", " >= ", ") ? 1 : 0");
}

void d912pxy_hlsl_generator::ProcSIO_SGN(DWORD * op)
{
	//megai2: TODO
	ProcSIO_2OP(op, "(", " < ", ") ? -1 : 1");
}

void d912pxy_hlsl_generator::ProcSIO_SINCOS(DWORD * op)
{
	UINT64 writeMask = GetWriteMask(op[1]);
	d912pxy_hlsl_generator_regtext sDst = FormatDstRegister(op);

	d912pxy_hlsl_generator_regtext sSrc0 = FormatSrcRegister(op, 0x1, 0, 1, 1);

	HLSL_GEN_WRITE_PROC("{ // sincos");
	++procIdent;

	HLSL_GEN_WRITE_PROC("float2 tmp = {0, 0};");

	if (writeMask & 0x1)
	{
		d912pxy_hlsl_generator_regtext rSide = FormatRightSide1(op[1], "cos(", ")", sSrc0, 1);
	

		HLSL_GEN_WRITE_PROC("tmp.x = %s;",			
			rSide.t
		);	
	}

	if (writeMask & 0x2)
	{
		d912pxy_hlsl_generator_regtext rSide = FormatRightSide1(op[1], "sin(", ")", sSrc0, 1);

		HLSL_GEN_WRITE_PROC("tmp.y = %s;",			
			rSide.t
		);
	}

	HLSL_GEN_WRITE_PROC("%s = tmp;", sDst.t);

	--procIdent;
	HLSL_GEN_WRITE_PROC("}");	
}

void d912pxy_hlsl_generator::ProcSIO_UNK(DWORD * op)
{
	UINT ilen = (op[0] >> 24) & 0xF;
	LOG_DBG_DTDM("unknown opcode %u length %u", op[0] & 0xFFFF, ilen);
	HLSL_GEN_WRITE_PROC("error //UNK OP %u length %u", op[0] & 0xFFFF, ilen);

	for (int i = 1; i != ilen+1; ++i)
	{
		LOG_DBG_DTDM("op par %u = %08lX", i - 1, op[i]);
		HLSL_GEN_WRITE_PROC("error //op par %u = %08lX", i - 1, op[i]);
	}
	
}

void d912pxy_hlsl_generator::WriteShaderHeadData()
{
	if (genProfile[PXY_INNER_SHDR_BUG_SRGB_READ])
		HLSL_GEN_WRITE_HEADI(0, "#define dx9_texture_srgb_read(a,b) dx9_texture_srgb_read_proc(a,b)");
	else
		HLSL_GEN_WRITE_HEADI(0, "#define dx9_texture_srgb_read(a,b) ");

	if (sRGB_alphatest_bits & PXY_SDB_HLSL_SRGB_ALPHATEST_COND_SRGBW)
		HLSL_GEN_WRITE_HEADI(0, "#define srgb_write_color_lin2s(color) color_lin2s_cond(color, texState.texture_s31 >> 13)");
	else
		HLSL_GEN_WRITE_HEADI(0, "#define srgb_write_color_lin2s(color) color_lin2s_thru(color)");
		


	HLSL_GEN_WRITE_HEADI(0, "#include \"../common.hlsli\"");
	HLSL_GEN_WRITE_HEADI(0, "	");

	HLSL_GEN_WRITE_PROC("	");
	HLSL_GEN_WRITE_PROC_PD(" ");

	if (isPS)
	{
		HLSL_GEN_WRITE_HEADI(0, "struct PS_INPUT");
		HLSL_GEN_WRITE_HEADI(0, "{");

		HLSL_GEN_WRITE_HEADO(0, "struct PS_OUTPUT");
		HLSL_GEN_WRITE_HEADO(0, "{");

		mainFunctionDeclStrIdx = HLSL_GEN_WRITE_PROC_PD("PS_OUTPUT main(PS_INPUT inp)");
		HLSL_GEN_WRITE_PROC_PD("{ ");
		++procIdent;
		HLSL_GEN_WRITE_PROC_PD("PS_OUTPUT ret; ");
		HLSL_GEN_WRITE_PROC_PD("");
	}
	else {
		HLSL_GEN_WRITE_HEADI(0, "struct VS_INPUT");
		HLSL_GEN_WRITE_HEADI(0, "{");

		HLSL_GEN_WRITE_HEADO(0, "struct VS_OUTPUT");
		HLSL_GEN_WRITE_HEADO(0, "{");

		if (genProfile[PXY_INNER_SHDR_BUG_CLIPPLANE0])
		{
			HLSL_GEN_WRITE_PROC_PD("vs_clip_plane0_def");
		}

		mainFunctionDeclStrIdx = HLSL_GEN_WRITE_PROC_PD("VS_OUTPUT main(VS_INPUT inp)");


		HLSL_GEN_WRITE_PROC_PD("{");
		++procIdent;
		HLSL_GEN_WRITE_PROC_PD("VS_OUTPUT ret;");
		HLSL_GEN_WRITE_PROC_PD("");
	}
}

void d912pxy_hlsl_generator::WriteShaderTailData()
{
	if (isPS)
	{
		if (!PSpositionUsed)
		{
			HLSL_GEN_WRITE_HEADI(HLSL_HIO_PRIORITY(HLSL_HIO_PRIOG_POS, 0), "	float4 unusedPos: SV_POSITION;");
		}
	}

	HLSL_GEN_WRITE_PROC_PD(" ");
	HLSL_GEN_WRITE_HEADO(HLSL_HIO_PRIORITY(HLSL_HIO_PRIOG_END, 0), "};");
	HLSL_GEN_WRITE_HEADI(HLSL_HIO_PRIORITY(HLSL_HIO_PRIOG_END, 0), "};");
	HLSL_GEN_WRITE_HEADI(HLSL_HIO_PRIORITY(HLSL_HIO_PRIOG_END, 1), "	");

	if (isPS)
	{
		//		HLSL_GEN_WRITE_PROC("*/");
		HLSL_GEN_WRITE_PROC("");

		if ((NaNguard_flag >> PXY_SDB_HLSL_NAN_GUARD_PS_SHIFT) & PXY_SDB_HLSL_NAN_GUARD_RET)
		{
			HLSL_GEN_WRITE_PROC("dx9_ps_nan_cull_emulation(dx9_ret_color_reg_ac);");
		}

		if (genProfile[PXY_INNER_SHDR_BUG_ALPHA_TEST])
		{
			if (genProfile[PXY_INNER_SHDR_BUG_SRGB_WRITE])
				HLSL_GEN_WRITE_PROC("dx9_ps_write_emulation_at_srgb(dx9_ret_color_reg_ac);");
			else
				HLSL_GEN_WRITE_PROC("dx9_ps_write_emulation_at(dx9_ret_color_reg_ac);");
		}
		else {
			if (genProfile[PXY_INNER_SHDR_BUG_SRGB_WRITE])
				HLSL_GEN_WRITE_PROC("dx9_ps_write_emulation_srgb(dx9_ret_color_reg_ac);");
			else
				HLSL_GEN_WRITE_PROC("dx9_ps_write_emulation(dx9_ret_color_reg_ac);");
		}

		/*if (genVSClipplane0)
		{
			if (PSpositionUsed)
			{
				HLSL_GEN_WRITE_PROC("dx9_clip_plane_ps(ps_ros_reg_ac, 0);");
			} else {
				HLSL_GEN_WRITE_PROC("dx9_clip_plane_ps(inp.unusedPos, 0);");
			}
		}*/
	}
	else {
		HLSL_GEN_WRITE_PROC("");
		HLSL_GEN_WRITE_PROC("dx9_halfpixel_pos_reg_ac = dx9_fix_halfpixel_offset(dx9_halfpixel_pos_reg_ac);");

		if (NaNguard_flag & PXY_SDB_HLSL_NAN_GUARD_RET)
		{
			HLSL_GEN_WRITE_PROC("dx9_vs_nan_cull_emulation(dx9_halfpixel_pos_reg_ac);");
		}
	}

	HLSL_GEN_WRITE_PROC("");
	HLSL_GEN_WRITE_PROC("return ret;");
	HLSL_GEN_WRITE_PROC("");
	--procIdent;
	HLSL_GEN_WRITE_PROC("}");
}

void d912pxy_hlsl_generator::WriteExtraUnusedRegs()
{
	//megai2: defining extra unused regs kills gpu memory bandwith
	if (d912pxy_s.config.GetValueUI32(PXY_CFG_SDB_FORCE_UNUSED_REGS))
	{
		//megai2: force define not used registers to align vs-ps io properly
		if (isPS)
		{
			UINT dclId = 15;

			UINT itr = (HLSL_HIO_PRIORITY(HLSL_HIO_PRIOG_FROM_D3DDECLUSAGE[D3DDECLUSAGE_TEXCOORD], dclId) + headerOffsetI);
			do
			{
				--itr;
				--dclId;

				if (lines[itr] == 0)
				{
					HLSL_GEN_WRITE_HEADI(
						itr - headerOffsetI,
						"	float4 %s%u_s%u: %s%u;",
						"unused_ireg_", 0, dclId, GetUsageString(D3DDECLUSAGE_TEXCOORD, 1), dclId
					);
				}
			} while (dclId != 0);
		}
		else {
			UINT dclId = 15;

			UINT itr = (HLSL_HIO_PRIORITY(HLSL_HIO_PRIOG_FROM_D3DDECLUSAGE[D3DDECLUSAGE_TEXCOORD], dclId) + headerOffsetO);
			do
			{
				--itr;
				--dclId;

				if (lines[itr] == 0)
				{
					HLSL_GEN_WRITE_HEADO(
						itr - headerOffsetO,
						"	float4 %s%u_s%u: %s%u;",
						"unused_ireg_", 0, dclId, GetUsageString(D3DDECLUSAGE_TEXCOORD, 1), dclId
					);
				}
			} while (dclId != 0);
		}
	}
}

void d912pxy_hlsl_generator::FillHandlers()
{

	for (int i = 0; i != d912pxy_hlsl_generator_op_handler_group_size * d912pxy_hlsl_generator_op_handler_cnt; ++i)
		SIOhandlers[i] = &d912pxy_hlsl_generator::ProcSIO_UNK;

	//megai2: sm 1_0, uses same handles as sm 2 and 3 , cuz i'm lazy and hope it will work
#define __SIOtOF d912pxy_hlsl_generator_op_handler_group_size * d912pxy_hlsl_generator_op_handler_1_x
	SIOhandlers[__SIOtOF + D3DSIO_DEF] = &d912pxy_hlsl_generator::ProcSIO_DEF;
	SIOhandlers[__SIOtOF + D3DSIO_DEFI] = &d912pxy_hlsl_generator::ProcSIO_DEF;
	SIOhandlers[__SIOtOF + D3DSIO_DCL] = &d912pxy_hlsl_generator::ProcSIO_DCL;
	SIOhandlers[__SIOtOF + D3DSIO_DP3] = &d912pxy_hlsl_generator::ProcSIO_DP3;
	SIOhandlers[__SIOtOF + D3DSIO_DP4] = &d912pxy_hlsl_generator::ProcSIO_DP4;
	SIOhandlers[__SIOtOF + D3DSIO_TEX] = &d912pxy_hlsl_generator::ProcSIO_TEXLD;
	SIOhandlers[__SIOtOF + D3DSIO_MUL] = &d912pxy_hlsl_generator::ProcSIO_MUL;
	SIOhandlers[__SIOtOF + D3DSIO_MAD] = &d912pxy_hlsl_generator::ProcSIO_MAD;
	SIOhandlers[__SIOtOF + D3DSIO_ADD] = &d912pxy_hlsl_generator::ProcSIO_ADD;
	SIOhandlers[__SIOtOF + D3DSIO_MOV] = &d912pxy_hlsl_generator::ProcSIO_MOV;
	SIOhandlers[__SIOtOF + D3DSIO_MOVA] = &d912pxy_hlsl_generator::ProcSIO_MOV;
	SIOhandlers[__SIOtOF + D3DSIO_REP] = &d912pxy_hlsl_generator::ProcSIO_REP;
	SIOhandlers[__SIOtOF + D3DSIO_ENDREP] = &d912pxy_hlsl_generator::ProcSIO_ENDREP;
	SIOhandlers[__SIOtOF + D3DSIO_MIN] = &d912pxy_hlsl_generator::ProcSIO_MIN;
	SIOhandlers[__SIOtOF + D3DSIO_MAX] = &d912pxy_hlsl_generator::ProcSIO_MAX;
	SIOhandlers[__SIOtOF + D3DSIO_RCP] = &d912pxy_hlsl_generator::ProcSIO_RCP;
	SIOhandlers[__SIOtOF + D3DSIO_CMP] = &d912pxy_hlsl_generator::ProcSIO_CMP;
	SIOhandlers[__SIOtOF + D3DSIO_DP2ADD] = &d912pxy_hlsl_generator::ProcSIO_DP2ADD;
	SIOhandlers[__SIOtOF + D3DSIO_FRC] = &d912pxy_hlsl_generator::ProcSIO_FRC;
	SIOhandlers[__SIOtOF + D3DSIO_POW] = &d912pxy_hlsl_generator::ProcSIO_POW;
	SIOhandlers[__SIOtOF + D3DSIO_RSQ] = &d912pxy_hlsl_generator::ProcSIO_RSQ;
	SIOhandlers[__SIOtOF + D3DSIO_NRM] = &d912pxy_hlsl_generator::ProcSIO_NRM;
	SIOhandlers[__SIOtOF + D3DSIO_LOG] = &d912pxy_hlsl_generator::ProcSIO_LOG;
	SIOhandlers[__SIOtOF + D3DSIO_EXP] = &d912pxy_hlsl_generator::ProcSIO_EXP;
	SIOhandlers[__SIOtOF + D3DSIO_EXPP] = &d912pxy_hlsl_generator::ProcSIO_EXPP;
	SIOhandlers[__SIOtOF + D3DSIO_TEXKILL] = &d912pxy_hlsl_generator::ProcSIO_TEXKILL;
	SIOhandlers[__SIOtOF + D3DSIO_IFC] = &d912pxy_hlsl_generator::ProcSIO_IF;
	SIOhandlers[__SIOtOF + D3DSIO_ELSE] = &d912pxy_hlsl_generator::ProcSIO_ELSE;
	SIOhandlers[__SIOtOF + D3DSIO_ENDIF] = &d912pxy_hlsl_generator::ProcSIO_ENDIF;
	SIOhandlers[__SIOtOF + D3DSIO_BREAKC] = &d912pxy_hlsl_generator::ProcSIO_BREAK;
	SIOhandlers[__SIOtOF + D3DSIO_TEXLDL] = &d912pxy_hlsl_generator::ProcSIO_TEXLDL;
	SIOhandlers[__SIOtOF + D3DSIO_LRP] = &d912pxy_hlsl_generator::ProcSIO_LRP;
	SIOhandlers[__SIOtOF + D3DSIO_SLT] = &d912pxy_hlsl_generator::ProcSIO_SLT;
	SIOhandlers[__SIOtOF + D3DSIO_ABS] = &d912pxy_hlsl_generator::ProcSIO_ABS;
	SIOhandlers[__SIOtOF + D3DSIO_SGE] = &d912pxy_hlsl_generator::ProcSIO_SGE;
	SIOhandlers[__SIOtOF + D3DSIO_SGN] = &d912pxy_hlsl_generator::ProcSIO_SGN;
	SIOhandlers[__SIOtOF + D3DSIO_SINCOS] = &d912pxy_hlsl_generator::ProcSIO_SINCOS;
#undef __SIOtOF

	//sm 2_0
#define __SIOtOF d912pxy_hlsl_generator_op_handler_group_size * d912pxy_hlsl_generator_op_handler_2_x
	SIOhandlers[__SIOtOF + D3DSIO_DEF] = &d912pxy_hlsl_generator::ProcSIO_DEF;
	SIOhandlers[__SIOtOF + D3DSIO_DEFI] = &d912pxy_hlsl_generator::ProcSIO_DEF;
	SIOhandlers[__SIOtOF + D3DSIO_DCL] = &d912pxy_hlsl_generator::ProcSIO_DCL;
	SIOhandlers[__SIOtOF + D3DSIO_DP3] = &d912pxy_hlsl_generator::ProcSIO_DP3;
	SIOhandlers[__SIOtOF + D3DSIO_DP4] = &d912pxy_hlsl_generator::ProcSIO_DP4;
	SIOhandlers[__SIOtOF + D3DSIO_TEX] = &d912pxy_hlsl_generator::ProcSIO_TEXLD;
	SIOhandlers[__SIOtOF + D3DSIO_MUL] = &d912pxy_hlsl_generator::ProcSIO_MUL;
	SIOhandlers[__SIOtOF + D3DSIO_MAD] = &d912pxy_hlsl_generator::ProcSIO_MAD;
	SIOhandlers[__SIOtOF + D3DSIO_ADD] = &d912pxy_hlsl_generator::ProcSIO_ADD;
	SIOhandlers[__SIOtOF + D3DSIO_MOV] = &d912pxy_hlsl_generator::ProcSIO_MOV;
	SIOhandlers[__SIOtOF + D3DSIO_MOVA] = &d912pxy_hlsl_generator::ProcSIO_MOV;
	SIOhandlers[__SIOtOF + D3DSIO_REP] = &d912pxy_hlsl_generator::ProcSIO_REP;
	SIOhandlers[__SIOtOF + D3DSIO_ENDREP] = &d912pxy_hlsl_generator::ProcSIO_ENDREP;
	SIOhandlers[__SIOtOF + D3DSIO_MIN] = &d912pxy_hlsl_generator::ProcSIO_MIN;
	SIOhandlers[__SIOtOF + D3DSIO_MAX] = &d912pxy_hlsl_generator::ProcSIO_MAX;
	SIOhandlers[__SIOtOF + D3DSIO_RCP] = &d912pxy_hlsl_generator::ProcSIO_RCP;
	SIOhandlers[__SIOtOF + D3DSIO_CMP] = &d912pxy_hlsl_generator::ProcSIO_CMP;
	SIOhandlers[__SIOtOF + D3DSIO_DP2ADD] = &d912pxy_hlsl_generator::ProcSIO_DP2ADD;
	SIOhandlers[__SIOtOF + D3DSIO_FRC] = &d912pxy_hlsl_generator::ProcSIO_FRC;
	SIOhandlers[__SIOtOF + D3DSIO_POW] = &d912pxy_hlsl_generator::ProcSIO_POW;
	SIOhandlers[__SIOtOF + D3DSIO_RSQ] = &d912pxy_hlsl_generator::ProcSIO_RSQ;
	SIOhandlers[__SIOtOF + D3DSIO_NRM] = &d912pxy_hlsl_generator::ProcSIO_NRM;
	SIOhandlers[__SIOtOF + D3DSIO_LOG] = &d912pxy_hlsl_generator::ProcSIO_LOG;
	SIOhandlers[__SIOtOF + D3DSIO_EXP] = &d912pxy_hlsl_generator::ProcSIO_EXP;
	SIOhandlers[__SIOtOF + D3DSIO_EXPP] = &d912pxy_hlsl_generator::ProcSIO_EXPP;
	SIOhandlers[__SIOtOF + D3DSIO_TEXKILL] = &d912pxy_hlsl_generator::ProcSIO_TEXKILL;
	SIOhandlers[__SIOtOF + D3DSIO_IFC] = &d912pxy_hlsl_generator::ProcSIO_IF;
	SIOhandlers[__SIOtOF + D3DSIO_ELSE] = &d912pxy_hlsl_generator::ProcSIO_ELSE;
	SIOhandlers[__SIOtOF + D3DSIO_ENDIF] = &d912pxy_hlsl_generator::ProcSIO_ENDIF;
	SIOhandlers[__SIOtOF + D3DSIO_BREAKC] = &d912pxy_hlsl_generator::ProcSIO_BREAK;
	SIOhandlers[__SIOtOF + D3DSIO_TEXLDL] = &d912pxy_hlsl_generator::ProcSIO_TEXLDL;
	SIOhandlers[__SIOtOF + D3DSIO_SLT] = &d912pxy_hlsl_generator::ProcSIO_SLT;
	SIOhandlers[__SIOtOF + D3DSIO_ABS] = &d912pxy_hlsl_generator::ProcSIO_ABS;
	SIOhandlers[__SIOtOF + D3DSIO_SGE] = &d912pxy_hlsl_generator::ProcSIO_SGE;
	SIOhandlers[__SIOtOF + D3DSIO_SGN] = &d912pxy_hlsl_generator::ProcSIO_SGN;
	SIOhandlers[__SIOtOF + D3DSIO_SINCOS] = &d912pxy_hlsl_generator::ProcSIO_SINCOS;

#undef __SIOtOF

	//sm 3_0
#define __SIOtOF d912pxy_hlsl_generator_op_handler_group_size * d912pxy_hlsl_generator_op_handler_3_x
	SIOhandlers[__SIOtOF + D3DSIO_DEF] = &d912pxy_hlsl_generator::ProcSIO_DEF;
	SIOhandlers[__SIOtOF + D3DSIO_DEFI] = &d912pxy_hlsl_generator::ProcSIO_DEF;
	SIOhandlers[__SIOtOF + D3DSIO_DCL] = &d912pxy_hlsl_generator::ProcSIO_DCL;
	SIOhandlers[__SIOtOF + D3DSIO_DP3] = &d912pxy_hlsl_generator::ProcSIO_DP3;
	SIOhandlers[__SIOtOF + D3DSIO_DP4] = &d912pxy_hlsl_generator::ProcSIO_DP4;
	SIOhandlers[__SIOtOF + D3DSIO_TEX] = &d912pxy_hlsl_generator::ProcSIO_TEXLD;
	SIOhandlers[__SIOtOF + D3DSIO_MUL] = &d912pxy_hlsl_generator::ProcSIO_MUL;
	SIOhandlers[__SIOtOF + D3DSIO_MAD] = &d912pxy_hlsl_generator::ProcSIO_MAD;
	SIOhandlers[__SIOtOF + D3DSIO_ADD] = &d912pxy_hlsl_generator::ProcSIO_ADD;
	SIOhandlers[__SIOtOF + D3DSIO_MOV] = &d912pxy_hlsl_generator::ProcSIO_MOV;
	SIOhandlers[__SIOtOF + D3DSIO_MOVA] = &d912pxy_hlsl_generator::ProcSIO_MOV;
	SIOhandlers[__SIOtOF + D3DSIO_REP] = &d912pxy_hlsl_generator::ProcSIO_REP;
	SIOhandlers[__SIOtOF + D3DSIO_ENDREP] = &d912pxy_hlsl_generator::ProcSIO_ENDREP;
	SIOhandlers[__SIOtOF + D3DSIO_MIN] = &d912pxy_hlsl_generator::ProcSIO_MIN;
	SIOhandlers[__SIOtOF + D3DSIO_MAX] = &d912pxy_hlsl_generator::ProcSIO_MAX;
	SIOhandlers[__SIOtOF + D3DSIO_RCP] = &d912pxy_hlsl_generator::ProcSIO_RCP;
	SIOhandlers[__SIOtOF + D3DSIO_CMP] = &d912pxy_hlsl_generator::ProcSIO_CMP;
	SIOhandlers[__SIOtOF + D3DSIO_DP2ADD] = &d912pxy_hlsl_generator::ProcSIO_DP2ADD;
	SIOhandlers[__SIOtOF + D3DSIO_FRC] = &d912pxy_hlsl_generator::ProcSIO_FRC;
	SIOhandlers[__SIOtOF + D3DSIO_POW] = &d912pxy_hlsl_generator::ProcSIO_POW;
	SIOhandlers[__SIOtOF + D3DSIO_RSQ] = &d912pxy_hlsl_generator::ProcSIO_RSQ;
	SIOhandlers[__SIOtOF + D3DSIO_NRM] = &d912pxy_hlsl_generator::ProcSIO_NRM;
	SIOhandlers[__SIOtOF + D3DSIO_LOG] = &d912pxy_hlsl_generator::ProcSIO_LOG;
	SIOhandlers[__SIOtOF + D3DSIO_EXP] = &d912pxy_hlsl_generator::ProcSIO_EXP;
	SIOhandlers[__SIOtOF + D3DSIO_EXPP] = &d912pxy_hlsl_generator::ProcSIO_EXPP;
	SIOhandlers[__SIOtOF + D3DSIO_TEXKILL] = &d912pxy_hlsl_generator::ProcSIO_TEXKILL;
	SIOhandlers[__SIOtOF + D3DSIO_IFC] = &d912pxy_hlsl_generator::ProcSIO_IF;
	SIOhandlers[__SIOtOF + D3DSIO_ELSE] = &d912pxy_hlsl_generator::ProcSIO_ELSE;
	SIOhandlers[__SIOtOF + D3DSIO_ENDIF] = &d912pxy_hlsl_generator::ProcSIO_ENDIF;
	SIOhandlers[__SIOtOF + D3DSIO_BREAKC] = &d912pxy_hlsl_generator::ProcSIO_BREAK;
	SIOhandlers[__SIOtOF + D3DSIO_TEXLDL] = &d912pxy_hlsl_generator::ProcSIO_TEXLDL;
	SIOhandlers[__SIOtOF + D3DSIO_LRP] = &d912pxy_hlsl_generator::ProcSIO_LRP;
	SIOhandlers[__SIOtOF + D3DSIO_SLT] = &d912pxy_hlsl_generator::ProcSIO_SLT;
	SIOhandlers[__SIOtOF + D3DSIO_ABS] = &d912pxy_hlsl_generator::ProcSIO_ABS;
	SIOhandlers[__SIOtOF + D3DSIO_SGE] = &d912pxy_hlsl_generator::ProcSIO_SGE;
	SIOhandlers[__SIOtOF + D3DSIO_SGN] = &d912pxy_hlsl_generator::ProcSIO_SGN;
	SIOhandlers[__SIOtOF + D3DSIO_SINCOS] = &d912pxy_hlsl_generator::ProcSIO_SINCOS;
#undef __SIOtOF
}
